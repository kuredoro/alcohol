#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <string>
#include <memory>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/LLVM.h>
#include <spdlog/spdlog.h>

struct AlcoholPluginAction : public clang::PluginASTAction {

    bool ParseArgs(const clang::CompilerInstance &, const std::vector<std::string>& args) override
    {
        spdlog::set_level(spdlog::level::info);

        for (auto& arg : args)
        {
            if (arg == "-debug")
            {
                spdlog::set_level(spdlog::level::trace);
            }
        }

        return true;
    }

    ActionType getActionType() override { return Cmdline; }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, clang::StringRef file) override;

private:
    std::unique_ptr<clang::ast_matchers::MatchFinder> m_astFinder;
    std::unique_ptr<clang::ast_matchers::MatchFinder::MatchCallback> m_funcMatcherCb;
};
