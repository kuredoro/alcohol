#include "ast/manager.hpp"
#include <ast/replace_var.hpp>

namespace ast
{

ast::expression* replace_var(ast::manager& store, ast::expression* expr, ast::var* destVar, ast::expression* with)
{
    if (auto var = dynamic_cast<ast::var*>(expr))
    {
        return with;
    }

    return expr;
}

}
