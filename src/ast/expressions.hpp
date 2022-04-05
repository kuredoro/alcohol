#pragma once

#include <ast/manager.hpp>

#include <string>
#include <memory>
#include <unordered_map>

namespace ast
{

template <class X, class SubType = expression>
struct visitable_expression : public SubType
{
    virtual void accept(expression_visitor& visitor);
};

struct var final : public visitable_expression<var>
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

struct integer final : public visitable_expression<integer>
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

struct add final : public visitable_expression<add>
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

struct multiply final : public visitable_expression<multiply>
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

struct expression_visitor
{
    virtual void process(expression&) = 0;
    virtual void process(var&) = 0;
    virtual void process(integer&) = 0;
    virtual void process(add&) = 0;
    virtual void process(multiply&) = 0;
};

template <class X, class SubType>
void visitable_expression<X, SubType>::accept(expression_visitor& visitor)
{
    visitor.process(static_cast<X&>(*this));
}

bool operator==(const expression& left, const expression& right);
bool operator!=(const expression& left, const expression& right);

expression* replace_variable(manager& store, expression* expr, const std::string& var, const expression* withExpr);

};
