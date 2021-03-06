#pragma once

#include "Chunk.hpp"
#include "IRChunk.hpp"
#include "InstVisitor.hpp"

#include <string>
#include <unordered_map>

struct valInfo
{
    size_t addr;
    size_t size;

    valInfo()
        : addr(0), size(0)
    {
    }

    valInfo(size_t addr, size_t size)
        : addr(addr), size(size)
    {
    }
};

class CodeGen : public InstVisitor
{
private:
    Chunk* chunk;
    std::vector<valInfo> constPositions;
    std::vector<std::pair<std::string, valInfo>> globalInfo;
    std::vector<Data> m_globals;
    int pos;
    bool hadError;

public:
    CodeGen(std::unordered_map<std::string, EnvNamespace*>& allNamespaces)
        : hadError(false)
    {
        m_globals.resize(EnvNamespace::currentPos);
        for (auto& ns : allNamespaces)
        {
            for (auto& var : ns.second->vars)
            {
                for (int i = 0; i < var.second.type->getSize(); i++)
                    m_globals[var.second.position + i] = var.second.val[i].data;
                globalInfo.push_back({var.second.fullName, valInfo(var.second.position, var.second.type->getSize())});
                delete var.second.val;
            }
        }
    }

    void generateCode(IRChunk* irChunk)
    {
        pos = 0;
        chunk = irChunk->chunk;
        constPositions.clear();
        for (auto& val : irChunk->getConstants())
        {
            size_t addr = chunk->addConstant(val->data);
            constPositions.push_back(valInfo(addr, val->type->getSize()));
        }

        for (auto& inst : irChunk->getCode())
        {
            inst->accept(this);
        }
    }

    inline std::vector<Data> getGlobals()
    {
        return m_globals;
    }

    void error(const char* msg)
    {
        // TODO: add which line to here
        this->hadError = true;
        std::cout << msg << std::endl;
    }

