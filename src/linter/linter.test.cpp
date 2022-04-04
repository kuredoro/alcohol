#include <linter/linter.hpp>
#include <ast/manager.hpp>
#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <iostream>

int main()
{
    ast::manager store;

    auto empty =
        ast::block(store,
            ast::decl(store, "x", ast::integer(store, 0)),
            ast::decl(store, "y", ast::integer(store, 0)),

            ast::alloc(store, "x", 2),
            ast::store(store, ast::var(store, "x"), ast::integer(store, 1)),
            ast::store(store, ast::add(store, ast::var(store, "x"), ast::integer(store, 1)), ast::integer(store, 3)),

            ast::alloc(store, "y", 1),
            ast::store(store, ast::var(store, "y"), ast::integer(store, 2)),

            ast::dispose(store, "x"),
            ast::dispose(store, "y")
        );


    std::cout << "Program:\n" << empty.to_string() << "\n\nLinter:\n";

    return 0;
}
