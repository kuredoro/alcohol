#include "ast.hpp"
#include <iostream>

int main()
{
    auto hello =
        ast::block(
            ast::nop(),
            ast::while_loop(
                ast::block(
                    ast::nop(),
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
