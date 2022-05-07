#include <ast/statements.hpp>
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

    }

    void process(ast::assign& assignment) override
    {
        // Skip, if the variable does not influence the pointers.
        // TODO

        // Required: just replace the variable with expression
        
        // BONUS: check reachability
    }

    void process(ast::alloc& alloc) override
    {
        // TODO: find out what proveFrom means

        // Required: replace var with a temporary
        // Maybe: amend current address expressions



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

    linter_visitor v(*this);
    block.accept(v);
}

}
