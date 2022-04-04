#pragma once

#include <ast/manager.hpp>

#include <string>
#include <memory>
#include <unordered_map>

namespace ast
{

struct var final : public expression
{
    var(manager&, const std::string& name) :
        name_(name)
    {}

    std::string to_string() const override
    {
        return name_;
    }

    std::string name() const
    {
        return name_;
    }

private:
    // TODO: replace with faster version
    std::string name_;
};

struct integer final : public expression
{
    integer(manager&, int val) :
        val_(val)
    {}

    std::string to_string() const override
    {
        return std::to_string(val_);
    }

private:
    int val_;
};

struct add final : public expression
{
    // TODO: add concepts
    template <class LHS, class RHS>
    add(manager& store, LHS&& left, RHS&& right) :
        left_(store.acquire_expression(std::forward<LHS>(left))),
        right_(store.acquire_expression(std::forward<RHS>(right)))
    {}

    std::string to_string() const override
    {
        return "(" + left_->to_string() + " + " + right_->to_string() + ")";
    }

private:
    expression* left_;
    expression* right_;
};

struct multiply final : public expression
{
    // TODO: add concepts
    template <class LHS, class RHS>
    multiply(manager& store, LHS&& left, RHS&& right) :
        left_(store.acquire_expression(std::forward<LHS>(left))),
        right_(store.acquire_expression(std::forward<RHS>(right)))
    {}

    std::string to_string() const override
    {
        return "(" + left_->to_string() + " * " + right_->to_string() + ")";
    }


private:
    expression* left_;
    expression* right_;
};

bool operator==(const expression& left, const expression& right);
bool operator!=(const expression& left, const expression& right);

expression* replace_variable(manager& store, expression* expr, const std::string& var, const expression* withExpr);

};
