#include "ast/manager.hpp"
#include <ast/expressions.hpp>

namespace ast
{

void expression::accept(expression_visitor& visitor)
{
    visitor.process(*this);
}

bool operator==(const expression& left, const expression& right)
{
    return left.to_string() == right.to_string();
}

bool operator!=(const expression& left, const expression& right)
{
    return !(left == right);
}

}
