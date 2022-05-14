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

struct MatchPointerMutationCallback : public MatchCallbackWithVisitor
{
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

    allocCb_ = std::make_unique<MatchMemoryAllocationCallback>(*this);
    auto allocDeclPattern =
        declStmt(
            hasSingleDecl(varDecl(
                hasDescendant(mallocPattern)
            ).bind("varDecl"))
        );

    auto allocAssignPattern =
        binaryOperator(
            isAssignmentOperator(),
            hasLHS(declRefExpr(hasDeclaration(varDecl().bind("varDecl")))),
            hasRHS(hasDescendant(mallocPattern))
        );

    deallocCb_ = std::make_unique<MatchMemoryDeallocationCallback>(*this);
    auto deallocPattern =
        callExpr(
            hasDeclaration(functionDecl(
                hasName("free")
            )),
            hasArgument(0, declRefExpr(hasDeclaration(varDecl().bind("varDecl"))))
        );

    declCb_ = std::make_unique<MatchVariableDeclarationCallback>(*this);
    auto varDeclPattern =
        declStmt(
            hasSingleDecl(varDecl(
                hasInitializer(expr().bind("expr"))
            ).bind("varDecl"))
        );

    matcher_->addMatcher(allocDeclPattern, allocCb_.get());
    matcher_->addMatcher(allocAssignPattern, allocCb_.get());

    matcher_->addMatcher(deallocPattern, deallocCb_.get());

    matcher_->addMatcher(varDeclPattern, declCb_.get());
  }

  bool TraverseFunctionDecl(const clang::FunctionDecl* funcDecl)
  {
    llvm::outs() << "Func!!!\n";
    auto body = dyn_cast<CompoundStmt>(funcDecl->getBody());
    if (!body)
    {
      return true;
    }

    TraverseCompoundStmt(body);

    auto block = store_.make_statement<ast::block>(gsl::make_span(mainStatements_));

    llvm::outs() << "Program:\n" << block->to_string() << '\n';

    return true;
  }
  
  bool TraverseCompoundStmt(const CompoundStmt* stmts)
  {
    for (auto* stmt : stmts->body())
    {
      matcher_->match(*stmt, *const_cast<ASTContext*>(ctx_));
      llvm::outs() << "STMT\n";
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
  }

private:
  std::unique_ptr<MatchFinder> matcher_;
  //std::unique_ptr<MatchFinder::MatchCallback> m_dataDeclCb, m_ptrDeclCb;
  std::unique_ptr<MatchCallbackWithVisitor> allocCb_, deallocCb_, declCb_;

  ast::manager store_;
  std::vector<ast::statement*> mainStatements_;

  const ASTContext* ctx_;
  const SourceManager& sm_;
};
