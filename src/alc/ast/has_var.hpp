#pragma once

#include <alc/ast/manager.hpp>
#include <alc/ast/expressions.hpp>

namespace ast
{

bool has_var(ast::expression* expr, ast::var* target);

}
