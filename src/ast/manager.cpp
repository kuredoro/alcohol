#include <ast/manager.hpp>

namespace ast
{

bool manager::same(const ast::expression* a, const ast::expression* b) const
{
    return a->to_string() == b->to_string();
}

}
