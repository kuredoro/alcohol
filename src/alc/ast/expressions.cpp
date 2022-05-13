#include <alc/ast/manager.hpp>
#include <alc/ast/expressions.hpp>

namespace ast
{

void expression::accept(expression_visitor& visitor)
{
    visitor.process(*this);
}

}
