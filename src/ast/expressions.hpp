#pragma once

#include <ast/manager.hpp>

#include <string>
#include <memory>
#include <unordered_map>

namespace ast
{

struct var final : public expression
{
    var(const std::string& name) :
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
    integer(int val) :
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
    add(LHS&& left, RHS&& right) :
        left_(std::make_unique<LHS>(std::forward<LHS>(left))),
        right_(std::make_unique<RHS>(std::forward<RHS>(right)))
    {}

    std::string to_string() const override
    {
        return "(" + left_->to_string() + " + " + right_->to_string() + ")";
    }

private:
    std::unique_ptr<expression> left_;
    std::unique_ptr<expression> right_;
};

struct multiply final : public expression
{
    // TODO: add concepts
    template <class LHS, class RHS>
    multiply(LHS&& left, RHS&& right) :
        left_(std::make_unique<LHS>(std::forward<LHS>(left))),
        right_(std::make_unique<RHS>(std::forward<RHS>(right)))
    {}

    std::string to_string() const override
    {
        return "(" + left_->to_string() + " * " + right_->to_string() + ")";
    }


private:
    std::unique_ptr<expression> left_, right_;
};

bool operator==(const expression& left, const expression& right);
bool operator!=(const expression& left, const expression& right);

expression& replace_variable(expression& expr, const std::string& var, const expression& withExpr);

};
