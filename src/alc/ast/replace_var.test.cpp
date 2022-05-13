#include <alc/ast/expressions.hpp>
#include <alc/ast/manager.hpp>
#include <boost/ut.hpp>

#include <alc/ast/replace_var.hpp>

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
        {
            // input
            store.make_expression<ast::var>("foo"),
            "bar",
            store.make_expression<ast::var>("zap"),

            // want
            store.make_expression<ast::var>("foo"),
        },
        {
            // input
            store.make_expression<ast::add>(
                ast::var(store, "foo"),
                ast::var(store, "bar")
            ),
            "bar",
            store.make_expression<ast::var>("zap"),

            // want
            store.make_expression<ast::add>(
                ast::var(store, "foo"),
                ast::var(store, "zap")
            ),
        },
        {
            // input
            store.make_expression<ast::multiply>(
                ast::var(store, "foo"),
                ast::var(store, "bar")
            ),
            "foo",
            store.make_expression<ast::var>("bar"),

            // want
            store.make_expression<ast::multiply>(
                ast::var(store, "bar"),
                ast::var(store, "bar")
            ),
        },
        {
            // input
            store.make_expression<ast::add>(
                ast::multiply(store,
                    ast::integer(store, 42),
                    ast::var(store, "foo")
                ),
                ast::var(store, "bar")
            ),
            "foo",
            store.make_expression<ast::add>(
                ast::var(store, "bar"),
                ast::var(store, "zap")
            ),

            // want
            store.make_expression<ast::add>(
                ast::multiply(store,
                    ast::integer(store, 42),
                    ast::add(store,
                        ast::var(store, "bar"),
                        ast::var(store, "zap")
                    )
                ),
                ast::var(store, "bar")
            ),
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
