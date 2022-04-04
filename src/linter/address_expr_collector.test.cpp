#include <boost/ut.hpp>
#include <gsl/gsl-lite.hpp>

#include <linter/address_expr_collector.hpp>
#include <ast/manager.hpp>
#include <ast/statements.hpp>
#include <ast/expressions.hpp>

#include <iostream>
#include <unordered_map>

using namespace boost::ut;

bool assert_no_duplicates(const gsl::span<ast::expression*> exprs)
{
    std::vector<int> eqClasses(exprs.size(), -1);

    bool noDuplicates = true;
    for (size_t i = 0; i < exprs.size(); i++)
    {
        if (eqClasses[i] != -1)
            continue;

        eqClasses[i] = i;

        size_t dupCount = 0;
        for (size_t j = i + 1; j < exprs.size(); j++)
        {
            std::cout << "i = " << i << " and j = " << j << " :: " << exprs[i]->to_string() << " vs. " << exprs[j]->to_string() << '\n';
            if (exprs[i]->to_string() == exprs[j]->to_string())
            {
                eqClasses[j] = i;
                dupCount++;
                noDuplicates = false;
            }
        }

        expect(dupCount == 0_i) << "duplicates for expression" << exprs[i]->to_string();
    }

    if (!noDuplicates)
    {
        std::stringstream ss;
        for (auto& expr : exprs)
        {
            ss << expr->to_string() << " ";
        }

        expect(false) << "expression array has duplicates:" << ss.str();
    }

    return noDuplicates;
}

int main()
{
    ast::manager store;








    "small_programs"_test = [&] {
        should("empty_example") = [&] {
            auto input = store.make_statement<ast::block>(
                ast::decl(store, "x", ast::integer(store, 0)),
                ast::decl(store, "y", ast::integer(store, 0)),

                ast::alloc(store, "x", 2),
                ast::store(store, ast::var(store, "x"), ast::integer(store, 1)),
                ast::store(store, ast::add(store, ast::var(store, "x"), ast::integer(store, 1)), ast::integer(store, 3)),

                ast::alloc(store, "y", 1),
                ast::store(store, ast::var(store, "y"), ast::integer(store, 2)),

                ast::dispose(store, "x"),
                ast::dispose(store, "y")
            );

            std::vector<ast::expression*> want{
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::add>(ast::var(store, "x"), ast::integer(store, 1)),
                store.make_expression<ast::var>("y"),
            };

            linter::address_expr_collector collector(store);

            collector.process(*input);

            auto got = collector.address_expressions();

            std::cout << "\n\nAddress expressions:\n";
            for (auto& expr : got)
            {
                std::cout << expr->to_string() << '\n';
            }

            expect(assert_no_duplicates(got) >> fatal);
        };
    };

    return 0;
}
