#include <constraint/set.hpp>
#include <ast/manager.hpp>
#include <ast/expressions.hpp>
#include <ast/replace_var.hpp>
#include <ast/has_var.hpp>
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

std::vector<ast::expression*> configuration::current_address_exprs(ast::manager& store)
{
    std::vector<ast::expression*> allExprs;
    for (auto& c : constraints_.get())
    {
        allExprs.push_back(c->left());
        allExprs.push_back(c->right());
    }

    for (size_t i = 0; i < allExprs.size(); i++)
    {
        if (allExprs[i] == nullptr)
            continue;

        for (size_t j = i + 1; j < allExprs.size(); j++)
        {
            if (store.same(allExprs[i], allExprs[j]))
            {
                allExprs[j] = nullptr;
            }
        }
    }

    std::vector<ast::expression*> exprs;
    for (const auto& e : allExprs)
    {
        if (e == nullptr)
            continue;

        exprs.push_back(e);
    }

    return exprs;
}

bool configuration::check_reachability_from(ast::manager& store, ast::var* from, ast::expression* to)
{
    auto currentExprs = current_address_exprs(store);

    bool hasLeak = false;
    for (auto& e1 : currentExprs)
    {
        bool reachable = true;
        for (auto& e2 : currentExprs)
        {
            auto e2Replaced = ast::replace_var(store, e2, from, to);
            auto eq = store.make_expression<ast::constraint>(
                ast::constraint::relation::eq, e1, e2Replaced
            );

            bool result = constraints_.check_satisfiability_of(eq);
            std::cout << "        Checking reachibility of " << eq->to_string() << " :: " << !result << "\n";

            reachable = reachable && !result;
            if (!reachable)
                break;
        }

        if (!reachable)
            return false;
    }

    return true;
}

bool configuration::add_var(ast::var* var)
{
    currentVars_.insert(var->name());
    currentAddressExprs_.insert(var);
    return true;
}

bool configuration::remove_var(ast::var* var)
{
    currentVars_.erase(var->name());

    auto it = currentAddressExprs_.begin();
    while (it != currentAddressExprs_.end())
    {
        if (ast::has_var(*it, var))
        {
            it = currentAddressExprs_.erase(it);
            continue;
        }
        
        ++it;
    }

    bool oneRemoved = false;

    constraint::set newSet;
    for (auto& c : constraints_.get())
    {
        if (ast::has_var(c, var))
        {
            oneRemoved = true;
            continue;
        }

        newSet.add(c);
    }

    constraints_ = newSet;
    return oneRemoved;
}

void configuration::add_array_constraints_for(ast::manager& store, ast::var* var, size_t elemCount)
{

}

void configuration::replace(ast::var* from, ast::expression* to)
{

}

}
