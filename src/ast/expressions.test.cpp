#include <ast/expressions.hpp>
#include <ios>
#include <memory>
#include <iostream>

int main()
{
    auto a1 =
        ast::add(
            ast::multiply(ast::integer(2), ast::integer(3)),
            ast::multiply(ast::integer(42), ast::var("foo"))
        );

    auto a2 =
        ast::add(
            ast::multiply(ast::integer(2), ast::integer(3)),
            ast::multiply(ast::integer(42), ast::var("foo"))
        );

    auto b = ast::add(ast::var("woah"), ast::var("cute"));

    std::cout << "a1: " << a1.to_string() << "\n";
    std::cout << "a2: " << a2.to_string() << "\n";
    std::cout << "b:  " << b.to_string() << "\n";

    std::cout << "\n";

    std::cout << std::boolalpha << "a1 == a2: " << (a1 == a2) << "\n";
    std::cout << std::boolalpha << "b == a2: " << (b == a2) << "\n";
    std::cout << std::boolalpha << "a1 == b: " << (a1 == b) << "\n";

    std::cout << "\n";

    std::cout << std::boolalpha << "a1 != a2: " << (a1 != a2) << "\n";
    std::cout << std::boolalpha << "b != a2: " << (b != a2) << "\n";
    std::cout << std::boolalpha << "a1 != b: " << (a1 != b) << "\n";
    return 0;
}
