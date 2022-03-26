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

namespace ast
{

using namespace boost::mp11;

struct statement_visitor;

struct statement
{
    statement() = default;
    statement(statement&&) = default;
    statement& operator=(statement&&) = default;

    statement(const statement&) = delete;
    statement& operator=(const statement&) = delete;

    virtual std::string to_string() const
    {
        return "<unknown statement>\n";
    };

    virtual void accept(statement_visitor&);

    virtual ~statement() = default;
};

template <class X, class SubType = statement>
struct visitable_statement : public SubType
{
    virtual void accept(statement_visitor& visitor);
};

struct nop final : public visitable_statement<nop>
{
    std::string to_string() const override
    {
        return "nop\n";
    }
};

struct decl final : public visitable_statement<decl>
{
    template <class Expression>
    decl(const std::string& varName, Expression&& expr) :
        varName_(varName), expr_(std::make_unique<Expression>(std::move(expr)))
    {}

    std::string to_string() const override
    {
        return "let " + varName_ + " := " + expr_->to_string() + "\n";
    }

private:
    std::string varName_;
    std::unique_ptr<expression> expr_;
};

struct assign final : public visitable_statement<assign>
{
    template <class Expression>
    assign(const std::string& varName, Expression&& expr) :
        varName_(varName), expr_(std::make_unique<Expression>(std::move(expr)))
    {}

    std::string to_string() const override
    {
        return varName_ + " := " + expr_->to_string() + "\n";
    }

private:
    std::string varName_;
    std::unique_ptr<expression> expr_;
};

struct alloc final : public visitable_statement<alloc>
{
    alloc(const std::string& varName, size_t size) :
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

    // TODO: std::string seems not optimal, is there copy elission, I'm pretty sure,
    // but gotta check...
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
    store(LHS&& address, RHS&& value) :
        destination_(std::make_unique<LHS>(std::forward<LHS>(address))),
        value_(std::make_unique<RHS>(std::forward<RHS>(value)))
    {}

    std::string to_string() const override
    {
        return "*" + destination_->to_string() + " := " + value_->to_string() + "\n";
    }

    expression* destination() const
    {
        return destination_.get();
    }

private:
    std::unique_ptr<expression> destination_, value_;
};

struct load final : public visitable_statement<load>
{
    load(const std::string& toVar, const std::string& fromVar) :
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
    explicit dispose(const std::string& varName) :
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
    explicit block(Statements&&... statements)
    {
        static_assert(ArgumentsAreStatements, "The constructor arguments should be derived from ast::statement");

        statements_.resize(sizeof...(statements));

        auto&& insert = [this](size_t i, auto&& stmt)
        {
            statements_[i] = util::make_unique_from_ref<decltype(stmt)>(std::forward<decltype(stmt)>(stmt));
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

    gsl::span<const std::unique_ptr<statement>> statements() const
    {
        return gsl::make_span(statements_);
    }

private:
    std::vector<std::unique_ptr<statement>> statements_;
};

struct if_else final : public visitable_statement<if_else>
{
    template <class TrueBranch,
             bool TrueBranchIsStatement = std::is_base_of<statement, std::remove_pointer_t<TrueBranch>>::value
    >
    explicit if_else(TrueBranch&& trueBranch) :
        trueBranch_(std::make_unique<TrueBranch>(std::forward<TrueBranch>(trueBranch)))
    {
        static_assert(TrueBranchIsStatement, "the true branch must be a statement");
    }

    template <class TrueBranch,
             class FalseBranch,
             bool TrueBranchIsStatement = std::is_base_of<statement, std::remove_pointer_t<TrueBranch>>::value,
             bool FalseBranchIsStatement = std::is_base_of<statement, std::remove_pointer_t<FalseBranch>>::value
    >
    if_else(TrueBranch&& trueBranch, FalseBranch&& falseBranch) :
        trueBranch_(std::make_unique<TrueBranch>(std::forward<TrueBranch>(trueBranch))),
        falseBranch_(std::make_unique<FalseBranch>(std::forward<FalseBranch>(falseBranch)))
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
    std::unique_ptr<statement> trueBranch_{}, falseBranch_{};
};

struct while_loop final : public visitable_statement<while_loop>
{
    template <class Body,
             bool BodyIsStatement = std::is_base_of<statement, std::remove_pointer_t<Body>>::value
    >
    explicit while_loop(Body&& body)
        : body_(std::make_unique<Body>(std::forward<Body>(body)))
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
    std::unique_ptr<statement> body_;
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
