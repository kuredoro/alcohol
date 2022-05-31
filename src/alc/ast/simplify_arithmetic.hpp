#pragma once

#include <alc/ast/manager.hpp>
#include <alc/ast/expressions.hpp>

namespace ast
{

ast::expression* simplify_arithmetic(ast::manager&, ast::expression* expr);

}
