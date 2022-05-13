#include <vector>
#include <string>
#include <memory>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/LLVM.h>

struct AlcoholPluginAction : public clang::PluginASTAction {

    bool ParseArgs(const clang::CompilerInstance &, const std::vector<std::string> &) override
    {
      return true;
    }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, clang::StringRef file) override;

private:
    std::unique_ptr<clang::ast_matchers::MatchFinder> m_astFinder;
    std::unique_ptr<clang::ast_matchers::MatchFinder::MatchCallback> m_funcMatcherCb;
};
