#include <ast/collect_vars.hpp>

#include <gsl/gsl-lite.hpp>

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

namespace ast
{

std::vector<ast::var*> collect_vars(ast::expression* root)
{
    var_collector collector;
    root->accept(collector);

    return collector.move_vars();
}

}
