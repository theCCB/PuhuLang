#include "AST.h"
#include "AstVisitor.hpp"

void ExprBinary::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}

void ExprCall::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}

void ExprCast::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}

void ExprLiteral::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}

void ExprUnary::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}

void ExprVariable::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}

void StmtBlock::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}

void StmtFunc::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}

void StmtExpr::accept(AstVisitor* visitor)
{
    visitor->visit(this);
}