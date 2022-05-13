#include <boost/ut.hpp>

#include <linter/linter.hpp>
#include <ast/manager.hpp>
#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <linter/linter.hpp>
#include <iostream>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

using namespace boost::ut;

int main()
{
    spdlog::set_pattern("%H:%M:%S %^%l%$ %v");
    spdlog::set_level(spdlog::level::debug);

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

    /*
    auto disposeUnallocated =
        ast::block(store,
            ast::decl(store, "x", 0),
            ast::store(store, ast::var(store, "x"), ast::integer(store, 1)),
            ast::store(store, ast::add(store, ast::var(store, "x"), ast::integer(store, 1)), ast::integer(store, 3)),

            ast::assign(store, "x", ast::integer(store, 0)),

            ast::dispose(store, "x")
        );
        */

    auto overwrite =
        ast::block(store,
            ast::alloc(store, "x", 2),
            ast::store(store, ast::var(store, "x"), ast::integer(store, 1)),
            ast::store(store, ast::add(store, ast::var(store, "x"), ast::integer(store, 1)), ast::integer(store, 3)),

            ast::assign(store, "x", ast::integer(store, 0)),

            ast::dispose(store, "x")
        );

    struct test_case { std::string name; ast::block& code; };
    std::vector<test_case> tests{
        { "empty", empty },
        { "direct_double_dispose", directDoubleDispose },
        { "indirect_double_dispose", indirectDoubleDispose },
        { "overwrite", overwrite },
    };
    
    for (auto& t : tests)
    {
        testOn(t.name, t.code);
    }

    return 0;
}
