#pragma once

#include <ast/statements.hpp>
#include <ast/expressions.hpp>

namespace linter
{

struct linter
{
    linter() = default;

    void process(ast::block& block);

    const std::vector<std::string>& warnings() const
    {
        return diagnostics_;
    }

private:
    std::vector<std::string> diagnostics_;

    void process(ast::statement& statement);
    void process(ast::decl& decl);
    void process(ast::alloc& alloc);
    void process(ast::store& store);
    void process(ast::dispose& dispose);
};

}
