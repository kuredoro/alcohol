#include <linter/linter.hpp>
#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <iostream>

int main()
{
    auto empty =
        ast::block(
            ast::decl("x", ast::integer(0)),
            ast::decl("y", ast::integer(0)),

            ast::alloc("x", 2),
            ast::store(ast::var("x"), ast::integer(1)),
            ast::store(ast::add(ast::var("x"), ast::integer(1)), ast::integer(3)),

            ast::alloc("y", 1),
            ast::store(ast::var("y"), ast::integer(2)),

            ast::dispose("x"),
            ast::dispose("y")
        );


    std::cout << "Program:\n" << empty.to_string() << "\n\nLinter:\n";

    linter::address_var_collector collector;

    collector.process(empty);

    auto exprs = collector.address_expressions();

    std::cout << "\n\nAddress expressions:\n";
    for (auto& expr : exprs)
    {
        std::cout << expr->to_string() << '\n';
    }

    return 0;
}
