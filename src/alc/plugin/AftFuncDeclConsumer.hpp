#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/Basic/LLVM.h>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>

#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <gsl/gsl-lite.hpp>
#include <llvm/Support/raw_ostream.h>

#include <alc/ast/statements.hpp>
#include <alc/ast/manager.hpp>
#include <alc/linter/linter.hpp>

#include <memory>
#include <type_traits>

using namespace clang;
using namespace ast_matchers;

struct AftFuncDeclVisitor;

struct MatchCallbackWithVisitor : public MatchFinder::MatchCallback
{
    AftFuncDeclVisitor& visitor;

    MatchCallbackWithVisitor(AftFuncDeclVisitor& visitor) : visitor(visitor) {}
    MatchCallbackWithVisitor(const MatchCallbackWithVisitor&) = default;
    MatchCallbackWithVisitor(MatchCallbackWithVisitor&&) = default;
};

struct MatchMemoryAllocationCallback : public MatchCallbackWithVisitor
{
    MatchMemoryAllocationCallback(AftFuncDeclVisitor& v) : MatchCallbackWithVisitor(v) {}

    void run(const MatchFinder::MatchResult& result) override;
};

struct MatchMemoryDeallocationCallback : public MatchCallbackWithVisitor
{
    MatchMemoryDeallocationCallback(AftFuncDeclVisitor& v) : MatchCallbackWithVisitor(v) {}

    void run(const MatchFinder::MatchResult& result) override;
};

struct MatchVariableDeclarationCallback : public MatchCallbackWithVisitor
{
    MatchVariableDeclarationCallback(AftFuncDeclVisitor& v) : MatchCallbackWithVisitor(v) {}

    void run(const MatchFinder::MatchResult& result) override;
};

struct MatchVariableAssignmentCallback : public MatchCallbackWithVisitor
{
    MatchVariableAssignmentCallback(AftFuncDeclVisitor& v) : MatchCallbackWithVisitor(v) {}

    void run(const MatchFinder::MatchResult& result) override;
};

struct MatchStoreCallback : public MatchCallbackWithVisitor
{
    MatchStoreCallback(AftFuncDeclVisitor& v) : MatchCallbackWithVisitor(v) {}

    void run(const MatchFinder::MatchResult& result) override;
};

inline auto newValuePattern(std::string bindAs)
{
    return 
        anyOf(
            ignoringParenImpCasts(integerLiteral().bind(bindAs)),
            binaryOperator().bind(bindAs),
            hasDescendant(declRefExpr(hasDeclaration(varDecl())).bind(bindAs))
        );
}

