#pragma once

#include <alc/ast/manager.hpp>
#include <alc/ast/expressions.hpp>

namespace ast
{

ast::expression* replace_var(ast::manager&, ast::expression* expr, ast::var* var, ast::expression* with);

}
