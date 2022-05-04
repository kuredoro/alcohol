#include "ast/manager.hpp"
#include <boost/ut.hpp>

#include <ast/replace_var.hpp>

struct test_case
{
    ast::expression* input;
    std::string varName;
    ast::expression* with;

    ast::expression* want;
};

using namespace boost::ut;

int main()
{
    ast::manager store;

    std::vector<test_case> cases{
        {
            // input
            store.make_expression<ast::integer>(42),
            "foo",
            store.make_expression<ast::var>("bar"),

            // want
            store.make_expression<ast::integer>(42),
        },
        {
            // input
            store.make_expression<ast::var>("foo"),
            "foo",
            store.make_expression<ast::var>("bar"),

            // want
            store.make_expression<ast::var>("bar"),
        },
    };

    for (auto& testCase : cases)
    {
        auto testName =
            "given " + testCase.varName
            + " -> " + testCase.with->to_string()
            + ", expression " + testCase.input->to_string()
            + " becomes " + testCase.want->to_string();

        test(testName) = [&]() {
            auto targetVar = store.make_expression<ast::var>(testCase.varName);
            auto got = ast::replace_var(store, testCase.input, targetVar, testCase.with);

            expect(store.same(got, testCase.want)) << "got" << got->to_string() << "but want" << testCase.want->to_string();
        };
    }

    return 0;
}
