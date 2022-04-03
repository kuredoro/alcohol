#pragma once

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/integral.hpp>
#include <memory>
#include <string_view>
#include <type_traits>
#include <vector>
#include <utility>
#include <gsl/gsl-lite.hpp>
#include <boost/mp11.hpp>

#include <util/make_unique_from_ref.hpp>
#include <util/for_each_argument.hpp>
#include <util/indent_text.hpp>
#include <ast/expressions.hpp>
#include <ast/manager.hpp>

namespace ast
{

using namespace boost::mp11;

template <class X, class SubType = statement>
struct visitable_statement : public SubType
{
    virtual void accept(statement_visitor& visitor);
};

struct nop final : public visitable_statement<nop>
{
    nop() = default;
    explicit nop(manager&) {}

    std::string to_string() const override
    {
        return "nop\n";
    }
};

struct decl final : public visitable_statement<decl>
{
    template <class Expression>
    decl(manager& store, const std::string& varName, Expression&& expr) :
        varName_(varName), expr_(store.acquire_expression(std::forward<Expression>(expr)))
    {}

    std::string to_string() const override
    {
        return "let " + varName_ + " := " + expr_->to_string() + "\n";
    }

private:
    std::string varName_;
    expression* expr_;
};

struct assign final : public visitable_statement<assign>
{
    template <class Expression>
    assign(manager& store, const std::string& varName, Expression&& expr) :
        varName_(varName), expr_(store.acquire_expression(std::forward<Expression>(expr)))
    {}

    std::string to_string() const override
    {
        return varName_ + " := " + expr_->to_string() + "\n";
    }

private:
    std::string varName_;
    expression* expr_;
};

struct alloc final : public visitable_statement<alloc>
{
    alloc(manager&, const std::string& varName, size_t size) :
        destVar_(varName), allocSize_(size)
    {}

    std::string to_string() const override
    {
        return "let " + destVar_.to_string() + " := alloc(" + std::to_string(allocSize_) + ")\n";
    }

    size_t alloc_size() const
    {
        return allocSize_;
    }

    var* destination_var()
    {
        return &destVar_;
    }

private:
    var destVar_;
    size_t allocSize_;
};

struct store final : public visitable_statement<store>
{
    // TODO: expr base check
    template <class LHS, class RHS>
    store(manager& store, LHS&& address, RHS&& value) :
        destination_(store.acquire_expression(std::forward<LHS>(address))),
        value_(store.acquire_expression(std::forward<RHS>(value)))
    {}

    std::string to_string() const override
    {
        return "*" + destination_->to_string() + " := " + value_->to_string() + "\n";
    }

    expression* destination() const
    {
        return destination_;
    }

private:
    expression* destination_;
    expression* value_;
};

struct load final : public visitable_statement<load>
{
    load(manager&, const std::string& toVar, const std::string& fromVar) :
        toVar_(toVar), fromVar_(fromVar)
    {}

    std::string to_string() const override
    {
        return toVar_.to_string() + " := *" + fromVar_.to_string() + "\n";
    }

    var* source()
    {
        return &fromVar_;
    }

private:
    var toVar_, fromVar_;
};

struct dispose final : public visitable_statement<dispose>
{
    explicit dispose(manager&, const std::string& varName) :
        targetVar_(varName)
    {}

    std::string to_string() const override
    {
        return "dispose "+ targetVar_.to_string() + "\n";
    }

    var* target_var()
    {
        return &targetVar_;
    }

private:
    var targetVar_;
};

struct block final : public visitable_statement<block>
{
    template <class... Statements,
             bool ArgumentsAreStatements = (std::is_base_of<statement, std::remove_pointer_t<Statements>>::value && ...)
    >
    explicit block(manager& store, Statements&&... statements)
    {
        static_assert(ArgumentsAreStatements, "The constructor arguments should be derived from ast::statement");

        statements_.resize(sizeof...(statements));

        auto&& insert = [this, &store](size_t i, auto&& stmt)
        {
            statements_[i] = store.acquire_statement<std::remove_reference_t<decltype(stmt)>>(std::forward<decltype(stmt)>(stmt));
        };

        util::for_each_argument(insert, std::forward<Statements>(statements)...);
    }

    std::string to_string() const override
    {
        std::string result = "begin\n";
        for (auto& stmt : statements_)
        {
            result += util::indent_text(stmt->to_string());
        }
        
        return result + "end\n";
    }

    gsl::span<statement* const> statements() const
    {
        return gsl::make_span(statements_);
    }

private:
    std::vector<statement*> statements_;
};

struct if_else final : public visitable_statement<if_else>
{
    template <class TrueBranch,
             bool TrueBranchIsStatement = std::is_base_of<statement, std::remove_pointer_t<TrueBranch>>::value
    >
    if_else(manager& store, TrueBranch&& trueBranch) :
        trueBranch_(store.acquire_statement(std::forward<TrueBranch>(trueBranch)))
    {
        static_assert(TrueBranchIsStatement, "the true branch must be a statement");
    }

    template <class TrueBranch,
             class FalseBranch,
             bool TrueBranchIsStatement = std::is_base_of<statement, std::remove_pointer_t<TrueBranch>>::value,
             bool FalseBranchIsStatement = std::is_base_of<statement, std::remove_pointer_t<FalseBranch>>::value
    >
    if_else(manager& store, TrueBranch&& trueBranch, FalseBranch&& falseBranch) :
        trueBranch_(store.acquire_statement(std::forward<TrueBranch>(trueBranch))),
        falseBranch_(store.acquire_statement(std::forward<FalseBranch>(falseBranch)))
    {
        static_assert(TrueBranchIsStatement, "the true branch must be a statement");
        static_assert(FalseBranchIsStatement, "the false branch must be a statement");
    }

    std::string to_string() const
    {
        std::string result = "if ...\n";
        if (trueBranch_)
            result += util::indent_text(trueBranch_->to_string());

        if (falseBranch_)
            result += "else\n" + util::indent_text(falseBranch_->to_string());

        result += "end_if\n";

        return result;
    }

private:
    statement* trueBranch_{};
    statement* falseBranch_{};
};

struct while_loop final : public visitable_statement<while_loop>
{
    template <class Body,
             bool BodyIsStatement = std::is_base_of<statement, std::remove_pointer_t<Body>>::value
    >
    while_loop(manager& store, Body&& body)
        : body_(store.acquire_statement(std::forward<Body>(body)))
    {
        static_assert(BodyIsStatement, "the while loop body must be a statement");
    }

    std::string to_string() const
    {
        std::string result = "while ...\n";
        result += util::indent_text(body_->to_string());
        return result + "end_while\n";
    }

private:
    statement* body_;
};

struct statement_visitor
{
    virtual void process(statement&) = 0;
    virtual void process(block&) = 0;
    virtual void process(decl&) = 0;
    virtual void process(alloc&) = 0;
    virtual void process(store&) = 0;
    virtual void process(load&) = 0;
    virtual void process(dispose&) = 0;
    virtual void process(if_else&) = 0;
    virtual void process(while_loop&) = 0;
};

template <class X, class SubType>
void visitable_statement<X, SubType>::accept(statement_visitor& visitor)
{
    visitor.process(static_cast<X&>(*this));
}

}
