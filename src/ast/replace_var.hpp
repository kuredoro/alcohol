#pragma once

#include <ast/manager.hpp>
#include <ast/expressions.hpp>

namespace ast
{

ast::expression* replace_var(ast::manager&, ast::expression* expr, ast::var* var, ast::expression* with);

}
