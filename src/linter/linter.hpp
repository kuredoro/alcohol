#pragma once

#include <memory>

#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <linter/address_expr_collector.hpp>

namespace linter
{

struct linter
{
    explicit linter(ast::manager& store) :
        astStore_(store)
    {}

    const std::vector<std::string>& warnings() const
    {
        return diagnostics_;
    }

    void process(ast::block& block)
    {
        address_expr_collector collector(astStore_);
        block.accept(collector);
    }

private:
    ast::manager& astStore_;
    std::vector<std::string> diagnostics_;
};

}
