#pragma once

#include <memory>

#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <linter/address_expr_collector.hpp>

namespace linter
{

struct linter
{
    linter() = default;

    const std::vector<std::string>& warnings() const
    {
        return diagnostics_;
    }

    void process(ast::block& block)
    {
        address_expr_collector collector;
        block.accept(collector);
    }

private:
    std::vector<std::string> diagnostics_;
};

}
