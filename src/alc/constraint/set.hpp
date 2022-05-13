#pragma once

#include <alc/ast/expressions.hpp>

#include <z3++.h>
#include <gsl/gsl-lite.hpp>

#include <string>

namespace constraint
{

struct set
{
    void add(ast::constraint*);
    bool check_satisfiability_of(ast::constraint*) const;
    bool check_consistency() const;

    std::string to_string() const;

    gsl::span<ast::constraint*> get() {
        return constraints_;
    }

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
