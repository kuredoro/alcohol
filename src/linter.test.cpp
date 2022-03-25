#include "ast.hpp"
#include <iostream>

int main()
{
    auto hello =
        ast::block(
            ast::decl("foo", ast::expression()),
            ast::alloc("array", 2),
            ast::decl("initPtr", ast::expression()),
            ast::store("initPtr", "aiaiai"),
            ast::assign("initPtr", ast::expression()),
            ast::store("initPtr", "aiaia"),
            ast::while_loop(
                ast::block(
                    ast::assign("oi", ast::expression()),
                    ast::if_else(
                        ast::nop(),
                        ast::nop()
                    )
                )
            )
        );

    std::cout << hello.to_string();

    return 0;
}
