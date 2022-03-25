#pragma once
#include <type_traits>
#include <vector>
#include <utility>
#include <gsl/gsl-lite.hpp>

namespace ast
{

struct statement {};
struct nop final : public statement {};

struct block : public statement
{
    template <class... Statements,
             bool ArgumentsAreStatements = (std::is_base_of<statement, std::remove_pointer_t<Statements>>::value && ...)
    >
    explicit block(Statements&&... statements)
    {
        static_assert(ArgumentsAreStatements, "The constructor arguments should be derived from ast::statement");
    }

private:
    std::vector<statement> statements_;
};

struct if_else final : public statement
{
    template <class TrueBranch,
             bool = std::is_base_of<statement, std::remove_pointer_t<TrueBranch>>::value
    >
    explicit if_else(TrueBranch&& trueBranch) :
        trueBranch_(std::forward<TrueBranch>(trueBranch))
    {}

    template <class TrueBranch,
             class FalseBranch,
             bool = std::is_base_of<statement, std::remove_pointer_t<TrueBranch>>::value,
             bool = std::is_base_of<statement, std::remove_pointer_t<FalseBranch>>::value
    >
    if_else(TrueBranch&& trueBranch, FalseBranch&& falseBranch) :
        trueBranch_(std::forward<TrueBranch>(trueBranch)),
        falseBranch_(std::forward<FalseBranch>(falseBranch))
    {}

private:
    statement trueBranch_{}, falseBranch_{};
};

struct while_loop final : public statement
{
    template <class Statement,
             bool = std::is_base_of<statement, std::remove_pointer_t<Statement>>::value
    >
    explicit while_loop(Statement&& body)
        : body_(std::forward<Statement>(body))
    {}

private:
    statement body_;
};

}