struct AftFuncDeclVisitor : public clang::RecursiveASTVisitor<AftFuncDeclVisitor>
{
  explicit AftFuncDeclVisitor(const clang::ASTContext* ctx) : ctx_(ctx), sm_(ctx->getSourceManager())
  {
    matcher_ = std::make_unique<MatchFinder>();  

    auto mallocPattern =
        callExpr(
            hasDeclaration(functionDecl(
                hasName("malloc")
            )),
            hasArgument(0, binaryOperator(
                hasDescendant(integerLiteral().bind("size"))
            ))
        );

    // ---

    allocCb_ = std::make_unique<MatchMemoryAllocationCallback>(*this);
    auto allocDeclPattern =
        declStmt(
            hasSingleDecl(varDecl(
                hasDescendant(mallocPattern)
            ).bind("varDecl"))
        ).bind("root");

    auto allocAssignPattern =
        binaryOperator(
            isAssignmentOperator(),
            hasLHS(declRefExpr(hasDeclaration(varDecl().bind("varDecl")))),
            hasRHS(hasDescendant(mallocPattern))
        ).bind("root");
    // TODO: find out if the order of `addMatcher` specifies the matcher priorities...
    matcher_->addMatcher(allocDeclPattern, allocCb_.get());
    matcher_->addMatcher(allocAssignPattern, allocCb_.get());

    // ---

    deallocCb_ = std::make_unique<MatchMemoryDeallocationCallback>(*this);
    auto deallocPattern =
        callExpr(
            hasDeclaration(functionDecl(
                hasName("free")
            )),
            hasArgument(0, declRefExpr(hasDeclaration(varDecl().bind("varDecl"))))
        ).bind("root");

    matcher_->addMatcher(deallocPattern, deallocCb_.get());

    // ---

    declCb_ = std::make_unique<MatchVariableDeclarationCallback>(*this);
    auto varDeclPattern =
        declStmt(
            hasSingleDecl(varDecl(
                hasInitializer(newValuePattern("newValue"))
            ).bind("varDecl"))
        ).bind("root");

    matcher_->addMatcher(varDeclPattern, declCb_.get());

    // ---

    varAssignCb_ = std::make_unique<MatchVariableAssignmentCallback>(*this);
    auto varAssignPattern =
        binaryOperator(
            isAssignmentOperator(),
            hasLHS(declRefExpr(hasDeclaration(varDecl().bind("varDecl")))),
            hasRHS(newValuePattern("newValue"))
        ).bind("root");

    matcher_->addMatcher(varAssignPattern, varAssignCb_.get());

    // ---

    storeCb_ = std::make_unique<MatchStoreCallback>(*this);
    auto storePattern =
        binaryOperator(
            isAssignmentOperator(),
            hasLHS(arraySubscriptExpr(
                hasBase(hasDescendant(declRefExpr(hasDeclaration(varDecl().bind("varDecl"))))),
                hasIndex(newValuePattern("index"))
            )),
            hasRHS(newValuePattern("newValue"))
        ).bind("root");

    matcher_->addMatcher(storePattern, storeCb_.get());
  }

  bool TraverseFunctionDecl(const clang::FunctionDecl* funcDecl)
  {
    auto body = dyn_cast<CompoundStmt>(funcDecl->getBody());
    if (!body)
    {
      return true;
    }

    TraverseCompoundStmt(body);

    auto block = store_.make_statement<ast::block>(gsl::make_span(mainStatements_));

    llvm::outs() << "Program:\n" << block->to_string() << '\n';

    linter::linter linter(store_);
    linter.process(*block);

    // Register custom diagnostic messages
    auto& diag = ctx_->getDiagnostics();
    auto diagWarnLeak = diag.getCustomDiagID(
        DiagnosticsEngine::Warning,
        "possible memory leak");

    auto diagWarnDoubleFree = diag.getCustomDiagID(
        DiagnosticsEngine::Warning,
        "possible double free");

    // Emit warnings
    auto diagnostics = linter.diagnostics();
    for (auto& issue : diagnostics)
    {
        if (issue.type == linter::diagnostic::type::memory_leak)
        {
            diag.Report(astMap_[issue.ref], diagWarnLeak);
        }
        else if (issue.type == linter::diagnostic::type::double_free)
        {
            diag.Report(astMap_[issue.ref], diagWarnDoubleFree);
        }
    }

    return true;
  }
  
  bool TraverseCompoundStmt(const CompoundStmt* stmts)
  {
    for (auto* stmt : stmts->body())
    {
      matcher_->match(*stmt, *const_cast<ASTContext*>(ctx_));
    }

    return true;
  }

  ast::manager& ast_store()
  {
      return store_;
  }

  void push_statement(ast::statement* stmt, clang::SourceLocation at)
  {
      mainStatements_.push_back(stmt);
      astMap_[stmt] = at;
  }

private:
  std::unique_ptr<MatchFinder> matcher_;
  //std::unique_ptr<MatchFinder::MatchCallback> m_dataDeclCb, m_ptrDeclCb;
  std::unique_ptr<MatchCallbackWithVisitor> allocCb_, deallocCb_, declCb_, varAssignCb_, storeCb_;

  ast::manager store_;
  std::vector<ast::statement*> mainStatements_;
  std::map<ast::statement*, clang::SourceLocation> astMap_;

  const ASTContext* ctx_;
  const SourceManager& sm_;
};
