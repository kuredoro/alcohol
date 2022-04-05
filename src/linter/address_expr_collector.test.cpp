#include <boost/ut.hpp>
#include <gsl/gsl-lite.hpp>

#include <linter/address_expr_collector.hpp>
#include <ast/manager.hpp>
#include <ast/statements.hpp>
#include <ast/expressions.hpp>

#include <iostream>
#include <type_traits>
#include <unordered_map>

using namespace boost::ut;

template <class Container>
std::string to_string(const Container& exprs)
{
    if (exprs.empty())
        return "[]";

    std::stringstream ss;
    ss << "[";

    for (size_t i = 0; i < exprs.size() - 1; i++)
    {
        ss << exprs[i]->to_string() << ", ";
    }

    ss << exprs.back()->to_string() << "]";

    return ss.str();
}

template <class Container>
bool assert_no_duplicates(const Container& exprs)
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
        expect(false) << "got expression array containing duplicates:" << to_string(exprs);
    }

    return noDuplicates;
}

template <
    class ContainerA,
    class ContainerB,
    bool = std::is_base_of<ast::expression, typename ContainerA::value_type>::value,
    bool = std::is_base_of<ast::expression, typename ContainerB::value_type>::value>
bool assert_expression_sets(const ContainerA& got, const ContainerB& want)
{
    bool gotIsSet = true, wantIsSet = true;
    expect(gotIsSet = assert_no_duplicates(got)) << "but it should contain only unique expressions";
    expect(wantIsSet = assert_no_duplicates(want)) << "the test case is flawed (!!!)";

    if (!wantIsSet)
        return false;

    std::unordered_map<std::string, std::pair<size_t, size_t>> count;
    for (auto& expr : got)
    {
        count[expr->to_string()].first++;
    }

    for (auto& expr : want)
    {
        count[expr->to_string()].second++;
    }

    bool same = true;
    for (auto& kv : count)
    {
        if (kv.second.first != 0 && kv.second.second != 0)
            continue;

        same = false;
        if (kv.second.first == 0)
            expect(false) << "missing" << kv.first;
        else if (kv.second.second == 0)
            expect(false) << "got superfluous" << kv.first;
    }

    if (!same)
    {
        expect(false) << "got expressions" << to_string(got) << "but want" << to_string(want);
    }

    return same;
}

struct test_case
{
    std::string name;
    ast::statement* input;
    std::vector<ast::var*> wantVars;
    std::vector<ast::expression*> wantExprs;
};

int main()
{
    ast::manager store;

    std::vector<test_case> perDefinitionCases{
        {
            "allocation_statement",
            store.make_statement<ast::alloc>(
                "foo", 3
            ),
            {
                store.make_expression<ast::var>("foo"),
            },
            {
                store.make_expression<ast::var>("foo"),
                store.make_expression<ast::add>(ast::var(store, "foo"), ast::integer(store, 1)),
                store.make_expression<ast::add>(ast::var(store, "foo"), ast::integer(store, 2)),
            },
        },
        {
            // Make test for expr lhs
            "store_to_var",
            store.make_statement<ast::store>(
                ast::var(store, "foo"), ast::integer(store, 3)
            ),
            {
                store.make_expression<ast::var>("foo"),
            },
            {
                store.make_expression<ast::var>("foo"),
            },
        },
        {
            "load_from_var",
            store.make_statement<ast::load>(
                "foo", ast::var(store, "bar")
            ),
            {
                store.make_expression<ast::var>("bar"),
            },
            {
                store.make_expression<ast::var>("bar"),
            },
        },
        {
            "dispose",
            store.make_statement<ast::dispose>(
                "zap"
            ),
            {
                store.make_expression<ast::var>("zap"),
            },
            {
                store.make_expression<ast::var>("zap"),
            },
        },
        {
            "address_var_assignment",
            store.make_statement<ast::block>(
                ast::decl(store, "foo", ast::integer(store, 0)),
                ast::alloc(store, "x", 2),
                ast::assign(store, "x",
                    ast::add(store,
                        ast::multiply(store,
                            ast::integer(store, 2), ast::var(store, "x")
                        ),
                        ast::integer(store, 5)
                    )
                ),
                ast::assign(store, "foo", ast::var(store, "x"))
            ),
            {
                store.make_expression<ast::var>("x"),
            },
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::add>(
                    ast::var(store, "x"),
                    ast::integer(store, 1)
                ),
                store.make_expression<ast::add>(
                    ast::multiply(store,
                        ast::integer(store, 2), ast::var(store, "x")
                    ),
                    ast::integer(store, 5)
                ),
            },
        },
        {
            "store_to_expr",
            store.make_statement<ast::store>(
                ast::add(store,
                    ast::var(store, "foo"), ast::integer(store, 1)
                ),
                ast::integer(store, 3)
            ),
            {
                store.make_expression<ast::var>("foo"),
            },
            {
                store.make_expression<ast::var>("foo"),
                store.make_expression<ast::add>(
                    ast::var(store, "foo"), ast::integer(store, 1)
                ),
            },
        },
    };

    "per_definition"_test = [&] {
        for (auto& testCase : perDefinitionCases)
        {
            should(testCase.name) = [&] {
                linter::address_expr_collector collector(store);

                testCase.input->accept(collector);

                auto gotVars = collector.address_variables();
                auto gotExprs = collector.address_expressions();

                expect(assert_expression_sets(gotVars, testCase.wantVars)) << "while asserting found address variables";
                expect(assert_expression_sets(gotExprs, testCase.wantExprs)) << "while asserting found address expressions";
            };   
        }
    };

    /*
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

            assert_expression_sets(got, want);
        };
    };
    */

    return 0;
}
