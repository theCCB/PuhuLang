#pragma once
#include "AstVisitor.hpp"
#include <iostream>

class AstDebugger : public ExprVisitor<void>, public StmtVisitor<void>
{
public:
    AstDebugger() = delete;
    explicit AstDebugger(std::vector<std::unique_ptr<Stmt>>& root, std::ostream& os = std::cout);

    void debug();
    void showTypes(bool isShow);
    
    void visit(ExprLogic* expr) override;
    void visit(ExprBinary* expr) override;
    void visit(ExprUnary* expr) override;
    void visit(ExprLiteral* expr) override;

    void visit(StmtExpr* stmt) override;
    
private:
    std::vector<std::unique_ptr<Stmt>>& root;
    std::ostream& os;
    bool canShowType;
};