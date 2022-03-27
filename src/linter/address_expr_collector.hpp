#pragma once

#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <memory>

namespace linter
{

struct address_expr_collector : public ast::statement_visitor
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

    gsl::span<ast::expression*> address_expressions()
    {
        return addrExprs_;
    }

private:
    std::vector<ast::expression*> addrExprs_;
    std::vector<std::unique_ptr<ast::expression>> derivedExprs_;
};

}
