#include <constraint/set.hpp>

#include <boost/ut.hpp>

using namespace boost::ut;

int main()
{
    "check_consistency"_test = [] () {
        should("empty set is consistent") = [&] () {
            constraint::set constraints;
            expect(constraints.check_consistency());
        };
    };

    return 0;
}
