#include "ast/manager.hpp"
#include <iostream>
#include <stdexcept>
#include <typeinfo>
#include <typeindex>
#include <memory>

#include <linter/address_expr_collector.hpp>
#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/mpl_list.hpp>
using namespace boost::mp11;

template <class ExpressionA, class ExpressionB>
void push_back_if_absent(ast::manager& store, std::vector<ExpressionA*>& exprs, ExpressionB* newItem)
{
    for (auto& item : exprs)
    {
        if (store.same(item, newItem))
        {
            return;
        }
    }

    exprs.push_back(newItem);
}

struct var_collector : public ast::expression_visitor
{
    void process(ast::expression& expr) override
    {
        throw std::runtime_error("var_collector: encountered an unknown statement type (" + expr.to_string() + ")");
    }

    void process(ast::var& var) override
    {
        vars_.push_back(&var);
    }

    void process(ast::integer& number) override
    {
        // Do nothing
    }

    void process(ast::add& sum) override
    {
        sum.left()->accept(*this);
        sum.right()->accept(*this);
    }

    void process(ast::multiply& product) override
    {
        product.left()->accept(*this);
        product.right()->accept(*this);
    }

    void process(ast::constraint& constraint) override
    {
        // TODO: maybe redirect by default?
        process(static_cast<ast::expression&>(constraint));
    }

    gsl::span<ast::var*> vars()
    {
        return vars_;
    }

    std::vector<ast::var*> move_vars()
    {
        return std::move(vars_);
    }

private:
    std::vector<ast::var*> vars_;
};

std::vector<ast::var*> collect_variables(ast::expression* root)
{
    var_collector collector;
    root->accept(collector);

    return collector.move_vars();
}

namespace linter
{

void address_expr_collector::process(ast::statement& statement)
{
    std::cout << "<unknown statement>\n";
}

void address_expr_collector::process(ast::block& block)
{
    for (auto& statement : block.statements())
    {
        statement->accept(*this);
    }
}

void address_expr_collector::process(ast::decl& decl)
{
    // Infer address expressions if assigned value is a single address variable
    if (auto varValue = dynamic_cast<ast::var*>(decl.value()); varValue && varAllocSizes_.count(varValue->name()) != 0)
    {
        auto sourceVarAllocSize = varAllocSizes_[varValue->name()];
        varAllocSizes_[decl.variable()->name()] = sourceVarAllocSize;
        for (size_t i = 1; i < sourceVarAllocSize; i++)
        {
            auto root = astStore_.make_expression<ast::add>(ast::var(astStore_, decl.variable()->name()), ast::integer(astStore_, i));
            push_back_if_absent(astStore_, addrExprs_, root);
        }
    }

    auto vars = collect_variables(decl.value());
    for (auto& var : vars)
    {
        for (auto& addrVar : addrVars_)
        {
            if (var->name() == addrVar->name())
            {
                push_back_if_absent(astStore_, addrVars_, decl.variable());
                push_back_if_absent(astStore_, addrExprs_, decl.variable());
                return;
            }
        }
    }
}

void address_expr_collector::process(ast::assign& assignment)
{
    // RHS is an address expression if being assigned to an address variable
    ast::var* dest = assignment.destination();
    for (auto& addrVar : addrVars_)
    {
        if (addrVar->name() == dest->name())
        {
            addrExprs_.push_back(assignment.value());
            break;
        }
    }

    // Infer address expressions if assigned value is a single address variable
    if (auto varValue = dynamic_cast<ast::var*>(assignment.value()); varValue && varAllocSizes_.count(varValue->name()) != 0)
    {
        auto sourceVarAllocSize = varAllocSizes_[varValue->name()];
        varAllocSizes_[dest->name()] = sourceVarAllocSize;
        for (size_t i = 1; i < sourceVarAllocSize; i++)
        {
            auto root = astStore_.make_expression<ast::add>(ast::var(astStore_, assignment.destination()->name()), ast::integer(astStore_, i));
            addrExprs_.push_back(root);
        }
    }

    // Mark destination as an address variable if an expression contains an address variable
    auto vars = collect_variables(assignment.value());
    for (auto& var : vars)
    {
        for (auto& addrVar : addrVars_)
        {
            if (addrVar->name() == var->name())
            {
                push_back_if_absent(astStore_, addrVars_, assignment.destination());
                push_back_if_absent(astStore_, addrExprs_, assignment.destination());
                return;
            }
        }
    }
}

void address_expr_collector::process(ast::alloc& alloc)
{
    push_back_if_absent(astStore_, addrVars_, alloc.destination_var());
    push_back_if_absent(astStore_, addrExprs_, alloc.destination_var());

    varAllocSizes_[alloc.destination_var()->name()] = alloc.alloc_size();

    for (size_t i = 1; i < alloc.alloc_size(); i++)
    {
        // TODO: test manager not to return nullptrs...
        auto root = astStore_.make_expression<ast::add>(ast::var(astStore_, alloc.destination_var()->name()), ast::integer(astStore_, i));
        push_back_if_absent(astStore_, addrExprs_, root);
    }
}

void address_expr_collector::process(ast::store& store)
{
    auto destPlace = store.destination();
    push_back_if_absent(astStore_, addrExprs_, destPlace);

    auto vars = collect_variables(destPlace);
    for (auto& var : vars)
    {
        push_back_if_absent(astStore_, addrVars_,var);
        push_back_if_absent(astStore_, addrExprs_, var);
    }
}

void address_expr_collector::process(ast::load& load)
{
    auto sourcePlace = load.source();
    push_back_if_absent(astStore_, addrExprs_, sourcePlace);
    
    auto vars = collect_variables(sourcePlace);
    for (auto& var : vars)
    {
        push_back_if_absent(astStore_, addrVars_, var);
        push_back_if_absent(astStore_, addrExprs_, var);
    }
}

void address_expr_collector::process(ast::dispose& dispose)
{
    push_back_if_absent(astStore_, addrVars_, dispose.target_var());
    push_back_if_absent(astStore_, addrExprs_, dispose.target_var());
}

void address_expr_collector::process(ast::if_else& ifElse)
{
    ifElse.true_branch()->accept(*this);

    if (auto falseBranch = ifElse.false_branch())
    {
        (*falseBranch)->accept(*this);
    }
}

void address_expr_collector::process(ast::while_loop& whileLoop)
{
    whileLoop.body()->accept(*this);
}

}
