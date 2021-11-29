#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/LLVM.h>

#include <clang/AST/Stmt.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <llvm/Support/raw_ostream.h>

using namespace clang;

//-----------------------------------------------------------------------------
// RecursiveASTVisitor
//-----------------------------------------------------------------------------
class AlcaSubsetVisitor
    : public clang::RecursiveASTVisitor<AlcaSubsetVisitor> {
public:
  explicit AlcaSubsetVisitor(clang::ASTContext *ctx) : ctx_(ctx), sm_(ctx->getSourceManager()) {}

  bool TraverseFunctionDecl(clang::FunctionDecl *S)
  {
    llvm::outs() << S->getDeclName().getAsString() << ":\n";
    
    auto body = dyn_cast<CompoundStmt>(S->getBody());
    if (!body)
    {
      llvm::outs() << "main has no compound body!\n";
      return true;
    }

    for (auto* stmt : body->body())
    {
      TraverseStmt(stmt);
    }

    return true;
  }
  
  bool TraverseStmt(Stmt* stmt)
  {
    if (stmt->getStmtClass() == Stmt::DeclStmtClass)
    {
      TraverseDeclStmt(dyn_cast<DeclStmt>(stmt));
      return true;
    }
    
    if (stmt->getStmtClass() == Stmt::BinaryOperatorClass)
    {
      TraverseBinaryOperator(dyn_cast<BinaryOperator>(stmt));
      return true;
    }
    
    return true;
  }
  
  bool TraverseDeclStmt(DeclStmt* decl)
  {
    std::string str;
    llvm::raw_string_ostream stream(str);
    decl->printPretty(stream, nullptr, PrintingPolicy(LangOptions()));

    llvm::outs() << str;
    return true;
  }
  
  bool TraverseBinaryOperator(BinaryOperator* expr)
  {
    if (!expr->isAssignmentOp())
      return true;

    std::string str;
    llvm::raw_string_ostream stream(str);
    expr->printPretty(stream, nullptr, PrintingPolicy(LangOptions()));

    llvm::outs() << str << '\n';
    return true;
  }

private:
  clang::ASTContext *ctx_;
  SourceManager& sm_;
};

//-----------------------------------------------------------------------------
// ASTConsumer
//-----------------------------------------------------------------------------
class AlcaSubsetASTConsumer : public clang::ASTConsumer {
public:
  AlcaSubsetASTConsumer(clang::ASTContext &Ctx, clang::SourceManager &SM)
    : sm_(SM), visitor_(&Ctx) {}

  void HandleTranslationUnit(clang::ASTContext &Ctx) override {
    // Only visit declarations declared in the input TU
    auto decls = Ctx.getTranslationUnitDecl()->decls();
    for (auto &decl : decls) {
      if (!sm_.isInMainFile(decl->getLocation()))
        continue;

      auto fDecl = dyn_cast<FunctionDecl>(decl);

      if (!fDecl || !fDecl->hasBody())
        continue;

      auto name = fDecl->getName();
      if (name != "main")
        continue;
      
      visitor_.TraverseDecl(fDecl);
    }
  }

private:
  clang::SourceManager &sm_;

  AlcaSubsetVisitor visitor_;
};

//-----------------------------------------------------------------------------
// FrotendAction
//-----------------------------------------------------------------------------
class FindAlcaSubsetPluginAction : public PluginASTAction {
public:
  // Our plugin can alter behavior based on the command line options
  bool ParseArgs(const CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }

  // Returns our ASTConsumer per translation unit.
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    return std::make_unique<AlcaSubsetASTConsumer>(CI.getASTContext(),
                                                         CI.getSourceManager());
  }
};

//-----------------------------------------------------------------------------
// Registration
//-----------------------------------------------------------------------------
static FrontendPluginRegistry::Add<FindAlcaSubsetPluginAction>
    X(/*Name=*/"alca-subset",
      /*Desc=*/"Find alias calculus subset in C code");
