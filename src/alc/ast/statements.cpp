#include <alc/ast/statements.hpp>

namespace ast
{

void statement::accept(statement_visitor& visitor)
{
    visitor.process(*this);
}

}
