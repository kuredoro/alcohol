#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <memory>

#include <linter/address_expr_collector.hpp>
#include <ast/expressions.hpp>
#include <ast/statements.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/mpl_list.hpp>
using namespace boost::mp11;

namespace linter
{

void address_expr_collector::process(ast::statement& statement)
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

void address_expr_collector::process(ast::block& block)
{
    std::cout << "processing block\n";

    for (auto& statement : block.statements())
    {
        statement->accept(*this);
    }

    std::cout << "done processing block\n";
}

void address_expr_collector::process(ast::decl& decl)
{
    std::cout << "processing decl\n";
}

void address_expr_collector::process(ast::alloc& alloc)
{
    addrExprs_.push_back(alloc.destination_var());

    for (size_t i = 1; i < alloc.alloc_size(); i++)
    {
        derivedExprs_.push_back(std::make_unique<ast::add>(ast::var(alloc.destination_var()->name()), ast::integer(i)));
        addrExprs_.push_back(derivedExprs_.back().get());
    }
}

void address_expr_collector::process(ast::store& store)
{
    addrExprs_.push_back(store.destination());
}

void address_expr_collector::process(ast::load& load)
{
    addrExprs_.push_back(load.source());
}

void address_expr_collector::process(ast::dispose& dispose)
{
    addrExprs_.push_back(dispose.target_var());

    std::cout << "processing dispose\n";
}

void address_expr_collector::process(ast::if_else& ifElse)
{
    std::cout << "processing if_else\n";
}

void address_expr_collector::process(ast::while_loop& whileLoop)
{
    std::cout << "processing while_loop\n";
}

}
