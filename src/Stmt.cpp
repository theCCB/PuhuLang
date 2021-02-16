#include "Stmt.hpp"
#include "AstVisitor.hpp"

template<typename T>
T Stmt::accept(StmtVisitor<T>* visitor) 
{
    do_accept(visitor);
    return visitor->result;
}

template<>
void Stmt::accept(StmtVisitor<void>* visitor) 
{
    do_accept(visitor);
}

void StmtExpr::do_accept(StmtVisitorBase* visitor) 
{
    visitor->visit(this);
}