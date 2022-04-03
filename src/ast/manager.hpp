#pragma once

#include <vector>
#include <string>
#include <memory>

namespace ast
{

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

    virtual std::string to_string() const
    {
        return "<unknow expression>";
    }

    virtual ~expression() = default;
};

struct manager
{
    manager() = default;

    template <class Statement, class... Args>
    Statement* make_statement(Args&&... args)
    {
        static_assert(std::is_base_of_v<statement, Statement>, "ast::manager::make_statement should only create subclasses of ast::statement");

        auto stmt = std::make_unique<Statement>(*this, std::forward<Args>(args)...);
        Statement* ptr = stmt.get();

        statements_.emplace_back(std::move(stmt));
        return ptr;
    }

    template <class Statement>
    Statement* acquire_statement(Statement&& stmt)
    {
        static_assert(std::is_base_of_v<statement, Statement>, "ast::manager::acquire_statement accepts subclasses of ast::statement only");

        auto uptr = std::make_unique<Statement>(std::forward<Statement>(stmt));
        Statement* ptr = uptr.get();

        statements_.emplace_back(std::move(uptr));
        return ptr;
    }

    template <class Expression, class... Args>
    Expression* make_expression(Args&&... args)
    {
        static_assert(std::is_base_of_v<expression, Expression>, "ast::manager::make_expression should only create subclasses of ast::expression");

        auto expr = std::unique_ptr<Expression>(); // std::make_unique<Expression>(*this, std::forward<Args>(args)...);
        Expression* ptr = expr.get();

        expressions_.emplace_back(std::move(expr));
        return ptr;
    }

    template <class Expression>
    Expression* acquire_expression(Expression&& expr)
    {
        static_assert(std::is_base_of_v<expression, Expression>, "ast::manager::acquire_expression accepts subclasses of ast::expression only");

        auto uptr = std::make_unique<Expression>(std::forward<Expression>(expr));
        Expression* ptr = uptr.get();

        expressions_.emplace_back(std::move(uptr));
        return ptr;
    }

private:
    std::vector<std::unique_ptr<ast::statement>> statements_;
    std::vector<std::unique_ptr<ast::expression>> expressions_;
};

}
