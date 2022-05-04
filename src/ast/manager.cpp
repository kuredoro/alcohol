#include <ast/manager.hpp>

namespace ast
{

bool manager::same(ast::expression* a, ast::expression* b) const
{
    return a->to_string() == b->to_string();
}

}
