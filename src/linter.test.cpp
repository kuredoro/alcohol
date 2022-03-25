#include "ast.hpp"

int main()
{
    auto hello =
        ast::block(
            ast::nop(),
            ast::if_else(
                ast::nop()
            ),
            1
        );

    return 0;
}
