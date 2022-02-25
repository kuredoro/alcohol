#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/LLVM.h>

#include "AlcoholPluginAction.hpp"

using namespace clang;
using namespace clang::ast_matchers;

struct MatchFuncCallback : public MatchFinder::MatchCallback
{
    void run(const MatchFinder::MatchResult& result) override
    {
        llvm::outs() << "いえい！\n";
    }
};

std::unique_ptr<ASTConsumer> AlcoholPluginAction::CreateASTConsumer(CompilerInstance &CI, StringRef file)
{
    m_astFinder = std::make_unique<MatchFinder>();

    m_funcMatcherCb = std::make_unique<MatchFuncCallback>();
    m_astFinder->addMatcher(
        traverse(TK_IgnoreUnlessSpelledInSource,
            functionDecl(hasName("main"))
        ),
        m_funcMatcherCb.get());

    return std::move(m_astFinder->newASTConsumer());
}
