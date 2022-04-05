#pragma once

#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <ast/manager.hpp>
#include <memory>

namespace linter
{

struct address_expr_collector : public ast::statement_visitor
{
    address_expr_collector(ast::manager& astStore) :
        astStore_(astStore)
    {}

    void process(ast::block&);
    void process(ast::statement&);
    void process(ast::decl&);
    void process(ast::assign&);
    void process(ast::alloc&);
    void process(ast::store&);
    void process(ast::dispose&);
    void process(ast::load&);
    void process(ast::if_else&);
    void process(ast::while_loop&);

    gsl::span<ast::var*> address_variables()
    {
        return addrVars_;
    }

    gsl::span<ast::expression*> address_expressions()
    {
        return addrExprs_;
    }

private:
    std::vector<ast::var*> addrVars_;
    std::vector<ast::expression*> addrExprs_;

    ast::manager& astStore_;
};

}
