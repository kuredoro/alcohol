#include "ast/expressions.hpp"
#include <ast/manager.hpp>
#include <constraint/set.hpp>

#include <boost/ut.hpp>

using namespace boost::ut;

int main()
{
    "check_consistency"_test = [] () {
        ast::manager store;

        should("empty set is consistent") = [&] () {
            constraint::set constraints;
            expect(constraints.check_consistency());
        };

        should("int1 == int1 is consistent") = [&] () {
            constraint::set constraints;

            auto expr = store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::integer(store, 42),
                ast::integer(store, 42)
            );

            constraints.add(expr);

            expect(constraints.check_consistency()) << "for a single constraint:" << expr->to_string();
        };

        should("int1 != int1 is inconsistent") = [&] () {
            constraint::set constraints;

            auto expr = store.make_expression<ast::constraint>(
                ast::constraint::relation::neq,
                ast::integer(store, 0),
                ast::integer(store, 0)
            );

            constraints.add(expr);

            expect(!constraints.check_consistency()) << "for a single constraint:" << expr->to_string();
        };

        should("var == var is consistent") = [&] () {
            constraint::set constraints;

            auto expr = store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::var(store, "foo"),
                ast::var(store, "foo")
            );

            constraints.add(expr);

            expect(constraints.check_consistency()) << "for a single constraint:" << expr->to_string();
        };

        should("var != var is inconsistent") = [&] () {
            constraint::set constraints;

            auto expr = store.make_expression<ast::constraint>(
                ast::constraint::relation::neq,
                ast::var(store, "bar"),
                ast::var(store, "bar")
            );

            constraints.add(expr);

            expect(!constraints.check_consistency()) << "for a single constraint:" << expr->to_string();
        };

        should("foo + bar == foo is consistent") = [&] () {
            constraint::set constraints;

            auto expr = store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::add(store,
                    ast::var(store, "foo"), ast::var(store, "bar")
                ),
                ast::var(store, "foo")
            );

            constraints.add(expr);

            expect(constraints.check_consistency()) << "for a single constraint:" << expr->to_string();
        };

        should("foo + 0 == foo + 1 is inconsistent") = [&] () {
            constraint::set constraints;

            auto expr = store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::add(store,
                    ast::var(store, "foo"), ast::integer(store, 0)
                ),
                ast::add(store,
                    ast::var(store, "foo"), ast::integer(store, 1)
                )
            );

            constraints.add(expr);

            expect(!constraints.check_consistency()) << "for a single constraint:" << expr->to_string();
        };

        should("2 * foo != 3 * foo is consistent") = [&] () {
            constraint::set constraints;

            auto expr = store.make_expression<ast::constraint>(
                ast::constraint::relation::neq,
                ast::multiply(store,
                    ast::integer(store, 2), ast::var(store, "foo")
                ),
                ast::multiply(store,
                    ast::integer(store, 3), ast::var(store, "foo")
                )
            );

            constraints.add(expr);

            expect(constraints.check_consistency()) << "for a single constraint:" << expr->to_string();
        };

        should("(foo + 1) * (bar + 1) == foo * bar is consistent") = [&] () {
            constraint::set constraints;

            auto expr = store.make_expression<ast::constraint>(
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
            );

            constraints.add(expr);

            expect(constraints.check_consistency()) << "for a single constraint:" << expr->to_string();
        };

        should("foo == foo && 42 == 42 is consistent") = [&] () {
            constraint::set constraints;

            auto expr1 = store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::var(store, "foo"),
                ast::var(store, "bar")
            );

            auto expr2 = store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::integer(store, 42),
                ast::integer(store, 42)
            );

            constraints.add(expr1);
            constraints.add(expr2);

            expect(constraints.check_consistency()) << "for 2 constraints:" << expr1->to_string() << " && " << expr2->to_string() << '\n';
        };
    };

    return 0;
}
