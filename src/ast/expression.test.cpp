#include "ast/expression.hpp"
#include <memory>
#include <iostream>

int main()
{
    auto var = std::make_unique<ast::var>("foo");
    auto answer = std::make_unique<ast::integer>(42);

    auto test = std::make_unique<ast::add>(std::move(var), std::move(answer));

    std::cout << test->to_string() << "\n";

    auto cloned = test->clone();
    std::cout << cloned->to_string() << "\n";

    return 0;
}
