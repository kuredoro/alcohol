#pragma once

#include <ast/manager.hpp>
#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <constraint/set.hpp>
#include <linter/address_expr_collector.hpp>

namespace linter
{

struct linter
{
    explicit linter(ast::manager& store) :
        astStore_(store), exprStat_(store)
    {}

    const std::vector<std::string>& warnings() const
    {
        return diagnostics_;
    }

    void process(ast::block& block);

private:
    ast::manager& astStore_;
    std::vector<std::string> diagnostics_;
    address_expr_collector exprStat_;
};

}
