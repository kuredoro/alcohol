#include "ast/manager.hpp"
#include <ast/replace_var.hpp>

namespace ast
{

struct replace_var_visitor : public ast::expression_visitor
{
    replace_var_visitor(ast::manager& store, ast::var* target, ast::expression* with) :
        store_(store), target_(target), with_(with)
    {}

    void process(ast::expression& expr) override
    {
        result = &expr;
    }

    void process(ast::var& var) override
    {
        if (var.name() != target_->name())
        {
            result = &var;
            return;
        }

        result = with_;
        return;
    }

    void process(ast::integer& integer) override
    {
        result = &integer;
        return;
    }

    void process(ast::add& sum) override
    {
        auto replacedLeft = replace_var(store_, sum.left(), target_, with_);
        auto replacedRight = replace_var(store_, sum.right(), target_, with_);

        if (store_.same(replacedLeft, sum.left()) && store_.same(replacedRight, sum.right()))
        {
            result = &sum;
            return;
        }

        result = store_.make_expression<ast::add>(replacedLeft, replacedRight);
        return;
        
    }

    void process(ast::multiply&) override {}
    void process(ast::constraint&) override {}

    ast::expression* result;

private:
    ast::manager& store_;
    ast::var* target_;
    ast::expression* with_;
};

ast::expression* replace_var(ast::manager& store, ast::expression* expr, ast::var* target, ast::expression* with)
{
    replace_var_visitor visitor(store, target, with);
    expr->accept(visitor);

    return visitor.result;
}

}
