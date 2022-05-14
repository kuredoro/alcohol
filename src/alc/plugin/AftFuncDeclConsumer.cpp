#include <alc/plugin/AftFuncDeclConsumer.hpp>

void MatchMemoryAllocationCallback::run(const MatchFinder::MatchResult& result)
{
    llvm::outs() << "Memory alloc!\n";

    auto& nodes = result.Nodes;

    auto decl = nodes.getNodeAs<VarDecl>("varDecl");
    auto size = nodes.getNodeAs<IntegerLiteral>("size");

    auto& store = visitor.ast_store();
    auto alloc = store.make_statement<ast::alloc>(decl->getNameAsString(), size->getValue().getZExtValue());

    visitor.push_statement(alloc, {});
}

void MatchMemoryDeallocationCallback::run(const MatchFinder::MatchResult& result)
{
    llvm::outs() << "Memory free!\n";

    auto& nodes = result.Nodes;

    auto decl = nodes.getNodeAs<VarDecl>("varDecl");

    auto& store = visitor.ast_store();
    auto dispose = store.make_statement<ast::dispose>(decl->getNameAsString());

    visitor.push_statement(dispose, {});
}
