#include <ast/expressions.hpp>
#include <ios>
#include <memory>
#include <iostream>

int main()
{
    ast::manager store;

    auto a1 =
        ast::add(store,
            ast::multiply(store, ast::integer(store, 2), ast::integer(store, 3)),
            ast::multiply(store, ast::integer(store, 42), ast::var(store, "foo"))
        );

    auto a2 =
        ast::add(store,
            ast::multiply(store, ast::integer(store, 2), ast::integer(store, 3)),
            ast::multiply(store, ast::integer(store, 42), ast::var(store, "foo"))
        );

    auto b = ast::add(store, ast::var(store, "woah"), ast::var(store, "cute"));

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

    auto eq = store.make_expression<ast::constraint>(ast::constraint::relation::eq, &a1, &b);
    std::cout << eq->to_string() << '\n';

    auto neq = store.make_expression<ast::constraint>(
        ast::constraint::relation::neq,
        ast::var(store, "foo"),
        ast::var(store, "bar")
    );

    std::cout << neq->to_string() << '\n';

    return 0;
}
