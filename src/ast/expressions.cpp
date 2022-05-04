#include "ast/manager.hpp"
#include <ast/expressions.hpp>

namespace ast
{

void expression::accept(expression_visitor& visitor)
{
    visitor.process(*this);
}

}
