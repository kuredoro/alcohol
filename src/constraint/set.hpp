#pragma once

#include <ast/expressions.hpp>

namespace constraint
{

struct set
{
    void add(ast::constraint*) {}
    bool check_satisfiability_with(ast::constraint*) const { return true; }
    bool check_consistency() const;
};

}
