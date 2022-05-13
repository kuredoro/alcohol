#pragma once

#include <alc/ast/manager.hpp>
#include <alc/ast/statements.hpp>
#include <alc/ast/expressions.hpp>
#include <alc/constraint/set.hpp>
#include <alc/linter/address_expr_collector.hpp>
#include <alc/linter/configuration.hpp>

#include <set>
#include <vector>

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

    // Our only configuration for now
    configuration cnf_;

    friend struct linter_visitor;
};

}
