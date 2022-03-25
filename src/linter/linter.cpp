#include "ast/statements.hpp"
#include <linter/linter.hpp>
#include <iostream>
#include <typeinfo>
#include <typeindex>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/mpl_list.hpp>
using namespace boost::mp11;

namespace linter
{


void linter::process(ast::statement& statement)
{
    const auto& type = typeid(statement);

    // TODO: check if assembly is optimal...
    mp_for_each<
        mp_transform<mp_list, mp_list<ast::decl, ast::alloc, ast::store, ast::dispose>>
    >([this, &type, &statement](auto x)
    {
        if (type == typeid(mp_first<decltype(x)>))
        {
            process(static_cast<mp_first<decltype(x)>&>(statement));
        }
    });

    // XXX: if none of the types above match, we get silence, no warnings...
}

void linter::process(ast::block& block)
{
    std::cout << "processing block\n";

    for (auto& statement : block.statements())
    {
        process(*statement.get());
    }

    std::cout << "done processing block\n";
}

void linter::process(ast::decl& decl)
{
    std::cout << "processing decl\n";
}

void linter::process(ast::alloc& alloc)
{
    std::cout << "processing alloc\n";
}

void linter::process(ast::store& store)
{
    std::cout << "processing store\n";
}

void linter::process(ast::dispose& store)
{
    std::cout << "processing dispose\n";
}

}
