#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <ast/manager.hpp>
#include <iostream>

int main()
{
    ast::manager store;

    auto hello =
        store.make_statement<ast::block>(
            ast::decl(store, "foo", ast::integer(store, 42)),
            ast::alloc(store, "array", 2),
            ast::decl(store, "initPtr",
                ast::add(store,
                    ast::var(store, "array"), ast::integer(store, 0)
                )
            ),
            ast::store(store, ast::var(store, "initPtr"), ast::integer(store, 1)),
            ast::assign(store, "initPtr",
                ast::add(store,
                    ast::var(store, "array"), ast::integer(store, 1)
                )
            ),
            ast::store(store, ast::var(store, "initPtr"), ast::integer(store, 2)),
            ast::while_loop(store,
                ast::block(store,
                    ast::assign(store, "foo", ast::var(store, "array")),
                    ast::if_else(store,
                        ast::assign(store, "foo", ast::add(store, ast::var(store, "array"), ast::integer(store, 1))),
                        ast::assign(store, "foo", ast::add(store, ast::var(store, "array"), ast::integer(store, 2)))
                    ),
                    ast::store(store, ast::var(store, "foo"), ast::integer(store, -1)),
                    ast::load(store, "foo", "foo")
                )
            )
        );

    std::cout << hello->to_string();

    return 0;
}
