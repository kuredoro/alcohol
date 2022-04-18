#include <constraint/set.hpp>

#include <ast/expressions.hpp>

#include <z3++.h>

#include <stdexcept>

namespace constraint
{

void set::add(ast::constraint* expr)
{
    constraints_.push_back(expr);
}

bool set::check_consistency() const
{
    z3::context ctx;
    z3::solver solver(ctx);

    for (auto& c : constraints_)
    {
        solver.add(to_z3_form(ctx, c));
    }

    switch (solver.check())
    {
    case z3::sat:
        return true;
    case z3::unsat:
        return false;
    default:
        throw std::runtime_error("constraint_set: check_consistency: z3 failed");
        return false;
    }
}

z3::expr to_z3_form(z3::context& c, ast::expression* expr)
{
    detail::to_z3_form_visitor visitor(c);
    expr->accept(visitor);

    return visitor.result();
}

namespace detail
{
    void to_z3_form_visitor::process(ast::expression&)
    {
        stack_.push_back(ctx_.int_const("foo") == ctx_.int_const("foo"));
    }

    void to_z3_form_visitor::process(ast::var& var)
    {
        process(static_cast<ast::expression&>(var));
    }

    void to_z3_form_visitor::process(ast::integer& integer)
    {
        stack_.push_back(ctx_.int_val(integer.value()));
    }

    void to_z3_form_visitor::process(ast::add& sum)
    {
        process(static_cast<ast::expression&>(sum));
    }

    void to_z3_form_visitor::process(ast::multiply& product)
    {
        process(static_cast<ast::expression&>(product));
    }

    void to_z3_form_visitor::process(ast::constraint& constraint)
    {
        constraint.left()->accept(*this);
        z3::expr left = stack_.back();
        stack_.pop_back();

        constraint.right()->accept(*this);
        z3::expr right = stack_.back();
        stack_.pop_back();

        switch (constraint.kind())
        {
        case ast::constraint::relation::eq:
            stack_.push_back(left == right);
            break;
        case ast::constraint::relation::neq:
            stack_.push_back(left != right);
            break;
        default:
            break;
        }
    }

}

}
