#include <alc/ast/expressions.hpp>
#include <alc/ast/manager.hpp>
#include <alc/constraint/set.hpp>

#include <boost/ut.hpp>

using namespace boost::ut;

struct consistency_test_case
{
    bool wantConsistent;
    std::vector<ast::constraint*> constraints;
};

struct satisfiability_test_case
{
    bool wantSatisfiability;
    std::vector<ast::constraint*> constraints;
    ast::constraint* target;
};

int main()
{
    ast::manager store;

    std::vector<consistency_test_case> checkConsistencyCases{
        {
            true,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::integer(store, 42),
                    ast::integer(store, 42)
                ),
            },
        },
        {
            false,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::neq,
                    ast::integer(store, 0),
                    ast::integer(store, 0)
                ),
            },
        },
        {
            true,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::var(store, "foo"),
                    ast::var(store, "foo")
                ),
            },
        },
        {
            false,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::neq,
                    ast::var(store, "foo"),
                    ast::var(store, "foo")
                ),
            },
        },
        {
            true,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::add(store,
                        ast::var(store, "foo"), ast::var(store, "bar")
                    ),
                    ast::var(store, "foo")
                ),
            },
        },
        {
            false,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::add(store,
                        ast::var(store, "foo"), ast::integer(store, 0)
                    ),
                    ast::add(store,
                        ast::var(store, "foo"), ast::integer(store, 1)
                    )
                ),
            },
        },
        {
            true,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::neq,
                    ast::multiply(store,
                        ast::integer(store, 2), ast::var(store, "foo")
                    ),
                    ast::multiply(store,
                        ast::integer(store, 3), ast::var(store, "foo")
                    )
                ),
            },
        },
        {
            true,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::multiply(store,
                        ast::add(store,
                            ast::var(store, "foo"), ast::integer(store, 1)
                        ),
                        ast::add(store,
                            ast::var(store, "bar"), ast::integer(store, 1)
                        )
                    ),
                    ast::multiply(store,
                        ast::var(store, "foo"), ast::var(store, "bar")
                    )
                ),
            },
        },
        {
            true,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::var(store, "foo"),
                    ast::var(store, "bar")
                ),
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::integer(store, 42),
                    ast::integer(store, 42)
                ),
            },
        },
        {
            false,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::add(store,
                        ast::var(store, "foo"), ast::var(store, "bar")
                    ),
                    ast::integer(store, 0)
                ),
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::add(store,
                        ast::var(store, "foo"), ast::var(store, "bar")
                    ),
                    ast::integer(store, 1)
                ),
            },
        },
    };

    std::vector<satisfiability_test_case> satisfiabilityCases{
        {
            true,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::var(store, "foo"),
                    ast::integer(store, 1)
                ),
            },
            store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::add(store,
                    ast::var(store, "foo"), ast::integer(store, 1)
                ),
                ast::integer(store, 2)
            ),
        },
        {
            false,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::var(store, "foo"),
                    ast::integer(store, 1)
                ),
            },
            store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::add(store,
                    ast::var(store, "foo"), ast::integer(store, 1)
                ),
                ast::integer(store, 0)
            ),
        },
        {
            true,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::var(store, "foo"),
                    ast::integer(store, 1)
                ),
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::var(store, "foo"),
                    ast::integer(store, 2)
                ),
            },
            store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::add(store,
                    ast::var(store, "foo"), ast::integer(store, 1)
                ),
                ast::integer(store, 0)
            ),
        },
        {
            true,
            {
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::var(store, "foo"),
                    ast::integer(store, 1)
                ),
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::var(store, "bar"),
                    ast::integer(store, 2)
                ),
            },
            store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::add(store,
                    ast::var(store, "foo"), ast::var(store, "bar")
                ),
                ast::integer(store, 3)
            ),
        },
    };

    "default constructed"_test = [&] () {
        should("is consistent") = [&] () {
            constraint::set constraints;
            expect(constraints.check_consistency());
        };

        should("satisfies satisfiable constraint") = [&] () {
            constraint::set constraints;

            expect(constraints.check_satisfiability_of(
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::integer(store, 1),
                    ast::integer(store, 1)
            )) == true);
        };

        should("unsatisfies unsatisfiable constraint") = [&] () {
            constraint::set constraints;

            expect(constraints.check_satisfiability_of(
                store.make_expression<ast::constraint>(
                    ast::constraint::relation::eq,
                    ast::integer(store, 0),
                    ast::integer(store, 1)
            )) == false);
        };
    };

    "check_consistency"_test = [&] () {
        // TODO: range-v3
        for (auto& testCase : checkConsistencyCases)
        {
            std::string testName;
            for (size_t i = 0; i < testCase.constraints.size(); i++)
            {
                if (i != 0)
                    testName += " && ";

                testName += testCase.constraints[i]->to_string();
            }

            if (testCase.wantConsistent)
            {
                testName += " is consistent";
            }
            else
            {
                testName += " is inconsistent";
            }

            should(testName) = [&] () {
                constraint::set constraints;

                for (auto& constraint : testCase.constraints)
                {
                    constraints.add(constraint);
                }

                expect(constraints.check_consistency() == testCase.wantConsistent);
            };
        }
    };

    "satisfiability"_test = [&] () {
        // TODO: range-v3
        for (auto& testCase : satisfiabilityCases)
        {
            std::string testName = testCase.target->to_string();

            if (testCase.wantSatisfiability)
            {
                testName += " is satisfiable with ";
            }
            else
            {
                testName += " is unsatisfiable with ";
            }

            for (size_t i = 0; i < testCase.constraints.size(); i++)
            {
                if (i != 0)
                    testName += " && ";

                testName += testCase.constraints[i]->to_string();
            }

            should(testName) = [&] () {
                constraint::set constraints;

                for (auto& constraint : testCase.constraints)
                {
                    constraints.add(constraint);
                }

                expect(constraints.check_satisfiability_of(testCase.target) == testCase.wantSatisfiability);
            };
        }
    };

    return 0;
}
