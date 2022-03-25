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

void address_var_collector::process(ast::statement& statement)
{
    std::cout << "<unknown statement>\n";
    /*
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
    */
}

void address_var_collector::process(ast::block& block)
{
    std::cout << "processing block\n";

    for (auto& statement : block.statements())
    {
        statement->accept(*this);
    }

    std::cout << "done processing block\n";
}

void address_var_collector::process(ast::decl& decl)
{
    std::cout << "processing decl\n";
}

void address_var_collector::process(ast::alloc& alloc)
{
    std::cout << "processing alloc\n";
}

void address_var_collector::process(ast::store& store)
{
    std::cout << "processing store\n";
}

void address_var_collector::process(ast::dispose& dispose)
{
    std::cout << "processing dispose\n";
}

void address_var_collector::process(ast::load& load)
{
    std::cout << "processing load\n";
}

void address_var_collector::process(ast::if_else& ifElse)
{
    std::cout << "processing if_else\n";
}

void address_var_collector::process(ast::while_loop& whileLoop)
{
    std::cout << "processing while_loop\n";
}

}
