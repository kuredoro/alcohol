#include <ast/expressions.hpp>
#include <memory>
#include <iostream>

int main()
{
    auto test =
        ast::add(
            ast::multiply(ast::integer(2), ast::integer(3)),
            ast::multiply(ast::integer(42), ast::var("foo"))
        );

    std::cout << test.to_string() << "\n";

    return 0;
}
