#pragma once

#include "Scanner.h"
#include "Value.hpp"

enum class ExprType
{
    Assignment,
    Binary,
    Call,
    Cast,
    Literal,
    Logic,
    Unary,
    Variable
};

class AstVisitor;

class Expr
{
public:
    const ExprType instance;
    std::shared_ptr<Type> type;

    Expr(ExprType instance, std::shared_ptr<Type> type)
        : instance(instance), type(type)
    {
    }

    virtual ~Expr() {}

    virtual void accept(AstVisitor* visitor) = 0;
};

class ExprAssignment : public Expr
{
public:
    Token name;
    Expr* assignment;

    ExprAssignment(Token name, Expr* assignment)
        : Expr(ExprType::Assignment, std::make_shared<TypePrimitive>(TypeTag::NULL_TYPE)), name(name), assignment(assignment)
    {
    }

    ~ExprAssignment()
    {
        delete assignment;
    }

    void accept(AstVisitor* visitor);
};

class ExprBinary : public Expr
{
public:
    Expr* left;
    Expr* right;
    Token op;

    ExprBinary(Expr* left, Expr* right, Token op)
        : Expr(ExprType::Binary, std::make_shared<TypePrimitive>(TypeTag::NULL_TYPE)), left(left), right(right), op(op)
    {
    }

    ~ExprBinary()
    {
        delete left;
        delete right;
    }

    void accept(AstVisitor* visitor);
};

class ExprCall : public Expr
{
public:
    Expr* callee;
    std::vector<Expr*> args;
    Token openParen;

    ExprCall(Expr* callee, std::vector<Expr*> args, Token openParen)
        : Expr(ExprType::Call, callee->type->intrinsicType), callee(callee), args(args), openParen(openParen)
    {
    }

    ~ExprCall()
    {
        delete callee;
        for (auto& arg : args)
            delete arg;
    }

    void accept(AstVisitor* visitor);
};

class ExprCast : public Expr
{
public:
    std::shared_ptr<Type> from;
    std::shared_ptr<Type> to;
    Expr* expr;

    ExprCast(std::shared_ptr<Type> to, Expr* expr)
        : Expr(ExprType::Cast, to), from(std::make_shared<TypePrimitive>(TypeTag::NULL_TYPE)), to(to), expr(expr)
    {
    }

    ~ExprCast()
    {
        delete expr;
    }

    void accept(AstVisitor* visitor);
};

class ExprUnary : public Expr
{
public:
    Expr* expr;
    Token op;

    ExprUnary(Expr* expr, Token op)
        : Expr(ExprType::Unary, std::make_shared<TypePrimitive>(TypeTag::NULL_TYPE)), expr(expr), op(op)
    {
    }

    ~ExprUnary()
    {
        delete expr;
    }

    void accept(AstVisitor* visitor);
};

class ExprVariable : public Expr
{
public:
    Token name;

    ExprVariable(Token name)
        : Expr(ExprType::Variable, std::make_shared<TypePrimitive>(TypeTag::NULL_TYPE)), name(name)
    {
    }

    void accept(AstVisitor* visitor);
};

class ExprLiteral : public Expr
{
public:
    Value* val;

    ExprLiteral(Value* val)
        : Expr(ExprType::Literal, val->type), val(val)
    {
    }
    void accept(AstVisitor* visitor);
};

class ExprLogic : public Expr
{
public:
    Expr* left;
    Expr* right;
    Token op;

    ExprLogic(Expr* left, Expr* right, Token op)
        : Expr(ExprType::Logic, std::make_shared<TypePrimitive>(TypeTag::BOOL)), left(left), right(right), op(op)
    {
    }

    ~ExprLogic()
    {
        delete left;
        delete right;
    }

    void accept(AstVisitor* visitor);
};

class Stmt
{
public:
    virtual ~Stmt(){}
    virtual void accept(AstVisitor* visitor) = 0;
};

class StmtExpr : public Stmt
{
public:
    Expr* expr;

    StmtExpr(Expr* expr)
        : expr(expr)
    {
    }

    ~StmtExpr()
    {
        delete expr;
    }

    void accept(AstVisitor* visitor);
};

class StmtBlock : public Stmt
{
public:
    std::vector<Stmt*> statements;

    StmtBlock(std::vector<Stmt*> statements)
        : statements(statements)
    {
    }

    ~StmtBlock()
    {
        for (auto& stmt : statements)
            delete stmt;
    }

    void accept(AstVisitor* visitor);
};

class StmtFunc : public Stmt
{
public:
    Token name;
    StmtBlock* body;
    std::shared_ptr<TypeFunction> func_type;
    std::vector<Token> args;

    StmtFunc(Token name, StmtBlock* body, std::shared_ptr<TypeFunction> func_type, std::vector<Token> args)
        : name(name), func_type(func_type), body(body), args(args)
    {
    }

    ~StmtFunc()
    {
        delete body;
    }

    void accept(AstVisitor* visitor);
};

class StmtVarDecleration : public Stmt
{
public:
    std::shared_ptr<Type> varType;
    Token name;
    Expr* initializer;

    StmtVarDecleration(std::shared_ptr<Type> varType, Token name, Expr* initializer)
        : varType(varType), name(name), initializer(initializer)
    {
    }

    ~StmtVarDecleration()
    {
        if (initializer)
            delete initializer;
    }

    void accept(AstVisitor* visitor);
};

class StmtReturn : public Stmt
{
public:
    Expr* retVal;

    StmtReturn(Expr* retVal)
        : retVal(retVal)
    {
    }

    ~StmtReturn()
    {
        delete retVal;
    }

    void accept(AstVisitor* visitor);
};

class StmtIf : public Stmt
{
public:
    Expr* condition;
    Stmt* then;
    Stmt* els;
    Token paren;

    StmtIf(Expr* condition, Stmt* then, Stmt* els, Token paren)
        : condition(condition), then(then), els(els), paren(paren)
    {
    }

    ~StmtIf()
    {
        delete condition;
        delete then;
        if (els)
            delete els;
    }

    void accept(AstVisitor* visitor);
};

class StmtFor : public Stmt
{
public:
    Stmt* decl;
    Expr* cond;
    Expr* inc;
    Stmt* loop;
    Token paren;

    StmtFor(Stmt* decl, Expr* cond, Expr* inc, Stmt* loop, Token paren)
        : decl(decl), cond(cond), inc(inc), loop(loop), paren(paren)
    {
    }

    ~StmtFor()
    {
        if (decl)
            delete decl;
        delete cond;
        if (inc)
            delete inc;
        delete loop;
    }

    void accept(AstVisitor* visitor);
};

class StmtWhile : public Stmt
{
public:
    Expr* condition;
    Stmt* loop;
    Token paren;

    StmtWhile(Expr* condition, Stmt* loop, Token paren)
        : condition(condition), loop(loop), paren(paren)
    {
    }

    ~StmtWhile()
    {
        delete condition;
        delete loop;
    }

    void accept(AstVisitor* visitor);
};
