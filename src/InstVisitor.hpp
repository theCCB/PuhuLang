#pragma once

#include "Instruction.h"

class InstVisitor
{
public:
    virtual void visit(InstConst* inst) = 0;
    virtual void visit(InstCast* inst) = 0;
    virtual void visit(InstAdd* inst) = 0;
    virtual void visit(InstSub* inst) = 0;
    virtual void visit(InstMul* inst) = 0;
    virtual void visit(InstDiv* inst) = 0;
    virtual void visit(InstNeg* inst) = 0;
    virtual void visit(InstMod* inst) = 0;
    virtual void visit(InstBit* inst) = 0;
    virtual void visit(InstNot* inst) = 0;
    virtual void visit(InstInc* inst) = 0;
    virtual void visit(InstLess* inst) = 0;
    virtual void visit(InstLte* inst) = 0;
    virtual void visit(InstGreat* inst) = 0;
    virtual void visit(InstGte* inst) = 0;
    virtual void visit(InstEq* inst) = 0;
    virtual void visit(InstNeq* inst) = 0;
    virtual void visit(InstGetGlobal* inst) = 0;
    virtual void visit(InstSetGlobal* inst) = 0;
    virtual void visit(InstGetLocal* inst) = 0;
    virtual void visit(InstSetLocal* inst) = 0;
    virtual void visit(InstAlloc* inst) = 0;
    virtual void visit(InstFree* inst) = 0;
    virtual void visit(InstGetDeref* inst) = 0;
    virtual void visit(InstSetDeref* inst) = 0;
    virtual void visit(InstGetDerefOff* inst) = 0;
    virtual void visit(InstSetDerefOff* inst) = 0;
    virtual void visit(InstAddrLocal* inst) = 0;
    virtual void visit(InstAddrGlobal* inst) = 0;
    virtual void visit(InstCall* inst) = 0;
    virtual void visit(InstPop* inst) = 0;
    virtual void visit(InstPush* inst) = 0;
    virtual void visit(InstReturn* inst) = 0;
    virtual void visit(InstLabel* inst) = 0;
    virtual void visit(InstJump* inst) = 0;
};