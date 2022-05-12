#include <ast/expressions.hpp>
#include <ast/manager.hpp>
#include <ast/replace_var.hpp>
#include <ast/statements.hpp>
#include <iomanip>
#include <linter/linter.hpp>

namespace linter
{

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

            std::cout << "  " << stmt->to_string();
            std::cout << linter_.cnf_.to_string() << "\n";
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
    }

    void process(ast::assign& assignment) override
    {
        // Skip, if the variable does not influence the pointers.
        // TODO

        // Required: just replace the variable with expression
        linter_.cnf_.replace(assignment.destination(), assignment.value());

        constraint::set newSet;
        auto addrExprs = linter_.exprStat_.address_expressions();
        for (size_t i = 0; i < addrExprs.size(); i++)
        {
            auto& e1 = addrExprs[i];
            for (size_t j = i; j < addrExprs.size(); j++)
            {
                auto& e2 = addrExprs[j];

                auto e1Replaced = ast::replace_var(linter_.astStore_, e1, assignment.destination(), assignment.value());
                auto e2Replaced = ast::replace_var(linter_.astStore_, e2, assignment.destination(), assignment.value());

                auto eq = linter_.astStore_.make_expression<ast::constraint>(
                    ast::constraint::relation::eq, e1Replaced, e2Replaced
                );

                bool valid = linter_.cnf_.constraints().check_satisfiability_of(eq);
                std::cout << "      Trying to prove " << eq->to_string() << " :: " << std::boolalpha << valid << '\n';
                if (valid)
                {
                    newSet.add(linter_.astStore_.make_expression<ast::constraint>(
                        ast::constraint::relation::eq, e1, e2
                    ));
                    continue;
                }

                auto neq = linter_.astStore_.make_expression<ast::constraint>(
                    ast::constraint::relation::neq, e1Replaced, e2Replaced
                );

                valid = linter_.cnf_.constraints().check_satisfiability_of(neq);
                std::cout << "      Trying to prove " << neq->to_string() << " :: " << std::boolalpha << valid << '\n';
                if (valid)
                {
                    newSet.add(linter_.astStore_.make_expression<ast::constraint>(
                        ast::constraint::relation::neq, e1, e2
                    ));
                    continue;
                }
            }
        }

        auto reachable = linter_.cnf_.check_reachability_from(linter_.astStore_, assignment.destination(), assignment.value());
        if (!reachable)
        {
            std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Memory leak detected\n";
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

        constraint::set newConstraints;
        auto addrExprs = linter_.exprStat_.address_expressions();
        for (size_t i = 0; i < addrExprs.size(); i++)
        {
            auto& e1 = addrExprs[i];
            for (size_t j = i; j < addrExprs.size(); j++)
            {
                auto& e2 = addrExprs[j];

                auto e1Replaced = ast::replace_var(linter_.astStore_, e1, alloc.destination_var(), tmpVar);
                auto e2Replaced = ast::replace_var(linter_.astStore_, e2, alloc.destination_var(), tmpVar);

                auto eq = linter_.astStore_.make_expression<ast::constraint>(
                    ast::constraint::relation::eq, e1Replaced, e2Replaced
                );

                bool valid = augmentedConstraints.constraints().check_satisfiability_of(eq);
                std::cout << "      Trying to prove " << eq->to_string() << " :: " << std::boolalpha << valid << '\n';
                if (valid)
                {
                    newConstraints.add(linter_.astStore_.make_expression<ast::constraint>(
                        ast::constraint::relation::eq, e1, e2
                    ));
                    continue;
                }

                auto neq = linter_.astStore_.make_expression<ast::constraint>(
                    ast::constraint::relation::neq, e1Replaced, e2Replaced
                );

                valid = augmentedConstraints.constraints().check_satisfiability_of(neq);
                std::cout << "      Trying to prove " << neq->to_string() << " :: " << std::boolalpha << valid << '\n';
                if (valid)
                {
                    newConstraints.add(linter_.astStore_.make_expression<ast::constraint>(
                        ast::constraint::relation::neq, e1, e2
                    ));
                    continue;
                }
            }
        }

        linter_.cnf_.constraints(newConstraints);
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
            std::cout << "!!!!!!!!!!!!!!!! Double free of " << dispose.target_var()->name() << "\n";
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

    std::cout << "ADDR VARS: ";
    for (auto& v : exprStat_.address_variables())
    {
        std::cout << v->to_string() << " ";
    }

    std::cout << "\nADDR EXPRS: ";
    for (auto& e : exprStat_.address_expressions())
    {
        std::cout << e->to_string() << ", ";
    }

    std::cout << "\n------\n";

    linter_visitor v(*this);
    block.accept(v);
}

}