    void visit(InstConst* inst)
    {
        valInfo& c = constPositions[inst->id];
        if (c.addr > 255)
            error("Constant position is out of bounds[255].");

        chunk->addCode(OpCode::CONSTANT, c.addr);
        pos += 2;
    }
    void visit(InstCast* inst)
    {
        chunk->addCode(OpCode::CAST, (uint8_t)inst->from, (uint8_t)inst->to);
        pos += 3;
    }
    void visit(InstAdd* inst)
    {
        OpCode code;
        switch (inst->type)
        {
        case TypeTag::INTEGER:
            code = OpCode::IADD;
            break;
        case TypeTag::FLOAT:
            code = OpCode::FADD;
            break;
        case TypeTag::DOUBLE:
            code = OpCode::DADD;
            break;
        }
        chunk->addCode(code);
        pos += 1;
    }
    void visit(InstSub* inst)
    {
        OpCode code;
        switch (inst->type)
        {
        case TypeTag::INTEGER:
            code = OpCode::ISUB;
            break;
        case TypeTag::FLOAT:
            code = OpCode::FSUB;
            break;
        case TypeTag::DOUBLE:
            code = OpCode::DSUB;
            break;
        }
        chunk->addCode(code);
        pos += 1;
    }
    void visit(InstMul* inst)
    {
        OpCode code;
        switch (inst->type)
        {
        case TypeTag::INTEGER:
            code = OpCode::IMUL;
            break;
        case TypeTag::FLOAT:
            code = OpCode::FMUL;
            break;
        case TypeTag::DOUBLE:
            code = OpCode::DMUL;
            break;
        }
        chunk->addCode(code);
        pos += 1;
    }
    void visit(InstDiv* inst)
    {
        OpCode code;
        switch (inst->type)
        {
        case TypeTag::INTEGER:
            code = OpCode::IDIV;
            break;
        case TypeTag::FLOAT:
            code = OpCode::FDIV;
            break;
        case TypeTag::DOUBLE:
            code = OpCode::DDIV;
            break;
        }
        chunk->addCode(code);
        pos += 1;
    }
    void visit(InstNeg* inst)
    {
        OpCode code;
        switch (inst->type)
        {
        case TypeTag::INTEGER:
            code = OpCode::INEG;
            break;
        case TypeTag::FLOAT:
            code = OpCode::FNEG;
            break;
        case TypeTag::DOUBLE:
            code = OpCode::DNEG;
            break;
        }
        chunk->addCode(code);
        pos += 1;
    }
    void visit(InstMod* inst)
    {
        chunk->addCode(OpCode::MOD);
        pos += 1;
    }
    void visit(InstBit* inst)
    {
        OpCode code;
        switch (inst->op_type)
        {
        case TokenType::TILDE:
            code = OpCode::BIT_NOT;
            break;

        case TokenType::BIT_AND:
            code = OpCode::BIT_AND;
            break;

        case TokenType::BIT_OR:
            code = OpCode::BIT_OR;
            break;

        case TokenType::BIT_XOR:
            code = OpCode::BIT_XOR;
            break;

        case TokenType::BITSHIFT_LEFT:
            code = OpCode::BITSHIFT_LEFT;
            break;

        case TokenType::BITSHIFT_RIGHT:
            code = OpCode::BITSHIFT_RIGHT;
            break;
        }
        chunk->addCode(code);
        pos += 1;
    }
    void visit(InstNot* inst)
    {
        chunk->addCode(OpCode::LOGIC_NOT);
        pos += 1;
    }
    void visit(InstInc* inst)
    {
        OpCode code;
        switch (inst->type)
        {
        case TypeTag::INTEGER:
            code = inst->inc == 1 ? OpCode::IINC : OpCode::IDEC;
            break;
        case TypeTag::FLOAT:
            code = inst->inc == 1 ? OpCode::FINC : OpCode::FDEC;
            break;
        case TypeTag::DOUBLE:
            code = inst->inc == 1 ? OpCode::DINC : OpCode::DDEC;
            break;
        }
        chunk->addCode(code);
        pos += 1;
    }
    void visit(InstLess* inst)
    {
        if (inst->type == TypeTag::INTEGER)
            chunk->addCode(OpCode::ILESS);
        else
            chunk->addCode(OpCode::DLESS);
        pos += 1;
    }
    void visit(InstLte* inst)
    {
        if (inst->type == TypeTag::INTEGER)
            chunk->addCode(OpCode::ILESS_EQUAL);
        else
            chunk->addCode(OpCode::DLESS_EQUAL);
        pos += 1;
    }
    void visit(InstGreat* inst)
    {
        if (inst->type == TypeTag::INTEGER)
            chunk->addCode(OpCode::IGREAT);
        else
            chunk->addCode(OpCode::DGREAT);
        pos += 1;
    }
    void visit(InstGte* inst)
    {
        if (inst->type == TypeTag::INTEGER)
            chunk->addCode(OpCode::IGREAT_EQUAL);
        else
            chunk->addCode(OpCode::DGREAT_EQUAL);
        pos += 1;
    }
    void visit(InstEq* inst)
    {
        if (inst->type == TypeTag::INTEGER)
            chunk->addCode(OpCode::IIS_EQUAL);
        else
            chunk->addCode(OpCode::DIS_EQUAL);
        pos += 1;
    }
    void visit(InstNeq* inst)
    {
        if (inst->type == TypeTag::INTEGER)
            chunk->addCode(OpCode::INOT_EQUAL);
        else
            chunk->addCode(OpCode::DNOT_EQUAL);
        pos += 1;
    }
    void visit(InstGetGlobal* inst)
    {
        valInfo var;
        for (auto v : globalInfo)
            if (v.first == inst->name)
            {
                var = v.second;
                break;
            }

        size_t size = inst->type->getSize();

        if (inst->offset)
        {
            if (size == 1)
                chunk->addCode(OpCode::GET_GLOBAL_OFF, var.addr);
            else
            {
                chunk->addCode(OpCode::GET_GLOBAL_OFFN, var.addr, size);
                pos++;
            }
        }
        else
        {
            if (size == 1)
                chunk->addCode(OpCode::GET_GLOBAL, var.addr);
            else
            {
                chunk->addCode(OpCode::GET_GLOBALN, var.addr, size);
                pos++;
            }
        }

        pos += 2;
    }
    void visit(InstSetGlobal* inst)
    {
        valInfo var;
        for (auto v : globalInfo)
            if (v.first == inst->name)
            {
                var = v.second;
                break;
            }

        size_t size = inst->type->getSize();

        if (inst->offset)
        {
            if (size == 1)
                chunk->addCode(OpCode::SET_GLOBAL_OFF, var.addr);
            else
            {
                chunk->addCode(OpCode::SET_GLOBAL_OFFN, var.addr, size);
                pos++;
            }
        }
        else
        {
            if (size == 1)
                chunk->addCode(OpCode::SET_GLOBAL, var.addr);
            else
            {
                chunk->addCode(OpCode::SET_GLOBALN, var.addr, size);
                pos++;
            }
        }

        pos += 2;
    }
    void visit(InstGetLocal* inst)
    {
        Variable& var = inst->var;
        size_t size = inst->type->getSize();

        if (inst->offset)
        {
            if (size == 1)
                chunk->addCode(OpCode::GET_LOCAL_OFF, var.position);
            else
            {
                chunk->addCode(OpCode::GET_LOCAL_OFFN, var.position, size);
                pos++;
            }
        }
        else
        {
            if (size == 1)
                chunk->addCode(OpCode::GET_LOCAL, var.position);
            else
            {
                chunk->addCode(OpCode::GET_LOCALN, var.position, size);
                pos++;
            }
        }

        pos += 2;
    }
    void visit(InstSetLocal* inst)
    {
        Variable& var = inst->var;
        size_t size = inst->type->getSize();

        if (inst->offset)
        {
            if (size == 1)
                chunk->addCode(OpCode::SET_LOCAL_OFF, var.position);
            else
            {
                chunk->addCode(OpCode::SET_LOCAL_OFFN, var.position, size);
                pos++;
            }
        }
        else
        {
            if (size == 1)
                chunk->addCode(OpCode::SET_LOCAL, var.position);
            else
            {
                chunk->addCode(OpCode::SET_LOCALN, var.position, size);
                pos++;
            }
        }

        pos += 2;
    }
    void visit(InstAlloc* inst)
    {
        chunk->addCode(OpCode::ALLOC, inst->type->getSize());
        pos += 2;
    }
    void visit(InstFree* inst)
    {
        chunk->addCode(OpCode::FREE);
        pos += 1;
    }
    void visit(InstGetDeref* inst)
    {
        chunk->addCode(OpCode::GET_DEREF, inst->type->getSize());
        pos += 2;
    }
    void visit(InstSetDeref* inst)
    {
        chunk->addCode(OpCode::SET_DEREF, inst->type->getSize());
        pos += 2;
    }
    void visit(InstGetDerefOff* inst)
    {
        chunk->addCode(OpCode::GET_DEREF_OFF, inst->type->getSize());
        pos += 2;
    }
    void visit(InstSetDerefOff* inst)
    {
        chunk->addCode(OpCode::SET_DEREF_OFF, inst->type->getSize());
        pos += 2;
    }
    void visit(InstAddrLocal* inst)
    {
        Variable& var = inst->var;
        size_t size = inst->type->getSize();

        if (inst->offset)
            chunk->addCode(OpCode::ADDR_LOCAL_OFF, var.position, size);
        else
            chunk->addCode(OpCode::ADDR_LOCAL, var.position);

        pos += 2;
    }
    void visit(InstAddrGlobal* inst)
    {
        valInfo var;
        for (auto v : globalInfo)
            if (v.first == inst->name)
            {
                var = v.second;
                break;
            }

        size_t size = inst->type->getSize();

        if (inst->offset)
            chunk->addCode(OpCode::ADDR_GLOBAL, var.addr);
        else
            chunk->addCode(OpCode::ADDR_GLOBAL_OFF, var.addr);

        pos += 2;
    }
    void visit(InstCall* inst)
    {
        size_t size = 0;
        for (auto& arg : inst->args)
            size += arg->getSize();

        if (size > 255)
            error("[ERROR] Too much data for call.");

        if (inst->callType == TypeTag::FUNCTION)
            chunk->addCode(OpCode::CALL, size);
        else if (inst->callType == TypeTag::NATIVE)
            chunk->addCode(OpCode::NATIVE_CALL, size);
        pos += 2;
    }
    void visit(InstPop* inst)
    {
        size_t size = 0;
        for (auto& t : inst->types)
            size += t->getSize();

        if (size > 0)
        {
            if (size > 255)
                error("[ERROR] Too much data for pop.");
            chunk->addCode(OpCode::POPN, size);
            pos += 2;
        }
    }
    void visit(InstPush* inst)
    {
        size_t size = 0;
        for (auto& t : inst->types)
            size += t->getSize();

        if (size > 0)
        {
            if (size > 255)
                error("[ERROR] Too much data for push.");
            chunk->addCode(OpCode::PUSHN, size);
            pos += 2;
        }
    }
    void visit(InstReturn* inst)
    {
        size_t size = inst->type->getSize();
        chunk->addCode(OpCode::RETURN, size);
        pos += 2;
    }
    void visit(InstLabel* inst)
    {
        inst->pos = pos;
        for (auto& jmp : inst->patches)
        {
            int diff = pos - jmp->pos - 2;
            if (diff > 255)
                error("Jump is too long.");
            chunk->code[jmp->pos + 1] = diff;
        }
        pos += 0;
    }
    void visit(InstJump* inst)
    {
        inst->pos = pos;
        if (inst->type == 0)
        {
            if (inst->label->pos > 0)
            {
                chunk->addCode(OpCode::LOOP, pos + 2 - inst->label->pos);
            }
            else
            {
                inst->label->patches.push_back(inst);
                chunk->addCode(OpCode::JUMP, 0);
            }
        }
        else
        {
            inst->label->patches.push_back(inst);
            if (inst->type == 1)
                chunk->addCode(OpCode::JUMP_NT, 0);
            else
                chunk->addCode(OpCode::JUMP_NT_POP, 0);
        }

        pos += 2;
    }
};
