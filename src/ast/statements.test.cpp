#include <ast/statements.hpp>
#include <ast/expressions.hpp>
#include <iostream>

int main()
{
    auto hello =
        ast::block(
            ast::decl("foo", ast::integer(42)),
            ast::alloc("array", 2),
            ast::decl("initPtr", ast::add(ast::var("array"), ast::integer(0))),
            ast::store(ast::var("initPtr"), ast::integer(1)),
            ast::assign("initPtr", ast::add(ast::var("array"), ast::integer(1))),
            ast::store(ast::var("initPtr"), ast::integer(2)),
            ast::while_loop(
                ast::block(
                    ast::assign("foo", ast::var("array")),
                    ast::if_else(
                        ast::assign("foo", ast::add(ast::var("array"), ast::integer(1))),
                        ast::assign("foo", ast::add(ast::var("array"), ast::integer(2)))
                    ),
                    ast::store(ast::var("foo"), ast::integer(-1)),
                    ast::load("foo", "foo")
                )
            )
        );

    std::cout << hello.to_string();

    return 0;
}
