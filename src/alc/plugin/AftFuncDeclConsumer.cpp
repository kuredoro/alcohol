#include "alc/ast/expressions.hpp"
#include "alc/ast/manager.hpp"
#include <alc/plugin/AftFuncDeclConsumer.hpp>
#include <alc/ast/statements.hpp>
#include <clang/AST/Expr.h>

ast::expression* to_alc_expression(ast::manager& store, const Expr* expr)
{
    auto castExpr = dyn_cast<CastExpr>(expr);
    if (castExpr)
    {
        return to_alc_expression(store, castExpr->getSubExpr());
    }

    auto intLit = dyn_cast<const IntegerLiteral>(expr);
    if (intLit)
    {
        return store.make_expression<ast::integer>(intLit->getValue().getZExtValue());
    }

    auto declRef = dyn_cast<DeclRefExpr>(expr);
    if (declRef)
    {
        auto varName = declRef->getDecl()->getNameAsString();
        return store.make_expression<ast::var>(varName);
    }

    auto binExpr = dyn_cast<BinaryOperator>(expr);
    if (binExpr)
    {
        auto lhs = to_alc_expression(store, binExpr->getLHS());
        if (!lhs)
            return nullptr;

        auto rhs = to_alc_expression(store, binExpr->getRHS());
        if (!rhs)
            return nullptr;

        if (binExpr->getOpcode() == BinaryOperator::Opcode::BO_Add)
            return store.make_expression<ast::add>(lhs, rhs);

        if (binExpr->getOpcode() == BinaryOperator::Opcode::BO_Mul)
            return store.make_expression<ast::multiply>(lhs, rhs);

        return nullptr;
    }

    return nullptr;
}

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

void MatchVariableDeclarationCallback::run(const MatchFinder::MatchResult& result)
{
    llvm::outs() << "Var decl!\n";

    auto& nodes = result.Nodes;

    auto varDecl = nodes.getNodeAs<VarDecl>("varDecl");
    auto newValue = nodes.getNodeAs<Expr>("newValue");

    auto& store = visitor.ast_store();
    auto alcExpr = to_alc_expression(store, newValue);

    if (!alcExpr)
    {
        llvm::outs() << "Unsupported expression:\n";
        varDecl->dump();
        return;
    }

    auto decl = store.make_statement<ast::decl>(varDecl->getNameAsString(), alcExpr);

    visitor.push_statement(decl, {});
}

void MatchVariableAssignmentCallback::run(const MatchFinder::MatchResult& result)
{
    llvm::outs() << "Var assignment!\n";

    auto& nodes = result.Nodes;

    auto varDecl = nodes.getNodeAs<VarDecl>("varDecl");
    auto newValue = nodes.getNodeAs<Expr>("newValue");

    auto& store = visitor.ast_store();
    auto alcExpr = to_alc_expression(store, newValue);

    if (!alcExpr)
    {
        llvm::outs() << "Unsupported expression:\n";
        varDecl->dump();
        return;
    }

    auto assign = store.make_statement<ast::assign>(varDecl->getNameAsString(), alcExpr);

    visitor.push_statement(assign, {});
}
