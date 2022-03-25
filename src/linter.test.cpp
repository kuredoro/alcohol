#include "ast.hpp"
#include <iostream>

int main()
{
    auto hello =
        ast::block(
            ast::decl("foo", 42),
            ast::alloc("array", 2),
            ast::decl("initPtr", 0),
            ast::store("initPtr", "aiaiai"),
            ast::assign("initPtr", 1),
            ast::store("initPtr", "aiaia"),
            ast::while_loop(
                ast::block(
                    ast::assign("oi", 42),
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
