#pragma once

#include <ast/manager.hpp>
#include <ast/expressions.hpp>
#include <constraint/set.hpp>

#include <set>
#include <string>

namespace linter
{

struct configuration
{

    bool check_reachability_from(ast::manager& store, ast::var* from, ast::expression* to) /*const*/;

    bool add_var(ast::var*);
    bool remove_var(ast::var*);
    void add_array_constraints_for(ast::manager&, ast::var*, size_t elemCount);
    void replace(ast::var* from, ast::expression* to);

    std::string to_string() const;

private:
    std::set<std::string> currentVars_;
    std::set<ast::expression*> currentAddressExprs_;
    constraint::set constraints_;
    std::vector<ast::expression*> current_address_exprs(ast::manager& store);
};

}
