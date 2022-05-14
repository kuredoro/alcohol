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

struct diagnostic
{
    enum class type
    {
        memory_leak, double_free
    };

    diagnostic(ast::statement* ref, type type) :
        ref(ref), type(type)
    {}

    ast::statement* ref;
    type type;
};

struct linter
{
    explicit linter(ast::manager& store) :
        astStore_(store), exprStat_(store)
    {}

    gsl::span<const diagnostic> diagnostics() const
    {
        return diagnostics_;
    }

    void process(ast::block& block);

private:
    ast::manager& astStore_;
    std::vector<diagnostic> diagnostics_;
    address_expr_collector exprStat_;

    // Our only configuration for now
    configuration cnf_;

    friend struct linter_visitor;
};

}
