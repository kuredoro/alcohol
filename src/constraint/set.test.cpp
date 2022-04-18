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

        should("singe integer that equals to itself is consistent") = [&] () {
            constraint::set constraints;

            auto expr = store.make_expression<ast::constraint>(
                ast::constraint::relation::eq,
                ast::integer(store, 42),
                ast::integer(store, 42)
            );

            constraints.add(expr);

            expect(constraints.check_consistency()) << "for a single constraint:" << expr->to_string();
        };

        should("singe integer that does not equal to itself is inconsistent") = [&] () {
            constraint::set constraints;

            auto expr = store.make_expression<ast::constraint>(
                ast::constraint::relation::neq,
                ast::integer(store, 0),
                ast::integer(store, 0)
            );

            constraints.add(expr);

            expect(!constraints.check_consistency()) << "for a single constraint:" << expr->to_string();
        };
    };

    return 0;
}
