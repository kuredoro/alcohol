#pragma once

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/integral.hpp>
#include <memory>
#include <type_traits>
#include <vector>
#include <utility>
#include <gsl/gsl-lite.hpp>
#include <boost/mp11.hpp>

#include <util/make_unique_from_ref.hpp>
#include <util/for_each_argument.hpp>
#include <util/indent_text.hpp>
#include <ast/expression.hpp>

namespace ast
{

using namespace boost::mp11;

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

    virtual ~statement() = default;
};

struct nop final : public statement 
{
    std::string to_string() const override
    {
        return "nop\n";
    }
};

struct decl final : public statement
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

struct assign final : public statement
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

struct alloc final : public statement
{
    alloc(const std::string& varName, size_t size) :
        varName_(varName), allocSize_(size)
    {}

    std::string to_string() const override
    {
        return "let " + varName_ + " := alloc(" + std::to_string(allocSize_) + ")\n";
    }

private:
    std::string varName_;
    size_t allocSize_;
};

struct store final : public statement
{
    template <class Expression>
    store(const std::string& toVar, Expression&& value) :
        toVar_(toVar), value_(std::make_unique<Expression>(std::move(value)))
    {}

    std::string to_string() const override
    {
        return "*" + toVar_ + " := " + value_->to_string() + "\n";
    }

private:
    std::string toVar_;
    std::unique_ptr<expression> value_;
};

struct load final : public statement
{
    load(const std::string& toVar, const std::string& fromVar) :
        toVar_(toVar), fromVar_(fromVar)
    {}

    std::string to_string() const override
    {
        return toVar_ + " := *" + fromVar_ + "\n";
    }

private:
    std::string toVar_, fromVar_;
};

struct dispose final : public statement
{
    explicit dispose(const std::string& varName) :
        varName_(varName)
    {}

    std::string to_string() const override
    {
        return "dispose "+ varName_ + "\n";
    }

private:
    std::string varName_;
};

struct block final : public statement
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

private:
    std::vector<std::unique_ptr<statement>> statements_;
};

struct if_else final : public statement
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

struct while_loop final : public statement
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

}
