#include <boost/ut.hpp>

#include <linter/linter.hpp>
#include <ast/manager.hpp>
#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <linter/linter.hpp>
#include <iostream>

using namespace boost::ut;

int main()
{
    ast::manager store;

    auto testOn = [&store](std::string name, ast::block& block) {
        test(name) = [&]() {
            std::cout << "#######\n";
            std::cout << "RUNNING " << name << "\n";
            std::cout << block.to_string();
            std::cout << "####### \n\n";

            linter::linter linter(store);
            linter.process(block);
        };
    };

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

    auto directDoubleDispose =
        ast::block(store,
            ast::alloc(store, "x", 2),
            ast::store(store, ast::var(store, "x"), ast::integer(store, 1)),
            ast::store(store, ast::add(store, ast::var(store, "x"), ast::integer(store, 1)), ast::integer(store, 3)),

            ast::dispose(store, "x"),
            ast::dispose(store, "x")
        );

    auto indirectDoubleDispose =
        ast::block(store,
            ast::alloc(store, "x", 2),
            ast::store(store, ast::var(store, "x"), ast::integer(store, 1)),
            ast::store(store, ast::add(store, ast::var(store, "x"), ast::integer(store, 1)), ast::integer(store, 3)),

            ast::decl(store, "y", ast::integer(store, 0)),
            ast::assign(store, "y", ast::var(store, "x")),

            ast::dispose(store, "x"),
            ast::dispose(store, "y")
        );

    auto overwrite =
        ast::block(store,
            ast::alloc(store, "x", 2),
            ast::store(store, ast::var(store, "x"), ast::integer(store, 1)),
            ast::store(store, ast::add(store, ast::var(store, "x"), ast::integer(store, 1)), ast::integer(store, 3)),

            ast::assign(store, "x", ast::integer(store, 0)),

            ast::dispose(store, "x")
        );

    testOn("empty", empty);
    testOn("direct_double_dispose", directDoubleDispose);
    testOn("indirect_double_dispose", indirectDoubleDispose);
    testOn("overwrite", overwrite);

    return 0;
}
