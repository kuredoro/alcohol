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
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <type_traits>

using namespace clang;
using namespace ast_matchers;

struct MatchMemoryAllocationCallback : public MatchFinder::MatchCallback
{
    void run(const MatchFinder::MatchResult& result) override
    {
        llvm::outs() << "Memory alloc!\n";
    }
};

struct MatchPointerMutationCallback : public MatchFinder::MatchCallback
{
    void run(const MatchFinder::MatchResult& result) override;
};

struct MatchMemoryDeallocationCallback : public MatchFinder::MatchCallback
{
    void run(const MatchFinder::MatchResult& result) override
    {
        llvm::outs() << "Memory free!\n";
    }
};

struct AftFuncDeclVisitor : public clang::RecursiveASTVisitor<AftFuncDeclVisitor>
{
  explicit AftFuncDeclVisitor(const clang::ASTContext* ctx) : m_ctx(ctx), m_sm(ctx->getSourceManager())
  {
    m_matcher = std::make_unique<MatchFinder>();  

    m_allocCb = std::make_unique<MatchMemoryAllocationCallback>();
    auto allocPattern =
        declStmt(
            hasSingleDecl(varDecl(
                hasDescendant(callExpr(
                    hasDeclaration(functionDecl(
                        hasName("malloc")
                    ))
                ))
            ))
        );

    m_deallocCb = std::make_unique<MatchMemoryDeallocationCallback>();
    auto deallocPattern =
        callExpr(
            hasDeclaration(functionDecl(
                hasName("free")
            ))
        );

    m_matcher->addMatcher(allocPattern, m_allocCb.get());
    m_matcher->addMatcher(deallocPattern, m_deallocCb.get());
  }

  bool TraverseFunctionDecl(const clang::FunctionDecl* funcDecl)
  {
    llvm::outs() << "Func!!!\n";
    auto body = dyn_cast<CompoundStmt>(funcDecl->getBody());
    if (!body)
    {
      return true;
    }

    return TraverseCompoundStmt(body);
  }
  
  bool TraverseCompoundStmt(const CompoundStmt* stmts)
  {
    for (auto* stmt : stmts->body())
    {
      m_matcher->match(*stmt, *const_cast<ASTContext*>(m_ctx));
      llvm::outs() << "STMT\n";
      //TraverseStmt(stmt);
    }

    return true;
  }
  
private:
  std::unique_ptr<MatchFinder> m_matcher;
  //std::unique_ptr<MatchFinder::MatchCallback> m_dataDeclCb, m_ptrDeclCb;
  std::unique_ptr<MatchFinder::MatchCallback> m_allocCb, m_deallocCb;

  const ASTContext* m_ctx;
  const SourceManager& m_sm;
};

/*
void MatchMemoryAllocationCallback::run(const MatchFinder::MatchResult& result)
{
    llvm::outs() << "Memory alloc!\n";
}
*/
