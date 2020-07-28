// #define DEBUG_CODE_TRACE

#include "VM.h"

#ifdef DEBUG_CODE_TRACE
#include "Debug.h"
#endif // DEBUG_CODE_TRACE

VM::VM(std::vector<Data> globals)
    : ip(0), currentChunk(nullptr), globals(globals)
{
#ifdef DEBUG_CODE_TRACE
    std::cout << "Globals: ";
    printStack(this->globals);
#endif
    stack.reserve(128);
}

VM::~VM()
{
}

bool VM::interpret(Chunk* entryChunk)
{
    this->currentChunk = entryChunk;
    while (ip < this->currentChunk->code.size())
    {
#ifdef DEBUG_CODE_TRACE
        printStack(this->stack);
        dissambleInstruction(currentChunk, this->ip);
#endif // DEBUG_CODE_TRACE

#define BINARY_OP(op, type)                         \
    do                                              \
    {                                               \
        auto b = stack.back().type;                 \
        stack.pop_back();                           \
        stack.back().type = stack.back().type op b; \
    } while (false)

#define CMP_OP(op, type)            \
    do                              \
    {                               \
        auto b = stack.back().type; \
        stack.pop_back();           \
        auto a = stack.back().type; \
        stack.pop_back();           \
        Data d;                     \
        d.valBool = a op b;         \
        stack.push_back(d);         \
    } while (false)

        switch ((OpCode)advance())
        {
        case OpCode::CONSTANT:
        {
            Data constant = currentChunk->getConstant(advance());
            stack.push_back(constant);
            break;
        }
        case OpCode::IADD:
        {
            BINARY_OP(+, valInt);
            break;
        }
        case OpCode::ISUB:
        {
            BINARY_OP(-, valInt);
            break;
        }
        case OpCode::IMUL:
        {
            BINARY_OP(*, valInt);
            break;
        }
        case OpCode::IDIV:
        {
            BINARY_OP(/, valInt);
            break;
        }
        case OpCode::INEG:
        {
            stack.back().valInt *= -1;
            break;
        }
        case OpCode::MOD:
        {
            BINARY_OP(%, valInt);
            break;
        }
        case OpCode::FADD:
        {
            BINARY_OP(+, valFloat);
            break;
        }
        case OpCode::FSUB:
        {
            BINARY_OP(-, valFloat);
            break;
        }
        case OpCode::FMUL:
        {
            BINARY_OP(*, valFloat);
            break;
        }
        case OpCode::FDIV:
        {
            BINARY_OP(/, valFloat);
            break;
        }
        case OpCode::FNEG:
        {
            stack.back().valFloat *= -1;
            break;
        }
        case OpCode::DADD:
        {
            BINARY_OP(+, valDouble);
            break;
        }
        case OpCode::DSUB:
        {
            BINARY_OP(-, valDouble);
            break;
        }
        case OpCode::DMUL:
        {
            BINARY_OP(*, valDouble);
            break;
        }
        case OpCode::DDIV:
        {
            BINARY_OP(/, valDouble);
            break;
        }
        case OpCode::DNEG:
        {
            stack.back().valDouble *= -1;
            break;
        }
        case OpCode::BIT_NOT:
        {
            stack.back().valInt = ~stack.back().valInt;
            break;
        }
        case OpCode::BIT_AND:
        {
            BINARY_OP(&, valInt);
            break;
        }
        case OpCode::BIT_OR:
        {
            BINARY_OP(|, valInt);
            break;
        }
        case OpCode::BIT_XOR:
        {
            BINARY_OP(^, valInt);
            break;
        }
        case OpCode::BITSHIFT_LEFT:
        {
            BINARY_OP(<<, valInt);
            break;
        }
        case OpCode::BITSHIFT_RIGHT:
        {
            BINARY_OP(>>, valInt);
            break;
        }
        case OpCode::IINC:
        {
            stack.back().valInt++;
            break;
        }
        case OpCode::IDEC:
        {
            stack.back().valInt--;
            break;
        }
        case OpCode::FINC:
        {
            stack.back().valFloat++;
            break;
        }
        case OpCode::FDEC:
        {
            stack.back().valFloat--;
            break;
        }
        case OpCode::DINC:
        {
            stack.back().valDouble++;
            break;
        }
        case OpCode::DDEC:
        {
            stack.back().valDouble--;
            break;
        }
        case OpCode::LOGIC_NOT:
        {
            stack.back().valBool = !stack.back().valBool;
            break;
        }
        case OpCode::DLESS:
        {
            CMP_OP(<, valDouble);
            break;
        }
        case OpCode::DGREAT:
        {
            CMP_OP(>, valDouble);
            break;
        }
        case OpCode::ILESS:
        {
            CMP_OP(<, valInt);
            break;
        }
        case OpCode::IGREAT:
        {
            CMP_OP(>, valInt);
            break;
        }
        case OpCode::DLESS_EQUAL:
        {
            CMP_OP(<=, valDouble);
            break;
        }
        case OpCode::DGREAT_EQUAL:
        {
            CMP_OP(>=, valDouble);
            break;
        }
        case OpCode::DIS_EQUAL:
        {
            CMP_OP(==, valDouble);
            break;
        }
        case OpCode::DNOT_EQUAL:
        {
            CMP_OP(!=, valDouble);
            break;
        }
        case OpCode::ILESS_EQUAL:
        {
            CMP_OP(<=, valInt);
            break;
        }
        case OpCode::IGREAT_EQUAL:
        {
            CMP_OP(>=, valInt);
            break;
        }
        case OpCode::IIS_EQUAL:
        {
            CMP_OP(==, valInt);
            break;
        }
        case OpCode::INOT_EQUAL:
        {
            CMP_OP(!=, valInt);
            break;
        }
        case OpCode::CAST:
        {
            TypeTag from = (TypeTag)advance();
            TypeTag to = (TypeTag)advance();
            typeCast(from, to);
            break;
        }
        case OpCode::POPN:
        {
            int toPop = advance();
            stack.resize(stack.size() - toPop);
            break;
        }
        case OpCode::PUSHN:
        {
            int toPop = advance();
            stack.resize(stack.size() + toPop);
            break;
        }
        case OpCode::SET_GLOBAL:
        {
            size_t slot = advance();
            Data val = stack.back();
            globals.push_back(val);
            break;
        }
        case OpCode::GET_GLOBAL:
        {
            size_t slot = advance();
            stack.push_back(globals[slot]);
            break;
        }
        case OpCode::SET_LOCAL:
        {
            uint8_t slot = advance();
            stack[slot + this->frames.back().frameStart] = stack.back();
            break;
        }
        case OpCode::GET_LOCAL:
        {
            uint8_t slot = advance();
            stack.push_back(stack[slot + this->frames.back().frameStart]);
            break;
        }
        case OpCode::SET_GLOBAL_POP:
        {
            uint8_t slot = advance();
            Data val = stack.back();
            stack.pop_back();
            globals.push_back(val);
            break;
        }
        case OpCode::SET_LOCAL_POP:
        {
            uint8_t slot = advance();
            stack[slot + this->frames.back().frameStart] = stack.back();
            stack.pop_back();
            break;
        }
        case OpCode::JUMP:
        {
            uint8_t offset = advance();
            this->ip += offset;
            break;
        }
        case OpCode::JUMP_NT_POP:
        {
            uint8_t offset = advance();
            if (!stack.back().valBool)
                this->ip += offset;
            stack.pop_back();
            break;
        }
        case OpCode::LOOP:
        {
            uint8_t offset = advance();
            this->ip -= offset;
            break;
        }
        case OpCode::JUMP_NT:
        {
            uint8_t offset = advance();
            if (!stack.back().valBool)
                this->ip += offset;
            break;
        }
        case OpCode::CALL:
        {
            uint8_t argSize = advance();
            Chunk* func = stack.back().valChunk;
            stack.pop_back();
            this->frames.push_back(Frame(ip, currentChunk, this->stack.size() - argSize));
            this->currentChunk = func;
            this->ip = 0;
            break;
        }
        case OpCode::NATIVE_CALL:
        {
            uint8_t argSize = advance();
            NativeFn func = stack.back().valNative;
            stack.pop_back();
            Data result = func(argSize, argSize > 0 ? &stack[this->stack.size() - argSize] : nullptr);
            this->stack.resize(this->stack.size() - argSize);
            stack.push_back(result);
            break;
        }
        case OpCode::RETURN:
        {
            uint8_t size = advance();

            auto frame = this->frames.back();
            this->frames.pop_back();

            if (size != 0)
            {
                stack[frame.frameStart] = stack.back();
            }
            this->stack.resize(frame.frameStart + size);

            this->ip = frame.ip;
            this->currentChunk = frame.chunk;

            break;
        }
        }
    }

#ifdef DEBUG_CODE_TRACE
    printStack(this->stack);
#endif // DEBUG_CODE_TRACE

    return true;
}

void VM::typeCast(TypeTag from, TypeTag to)
{
#define CAST(value)          \
    Data d;                  \
    switch (to)              \
    {                        \
    case TypeTag::INTEGER:   \
        d.valInt = value;    \
        break;               \
    case TypeTag::FLOAT:     \
        d.valFloat = value;  \
        break;               \
    case TypeTag::DOUBLE:    \
        d.valDouble = value; \
        break;               \
    case TypeTag::BOOL:      \
        d.valBool = value;   \
        break;               \
    case TypeTag::CHAR:      \
        d.valChar = value;   \
        break;               \
    }                        \
    stack.push_back(d);

    Data data = stack.back();
    stack.pop_back();

    switch (from)
    {
    case TypeTag::INTEGER:
    {
        CAST(data.valInt);
        break;
    }
    case TypeTag::FLOAT:
    {
        CAST(data.valFloat);
        break;
    }
    case TypeTag::DOUBLE:
    {
        CAST(data.valDouble);
        break;
    }
    case TypeTag::BOOL:
    {
        CAST(data.valBool);
        break;
    }
    case TypeTag::CHAR:
    {
        CAST(data.valChar);
        break;
    }
    }
}