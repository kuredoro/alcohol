#pragma once

#include <ast/expressions.hpp>

#include <z3++.h>

namespace constraint
{

struct set
{
    void add(ast::constraint*);
    bool check_satisfiability_with(ast::constraint*) const { return true; }
    bool check_consistency() const;

private:
    std::vector<ast::constraint*> constraints_;
};

z3::expr to_z3_form(z3::context&, ast::expression*);

namespace detail
{
    struct to_z3_form_visitor : public ast::expression_visitor
    {
        explicit to_z3_form_visitor(z3::context& ctx) : ctx_(ctx) {}

        void process(ast::expression&) override;
        void process(ast::var&) override;
        void process(ast::integer&) override;
        void process(ast::add&) override;
        void process(ast::multiply&) override;
        void process(ast::constraint&) override;

        z3::expr result() const
        {
            return stack_.back();
        }

    private:
        z3::context& ctx_;
        std::vector<z3::expr> stack_;
    };
}

}
