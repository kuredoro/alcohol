#pragma once

#include <ast/statements.hpp>
#include <ast/expressions.hpp>

namespace linter
{

struct address_var_collector : public ast::statement_visitor
{
    void process(ast::block&);
    void process(ast::statement&);
    void process(ast::decl&);
    void process(ast::alloc&);
    void process(ast::store&);
    void process(ast::dispose&);
    void process(ast::load&);
    void process(ast::if_else&);
    void process(ast::while_loop&);
};

struct linter
{
    linter() = default;

    const std::vector<std::string>& warnings() const
    {
        return diagnostics_;
    }

    void process(ast::block& block)
    {
        address_var_collector collector;
        block.accept(collector);
    }

private:
    std::vector<std::string> diagnostics_;
};


}
