#pragma once

#include <string>
#include <memory>

namespace ast
{

struct expression
{
    expression() = default;
    expression(expression&&) = default;
    expression& operator=(expression&&) = default;

    expression(const expression&) = delete;
    expression& operator=(const expression&) = delete;

    bool operator=(const expression& other) const
    {
        return false;
    }

    virtual std::string to_string() const = 0;

    virtual ~expression() = default;
};


struct var final : public expression
{
    var(const std::string& name) :
        name_(name)
    {}

    std::string to_string() const override
    {
        return name_;
    }

private:
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

expression& replace_variable(expression& expr, const std::string& var, const expression& withExpr);

};
