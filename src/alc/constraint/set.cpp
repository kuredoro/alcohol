#include <alc/constraint/set.hpp>

#include <alc/ast/expressions.hpp>

#include <z3++.h>

#include <stdexcept>

namespace constraint
{

std::string set::to_string() const
{
    std::string text;
    for (const auto& c : constraints_)
    {
        text += c->to_string();
        text += "\n";
    }

    return text;
}

void set::add(ast::constraint* expr)
{
    constraints_.push_back(expr);
}

void set::remove_duplicates(ast::manager& store)
{
    for (size_t i = 0; i < constraints_.size(); i++)
    {
        for (size_t j = i + 1; j < constraints_.size(); j++)
        {
            if (store.same(constraints_[i], constraints_[j]))
            {
                constraints_[i] = nullptr;
                break;
            }
        }
    }

    size_t out = 0;
    for (size_t in = 0; in < constraints_.size(); in++)
    {
        if (constraints_[in] == nullptr)
            continue;

        constraints_[out] = constraints_[in];
        out++;
    }

    constraints_.resize(out);
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

bool set::check_satisfiability_of(ast::constraint* target) const
{
    z3::context ctx;
    z3::solver solver(ctx);

    for (auto& c : constraints_)
    {
        solver.add(to_z3_form(ctx, c));
    }

    solver.add(!to_z3_form(ctx, target));

    switch (solver.check())
    {
    case z3::sat:
        return false;
    case z3::unsat:
        return true;
    default:
        throw std::runtime_error("constraint_set: satisfiable_with: z3 failed");
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
        stack_.push_back(ctx_.bool_val(false));
    }

    void to_z3_form_visitor::process(ast::var& var)
    {
        stack_.push_back(ctx_.int_const(var.name().c_str()));
    }

    void to_z3_form_visitor::process(ast::integer& integer)
    {
        stack_.push_back(ctx_.int_val(integer.value()));
    }

    void to_z3_form_visitor::process(ast::add& sum)
    {
        sum.left()->accept(*this);
        z3::expr left = stack_.back();
        stack_.pop_back();

        sum.right()->accept(*this);
        z3::expr right = stack_.back();
        stack_.pop_back();

        stack_.push_back(left + right);
    }

    void to_z3_form_visitor::process(ast::multiply& product)
    {
        product.left()->accept(*this);
        z3::expr left = stack_.back();
        stack_.pop_back();

        product.right()->accept(*this);
        z3::expr right = stack_.back();
        stack_.pop_back();

        stack_.push_back(left * right);
    }

    void to_z3_form_visitor::process(ast::constraint& constraint)
    {
        // TODO: remove this boilerplate
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
