#include <alc/ast/expressions.hpp>
#include <alc/ast/manager.hpp>
#include <alc/ast/simplify_arithmetic.hpp>

#include <cassert>

namespace ast
{

struct simplify_arithmetic_visitor : public ast::expression_visitor
{
    simplify_arithmetic_visitor(ast::manager& store) :
        store_(store)
    {}

    void process(ast::expression& expr) override
    {
        result = store_.make_expression<ast::add>(
            &expr, store_.make_expression<ast::integer>(0)
        );
    }

    void process(ast::var& var) override
    {
        process(static_cast<ast::expression&>(var));
    }

    void process(ast::integer& integer) override
    {
        result = &integer;
    }

    void process(ast::add& sum) override
    {
        auto simpleLeft = simplify_arithmetic(store_, sum.left());
        auto simpleRight = simplify_arithmetic(store_, sum.right());

        int intSum = 0;

        ast::expression* leftNonInt = nullptr;
        if (auto slInt = dynamic_cast<ast::integer*>(simpleLeft); slInt != nullptr)
        {
            intSum += slInt->value();
        }
        else if (auto slSum = dynamic_cast<ast::add*>(simpleLeft); slSum != nullptr)
        {
            if (auto slSumInt = dynamic_cast<ast::integer*>(slSum->right()); slSumInt != nullptr)
            {
                intSum += slSumInt->value();
            }

            leftNonInt = slSum->left();
        }
        else
        {
            // unreachable
            assert(false);
        }

        ast::expression* rightNonInt = nullptr;
        if (auto srInt = dynamic_cast<ast::integer*>(simpleRight); srInt != nullptr)
        {
            intSum += srInt->value();
        }
        else if (auto srSum = dynamic_cast<ast::add*>(simpleRight); srSum != nullptr)
        {
            if (auto srSumInt = dynamic_cast<ast::integer*>(srSum->right()); srSumInt != nullptr)
            {
                intSum += srSumInt->value();
            }

            rightNonInt = srSum->left();
        }
        else
        {
            // unreachable
            assert(false);
        }

        if (leftNonInt == nullptr && rightNonInt != nullptr)
        {
            std::swap(leftNonInt, rightNonInt);
        }

        auto intSumNode = store_.make_expression<ast::integer>(intSum);
        if (leftNonInt != nullptr && rightNonInt != nullptr)
        {
            auto nonIntSum = store_.make_expression<ast::add>(
                leftNonInt, rightNonInt
            );

            result = store_.make_expression<ast::add>(
                nonIntSum, intSumNode
            );
        }
        else if (leftNonInt != nullptr && rightNonInt == nullptr)
        {
            result = store_.make_expression<ast::add>(
                leftNonInt, intSumNode
            );
        }
        else
        {
            result = intSumNode;
        }
    }

    void process(ast::multiply& product) override
    {
        process(static_cast<ast::expression&>(product));
    }

    void process(ast::constraint&) override {}

    ast::expression* result;

private:
    ast::manager& store_;
    ast::var* target_;
    ast::expression* with_;
};

// Only focuses on simplifying ast::add expressions. It aims to collect all non-integer
// nodes on the left-hand side, and have the accumulated integer on the right.
// Will transform all expressions to form (expr) + <int>.
ast::expression* simplify_arithmetic(ast::manager& store, ast::expression* expr)
{
    simplify_arithmetic_visitor visitor(store);
    expr->accept(visitor);

    return visitor.result;
}

}
