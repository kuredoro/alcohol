#include <ast/expressions.hpp>
#include <ast/manager.hpp>
#include <ast/has_var.hpp>

namespace ast
{

struct has_var_visitor : public ast::expression_visitor
{
    has_var_visitor(ast::var* target) :
        target_(target)
    {}

    void process(ast::expression& expr) override
    {
        result = false;
    }

    void process(ast::var& var) override
    {
        result = (var.name() == target_->name());
    }

    void process(ast::integer& integer) override
    {
        result = false;
    }

    void process(ast::add& sum) override
    {
        result = (has_var(sum.left(), target_) || has_var(sum.right(), target_));
    }

    void process(ast::multiply& product) override
    {
        result = (has_var(product.left(), target_) || has_var(product.right(), target_));
    }

    void process(ast::constraint& c) override
    {
        result = (has_var(c.left(), target_) || has_var(c.right(), target_));

    }

    bool result;

private:
    ast::var* target_;
};

bool has_var(ast::expression* expr, ast::var* target)
{
    has_var_visitor visitor(target);
    expr->accept(visitor);

    return visitor.result;
}

}
