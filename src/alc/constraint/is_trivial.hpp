#pragma once

#include <alc/ast/manager.hpp>
#include <alc/ast/expressions.hpp>

#include <optional>

namespace constraint
{

std::optional<ast::constraint::relation> is_trivial(ast::manager&, ast::expression* a, ast::expression* b);

}
