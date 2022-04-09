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

        expect(dupCount == 0) << "found" << dupCount << "duplicates for expression" << exprs[i]->to_string();
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

    return gotIsSet && same;
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
            "variable_assigned_allocated_memory_is_address",
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
            "store_to_a_variable_makes_it_address",
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
            "load_from_a_variable_makes_it_address",
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
            "variable_in_dispose_is_address",
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
            "expression_assigned_to_address_var_is_address",
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
                )
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
            "store_to_an_expression_with_a_variable_makes_them_address",
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
        {
            "load_from_an_expression_with_a_variable_makes_them_address",
            store.make_statement<ast::load>(
                "foo",
                ast::add(store,
                    ast::var(store, "bar"), ast::integer(store, 1)
                )
            ),
            {
                store.make_expression<ast::var>("bar"),
            },
            {
                store.make_expression<ast::var>("bar"),
                store.make_expression<ast::add>(
                    ast::var(store, "bar"), ast::integer(store, 1)
                ),
            },
        },
        {
            "declared_variable_is_address_if_initialized_with_address_expression",
            store.make_statement<ast::block>(
                ast::alloc(store, "x", 1),
                ast::decl(store, "y", ast::var(store, "x"))
            ),
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::var>("y"),
            },
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::var>("y"),
            },
        },
        {
            "variable_is_address_if_assigned_address_expr",
            store.make_statement<ast::block>(
                ast::decl(store, "x", ast::integer(store, 0)),
                ast::alloc(store, "y", 1),
                ast::assign(store, "x", ast::var(store, "y"))
            ),
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::var>("y"),
            },
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::var>("y"),
            },
        },
        {
            "assigning_an_address_variable_transfers_its_implicit_address_expressions",
            store.make_statement<ast::block>(
                ast::alloc(store, "x", 3),
                ast::assign(store, "y", ast::var(store, "x")),
                ast::assign(store, "z", ast::var(store, "y")),
                ast::assign(store, "n", ast::var(store, "y"))
            ),
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::var>("y"),
                store.make_expression<ast::var>("z"),
                store.make_expression<ast::var>("n"),
            },
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::add>(ast::var(store, "x"), ast::integer(store, 1)),
                store.make_expression<ast::add>(ast::var(store, "x"), ast::integer(store, 2)),
                store.make_expression<ast::var>("y"),
                store.make_expression<ast::add>(ast::var(store, "y"), ast::integer(store, 1)),
                store.make_expression<ast::add>(ast::var(store, "y"), ast::integer(store, 2)),
                store.make_expression<ast::var>("z"),
                store.make_expression<ast::add>(ast::var(store, "z"), ast::integer(store, 1)),
                store.make_expression<ast::add>(ast::var(store, "z"), ast::integer(store, 2)),
                store.make_expression<ast::var>("n"),
                store.make_expression<ast::add>(ast::var(store, "n"), ast::integer(store, 1)),
                store.make_expression<ast::add>(ast::var(store, "n"), ast::integer(store, 2)),
            },
        },
        {
            "assigning_an_address_variable_to_declaration_transfers_its_implicit_address_expressions",
            store.make_statement<ast::block>(
                ast::alloc(store, "x", 3),
                ast::decl(store, "y", ast::var(store, "x")),
                ast::decl(store, "z", ast::var(store, "y")),
                ast::decl(store, "n", ast::var(store, "y"))
            ),
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::var>("y"),
                store.make_expression<ast::var>("z"),
                store.make_expression<ast::var>("n"),
            },
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::add>(ast::var(store, "x"), ast::integer(store, 1)),
                store.make_expression<ast::add>(ast::var(store, "x"), ast::integer(store, 2)),
                store.make_expression<ast::var>("y"),
                store.make_expression<ast::add>(ast::var(store, "y"), ast::integer(store, 1)),
                store.make_expression<ast::add>(ast::var(store, "y"), ast::integer(store, 2)),
                store.make_expression<ast::var>("z"),
                store.make_expression<ast::add>(ast::var(store, "z"), ast::integer(store, 1)),
                store.make_expression<ast::add>(ast::var(store, "z"), ast::integer(store, 2)),
                store.make_expression<ast::var>("n"),
                store.make_expression<ast::add>(ast::var(store, "n"), ast::integer(store, 1)),
                store.make_expression<ast::add>(ast::var(store, "n"), ast::integer(store, 2)),
            },
        }
    };

    std::vector<test_case> noDuplicatesCases{
        {
            "two_stores_to_an_expression_with_a_variable_does_not_produce_duplicates",
            store.make_statement<ast::block>(
                ast::store(store,
                    ast::add(store,
                        ast::var(store, "foo"), ast::integer(store, 1)
                    ),
                    ast::integer(store, 3)
                ),
                ast::store(store,
                    ast::add(store,
                        ast::var(store, "foo"), ast::integer(store, 1)
                    ),
                    ast::integer(store, 3)
                )
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
        {
            "two_loads_from_an_expression_with_a_variable_does_not_produce_duplicates",
            store.make_statement<ast::block>(
                ast::load(store,
                    "foo",
                    ast::add(store,
                        ast::var(store, "bar"), ast::integer(store, 1)
                    )
                ),
                ast::load(store,
                    "foo",
                    ast::add(store,
                        ast::var(store, "bar"), ast::integer(store, 1)
                    )
                )
            ),
            {
                store.make_expression<ast::var>("bar"),
            },
            {
                store.make_expression<ast::var>("bar"),
                store.make_expression<ast::add>(
                    ast::var(store, "bar"), ast::integer(store, 1)
                ),
            },
        },
        {
            "double_disposing_a_variable_does_not_produce_duplicates",
            store.make_statement<ast::block>(
                ast::dispose(store, "x"),
                ast::dispose(store, "x")
            ),
            {
                store.make_expression<ast::var>("x"),
            },
            {
                store.make_expression<ast::var>("x"),
            },
        },
        {
            "double_allocation_does_not_produce_duplicates",
            store.make_statement<ast::block>(
                ast::alloc(store, "x", 2),
                ast::alloc(store, "x", 2)
            ),
            {
                store.make_expression<ast::var>("x"),
            },
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::add>(ast::var(store, "x"), ast::integer(store, 1)),
            },
        },
    };

    std::vector<test_case> smallPrograms{
        {
            "control_statements",
            store.make_statement<ast::block>(
                ast::decl(store, "x", ast::integer(store, 0)),
                ast::if_else(store,
                    ast::block(store,
                        ast::while_loop(store,
                            ast::block(store,
                                ast::assign(store,
                                    "x",
                                    ast::add(store,
                                        ast::var(store, "x"), ast::integer(store, 1)
                                    )
                                )
                            )
                        )
                    ),
                    ast::block(store)
                ),
                ast::while_loop(store,
                    ast::if_else(store,
                        ast::alloc(store,
                            "x",
                            3
                        )
                    )
                )
            ),
            {
                store.make_expression<ast::var>("x"),
            },
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::add>(ast::var(store, "x"), ast::integer(store, 1)),
                store.make_expression<ast::add>(ast::var(store, "x"), ast::integer(store, 2)),
            },
        },
        {
            "empty_example",
            store.make_statement<ast::block>(
                ast::decl(store, "x", ast::integer(store, 0)),
                ast::decl(store, "y", ast::integer(store, 0)),

                ast::alloc(store, "x", 2),
                ast::store(store, ast::var(store, "x"), ast::integer(store, 1)),
                ast::store(store, ast::add(store, ast::var(store, "x"), ast::integer(store, 1)), ast::integer(store, 3)),

                ast::alloc(store, "y", 1),
                ast::store(store, ast::var(store, "y"), ast::integer(store, 2)),

                ast::dispose(store, "x"),
                ast::dispose(store, "y")
            ),
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::var>("y"),
            },
            {
                store.make_expression<ast::var>("x"),
                store.make_expression<ast::add>(ast::var(store, "x"), ast::integer(store, 1)),
                store.make_expression<ast::var>("y"),
            }
        }
    };

    auto runCases = [&store] (const std::vector<test_case>& cases) {
        return [&store, &cases] () {
            for (auto& testCase : cases)
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
    };

    "per_definition"_test = runCases(perDefinitionCases);

    "no_duplicates"_test = runCases(noDuplicatesCases);

    "small_programs"_test = runCases(smallPrograms);

    return 0;
}
