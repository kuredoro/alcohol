#include <alc/ast/collect_vars.hpp>
#include <alc/ast/expressions.hpp>
#include <alc/ast/manager.hpp>
#include <alc/ast/replace_var.hpp>
#include <alc/ast/collect_vars.hpp>
#include <alc/ast/statements.hpp>
#include <alc/linter/linter.hpp>
#include <spdlog/spdlog.h>

namespace linter
{

std::vector<ast::expression*> filter_expressions_with_vars(const gsl::span<ast::expression*> exprs, const std::set<std::string>& wantVars)
{
    std::vector<ast::expression*> filteredExprs;

    for (auto& e : exprs)
    {
        auto exprVars = ast::collect_vars(e);
        
        bool hasUnwantedVars = false;
        for (const auto& var : exprVars)
        {
            bool found = wantVars.find(var->name()) != wantVars.end();
            hasUnwantedVars = hasUnwantedVars || !found;

            if (hasUnwantedVars)
                break;
        }

        if (hasUnwantedVars)
            continue;

        filteredExprs.push_back(e);
    }

    return filteredExprs;
}

constraint::set infer_after_replacement(ast::manager& store, const constraint::set& constraints, const gsl::span<ast::expression*> addrExprs, ast::var* from, ast::expression* to)
{
    constraint::set newSet;
    for (size_t i = 0; i < addrExprs.size(); i++)
    {
        auto& e1 = addrExprs[i];
        for (size_t j = i; j < addrExprs.size(); j++)
        {
            auto& e2 = addrExprs[j];

            auto e1Replaced = ast::replace_var(store, e1, from, to);
            auto e2Replaced = ast::replace_var(store, e2, from, to);

            auto eq = store.make_expression<ast::constraint>(
                ast::constraint::relation::eq, e1Replaced, e2Replaced
            );

            bool valid = constraints.check_satisfiability_of(eq);
            spdlog::trace("      Trying to prove {} :: {}", eq->to_string(), valid);
            if (valid)
            {
                newSet.add(store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq, e1, e2
                ));
                continue;
            }

            auto neq = store.make_expression<ast::constraint>(
                ast::constraint::relation::neq, e1Replaced, e2Replaced
            );

            valid = constraints.check_satisfiability_of(neq);
            spdlog::trace("      Trying to prove {} :: {}", neq->to_string(), valid);

            if (valid)
            {
                newSet.add(store.make_expression<ast::constraint>(
                    ast::constraint::relation::neq, e1, e2
                ));
                continue;
            }
        }
    }

    return newSet;
}

struct linter_visitor : public ast::statement_visitor
{
    linter_visitor(linter& linter) :
        linter_(linter)
    {}

    void process(ast::statement&) override
    {

    }

    void process(ast::block& block) override
    {
        for (auto& stmt : block.statements())
        {
            stmt->accept(*this);

            spdlog::info("\n  {}{}\n", stmt->to_string(), linter_.cnf_.to_string());
        }
    }

    void process(ast::decl& decl) override
    {
        // Add a variable to configuration
        // Magic + variable -> expr in set...
        // replace all variables with the expression
        //
        // decl DOES NOT create new address variables!
        //
        // Hmm... So, when we declare variable, the replacement affects
        // the constraint set only if the variable was declared prior.

        linter_.cnf_.add_var(decl.variable());

        auto assignmentPart = linter_.astStore_.make_statement<ast::assign>(
            decl.variable()->name(), decl.value()
        );

        assignmentPart->accept(*this);
    }

    void process(ast::assign& assignment) override
    {
        // Skip, if the variable does not influence the pointers.
        // TODO

        // Required: just replace the variable with expression
        linter_.cnf_.replace(assignment.destination(), assignment.value());

        auto addrExprs = filter_expressions_with_vars(linter_.exprStat_.address_expressions(), linter_.cnf_.current_vars());
        auto newSet = infer_after_replacement(linter_.astStore_, linter_.cnf_.constraints(), addrExprs, assignment.destination(), assignment.value());

        auto reachable = linter_.cnf_.check_reachability_from(linter_.astStore_, assignment.destination(), assignment.value());
        if (!reachable)
        {
            spdlog::debug("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Memory leak detected");
        }
        
        linter_.cnf_.constraints(newSet);
    }

    void process(ast::alloc& alloc) override
    {
        // TODO: find out what proveFrom means

        // Required: replace var with a temporary
        // Maybe: amend current address expressions


        linter_.cnf_.add_var(alloc.destination_var());

        const auto& currentVars = linter_.cnf_.current_vars();

        static size_t tmpCount = 1;
        auto tmpVar = linter_.astStore_.make_expression<ast::var>("@" + std::to_string(tmpCount++));

        auto augmentedConstraints = linter_.cnf_;
        augmentedConstraints.add_array_constraints_for(linter_.astStore_, tmpVar, alloc.alloc_size());

        auto addrExprs = filter_expressions_with_vars(linter_.exprStat_.address_expressions(), linter_.cnf_.current_vars());
        auto newSet = infer_after_replacement(linter_.astStore_, linter_.cnf_.constraints(), addrExprs, alloc.destination_var(), tmpVar);

        linter_.cnf_.constraints(newSet);
    }

    void process(ast::store&) override
    {
        // Identity???
    }

    void process(ast::load&) override
    {

    }

    void process(ast::dispose& dispose) override
    {
        // Check if in current address variables

        // Required: remove var...
        bool removed = linter_.cnf_.remove_var(dispose.target_var());
        if (!removed)
        {
            spdlog::debug("!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Double free of {}", dispose.target_var()->name());
        }
    }

    void process(ast::if_else&) override
    {

    }

    void process(ast::while_loop&) override
    {

    }

private:
    linter& linter_;
};

void linter::process(ast::block& block)
{
    block.accept(exprStat_);

    spdlog::info("ADDR VARS: ");
    for (auto& v : exprStat_.address_variables())
    {
        spdlog::info("{}", v->to_string());
    }

    spdlog::info("ADDR EXPRS: ");
    for (auto& e : exprStat_.address_expressions())
    {
        spdlog::info("{}", e->to_string());
    }

    spdlog::info("-----");

    linter_visitor v(*this);
    block.accept(v);
}

}
