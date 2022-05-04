#pragma once

#include <ast/manager.hpp>

#include <boost/mp11/integral.hpp>
#include <boost/mp11/utility.hpp>

#include <string>
#include <memory>
#include <type_traits>
#include <unordered_map>

namespace ast
{

using namespace boost::mp11;

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

    int value() const
    {
        return val_;
    }

private:
    int val_;
};

struct add final : public visitable_expression<add>
{
    // TODO: add concepts
    template <
        class LHS,
        class RHS,
        bool = mp_if_c<!std::is_pointer_v<LHS>, mp_true, void>::value,
        bool = mp_if_c<!std::is_pointer_v<RHS>, mp_true, void>::value
    >
    add(manager& store, LHS&& left, RHS&& right) :
        left_(store.acquire_expression(std::forward<LHS>(left))),
        right_(store.acquire_expression(std::forward<RHS>(right)))
    {}

    add(manager& store, ast::expression* left, ast::expression* right) :
        left_(left),
        right_(right)
    {}

    std::string to_string() const override
    {
        return "(" + left_->to_string() + " + " + right_->to_string() + ")";
    }

    expression* left() const
    {
        return left_;
    }

    expression* right() const
    {
        return right_;
    }

private:
    expression* left_;
    expression* right_;
};

struct multiply final : public visitable_expression<multiply>
{
    // TODO: add concepts
    template <
        class LHS,
        class RHS,
        bool = mp_if_c<!std::is_pointer_v<LHS>, mp_true, void>::value,
        bool = mp_if_c<!std::is_pointer_v<RHS>, mp_true, void>::value
    >
    multiply(manager& store, LHS&& left, RHS&& right) :
        left_(store.acquire_expression(std::forward<LHS>(left))),
        right_(store.acquire_expression(std::forward<RHS>(right)))
    {}

    multiply(manager& store, ast::expression* left, ast::expression* right) :
        left_(left),
        right_(right)
    {}

    std::string to_string() const override
    {
        return "(" + left_->to_string() + " * " + right_->to_string() + ")";
    }

    expression* left() const
    {
        return left_;
    }

    expression* right() const
    {
        return right_;
    }

private:
    expression* left_;
    expression* right_;
};

struct constraint final : public visitable_expression<constraint>
{
    enum class relation
    {
        eq, neq
    };

    template <
        class LHS,
        class RHS,
        bool = mp_if_c<!std::is_pointer_v<LHS>, mp_true, void>::value,
        bool = mp_if_c<!std::is_pointer_v<RHS>, mp_true, void>::value
    >
    constraint(manager& store, relation op, LHS&& lhs, RHS&& rhs) :
        kind_(op),
        left_(store.acquire_expression(std::forward<LHS>(lhs))),
        right_(store.acquire_expression(std::forward<RHS>(rhs)))
    {}

    constraint(manager& store, relation op, expression* lhs, expression* rhs) :
        kind_(op),
        left_(lhs),
        right_(rhs)
    {}

    std::string to_string() const override
    {
        std::string opStr = "<?>";
        switch (kind_)
        {
        case relation::eq:
            opStr = "==";
            break;
        case relation::neq:
            opStr = "!=";
            break;
        default:
            break;
        }

        return left_->to_string() + " " + opStr + " " + right_->to_string();
    }

    relation kind() const
    {
        return kind_;
    }

    expression* left() const
    {
        return left_;
    }

    expression* right() const
    {
        return right_;
    }

private:
    relation kind_;
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
    virtual void process(constraint&) = 0;
};

template <class X, class SubType>
void visitable_expression<X, SubType>::accept(expression_visitor& visitor)
{
    visitor.process(static_cast<X&>(*this));
}

};
