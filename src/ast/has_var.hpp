#pragma once

#include <ast/manager.hpp>
#include <ast/expressions.hpp>

namespace ast
{

bool has_var(ast::expression* expr, ast::var* target);

}
