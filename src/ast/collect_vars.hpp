#pragma once

#include <ast/expressions.hpp>

#include <vector>

namespace ast
{

std::vector<ast::var*> collect_vars(ast::expression* root);

}
