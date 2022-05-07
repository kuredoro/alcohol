#include <linter/configuration.hpp>

namespace linter
{

std::string configuration::to_string() const
{
    std::string text;
    text += "Current vars: ";
    for (const auto& name : currentVars_)
    {
        if (name != *currentVars_.begin())
        {
            text += ", ";
        }

        text += name;
    }

    text += "\n";
    text += "Current expressions: ";
    for (const auto& expr : currentAddressExprs_)
    {
        if (expr != *currentAddressExprs_.begin())
        {
            text += ", ";
        }

        text += expr->to_string();
    }

    text += "\n";
    text += "Constraints:\n";
    text += constraints_.to_string();

    return text;
}

bool configuration::check_reachability_from(const configuration& old) const
{
    return true;
}

bool configuration::add_var(ast::var* var)
{
    currentVars_.insert(var->name());
    return true;
}

bool configuration::remove_var(ast::var* var)
{
    currentVars_.erase(var->name());
    return true;
}

void configuration::add_array_constraints_for(ast::var*, size_t elemCount)
{

}

void configuration::replace(ast::var* from, ast::expression* to)
{

}

}
