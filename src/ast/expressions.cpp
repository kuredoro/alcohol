#include <ast/expressions.hpp>

namespace ast
{

bool operator==(const expression& left, const expression& right)
{
    return left.to_string() == right.to_string();
}

bool operator!=(const expression& left, const expression& right)
{
    return !(left == right);
}

}
