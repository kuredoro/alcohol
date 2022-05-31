#include "alc/ast/manager.hpp"
#include <alc/constraint/is_trivial.hpp>
#include <optional>

namespace constraint
{

std::optional<ast::constraint::relation> is_trivial(ast::manager& store, ast::expression* a, ast::expression* b)
{
    if (store.same(a, b))
        return ast::constraint::relation::eq;

    auto aAsSum = dynamic_cast<ast::add*>(a);
    auto bAsSum = dynamic_cast<ast::add*>(b);
    if (aAsSum != nullptr && bAsSum != nullptr)
    {
        if (store.same(aAsSum->left(), bAsSum->left()))
        {
            auto aOffset = dynamic_cast<ast::integer*>(aAsSum);
            auto bOffset = dynamic_cast<ast::integer*>(bAsSum);

            if (aOffset != nullptr && bOffset != nullptr && store.same(aOffset, bOffset))
                return ast::constraint::relation::eq;
            else
                return ast::constraint::relation::neq;

        }
    }

    auto aAsInt = dynamic_cast<ast::integer*>(a);
    auto bAsInt = dynamic_cast<ast::integer*>(b);
    if (aAsInt != nullptr && bAsInt != nullptr)
    {
        if (aAsInt->value() == bAsInt->value())
            return ast::constraint::relation::eq;
        else
            return ast::constraint::relation::neq;
    }

    return std::nullopt;
}

}
