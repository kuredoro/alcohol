#pragma once

#include <string>
#include <memory>

namespace ast
{

struct expression
{
    bool operator=(const expression& other) const
    {
        return false;
    }

    virtual std::string to_string() const = 0;

    virtual std::unique_ptr<expression> clone() const = 0;
};


template <class Expr>
struct clonable_expression : public expression
{
    std::unique_ptr<expression> clone() const override
    {
        return std::make_unique<Expr>(*static_cast<const Expr*>(this));
    }
};

struct var final : public clonable_expression<var>
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

struct integer final : public clonable_expression<integer>
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

struct add final : public clonable_expression<add>
{

    add(std::unique_ptr<expression>&& left, std::unique_ptr<expression>&& right) :
        left_(std::move(left)), right_(std::move(right))
    {}

    add(const add& other) :
        left_(other.left_->clone()), right_(other.right_->clone())
    {}

    std::string to_string() const override
    {
        return "(" + left_->to_string() + " + " + right_->to_string() + ")";
    }

private:
    std::unique_ptr<expression> left_;
    std::unique_ptr<expression> right_;
};

expression& replace_variable(expression& expr, const std::string& var, const expression& withExpr);

};
