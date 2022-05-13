#include <clang/AST/Decl.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/LLVM.h>

#include "AlcoholPluginAction.hpp"
#include "AftFuncDeclConsumer.hpp"

using namespace clang;
using namespace clang::ast_matchers;

struct MatchFuncDeclCallback : public MatchFinder::MatchCallback
{
    void run(const MatchFinder::MatchResult& result) override;
};

std::unique_ptr<ASTConsumer> AlcoholPluginAction::CreateASTConsumer(CompilerInstance &CI, StringRef file)
{
    m_astFinder = std::make_unique<MatchFinder>();

    m_funcMatcherCb = std::make_unique<MatchFuncDeclCallback>();
    m_astFinder->addMatcher(
        traverse(TK_IgnoreUnlessSpelledInSource,
            functionDecl(hasName("main")).bind("main")
        ),
        m_funcMatcherCb.get());

    return std::move(m_astFinder->newASTConsumer());
}

void MatchFuncDeclCallback::run(const MatchFinder::MatchResult& result)
{
    auto checker = std::make_unique<AftFuncDeclVisitor>(result.Context);

    auto funcDecl = result.Nodes.getNodeAs<FunctionDecl>("main");
    if (!funcDecl)
    {
        llvm::outs() << "No main func\n";
        return;
    }

    checker->TraverseFunctionDecl(funcDecl);

    llvm::outs() << "いえい！\n";
}
