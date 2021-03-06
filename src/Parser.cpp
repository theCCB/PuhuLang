#include "Parser.h"
#include "AST.h"
#include "Enviroment.hpp"
#include "Natives.hpp"
#include "Value.hpp"

Parser::Parser()
    : currentToken(0), depth(0), currentNamespace(new EnvNamespace("", nullptr)), cont(true)
{
    std::vector<std::shared_ptr<Type>> s = {std::make_shared<TypePrimitive>(TypeTag::STRING)};
    std::shared_ptr<TypeFunction> v2s = std::make_shared<TypeFunction>(TypeTag::NATIVE, std::make_shared<TypePrimitive>(TypeTag::VOID), s, true);
    std::shared_ptr<TypeFunction> sr = std::make_shared<TypeFunction>(TypeTag::NATIVE, std::make_shared<TypePrimitive>(TypeTag::STRING), std::vector<std::shared_ptr<Type>>(), false);
    std::shared_ptr<TypeFunction> dr = std::make_shared<TypeFunction>(TypeTag::NATIVE, std::make_shared<TypePrimitive>(TypeTag::DOUBLE), std::vector<std::shared_ptr<Type>>(), false);
    std::shared_ptr<TypeFunction> ir = std::make_shared<TypeFunction>(TypeTag::NATIVE, std::make_shared<TypePrimitive>(TypeTag::INTEGER), std::vector<std::shared_ptr<Type>>(), false);
    std::shared_ptr<TypeFunction> fr = std::make_shared<TypeFunction>(TypeTag::NATIVE, std::make_shared<TypePrimitive>(TypeTag::FLOAT), std::vector<std::shared_ptr<Type>>(), false);

    currentNamespace->define("print", v2s, new NativeFunc(native_print, v2s));
    currentNamespace->define("input", sr, new NativeFunc(native_input, sr));
    currentNamespace->define("clock", dr, new NativeFunc(native_clock, dr));
    currentNamespace->define("inputInt", ir, new NativeFunc(native_inputInt, ir));
    currentNamespace->define("rand", fr, new NativeFunc(native_rand, fr));
    srand(time(NULL));

    allNamespaces.insert({"", currentNamespace});
}

StmtCompUnit* Parser::parseUnit(std::vector<Token> tokens)
{
    this->tokens = tokens;
    this->currentToken = 0;
    this->depth = 0;

    std::vector<Stmt*> root;
    while (!match(TokenType::EOF_TOKEN))
    {
        root.push_back(decleration());
    }

    return new StmtCompUnit(root);
}

void Parser::parseNamespaceInside(EnvNamespace* ns, std::vector<Token>& t, int& i)
{
    while (i < t.size())
    {
        if (t[i].type == TokenType::STRUCT && t[i + 1].type == TokenType::IDENTIFIER)
        {
            i++; // at name
            userTypes.insert({ns->getName() + "::" + t[i].getString(), std::make_shared<TypeStruct>(t[i])});
            i++; // at open brace
            while (t[i].type != TokenType::CLOSE_BRACE)
                i++;
        }
        else if (t[i].type == TokenType::NAMESPACE && t[i + 1].type == TokenType::IDENTIFIER)
        {
            i++; // at name
            EnvNamespace* closing = ns;
            std::string nsName = closing->getName() + "::" + t[i].getString();
            if (allNamespaces.find(nsName) == allNamespaces.end())
            {
                ns = new EnvNamespace(t[i].getString(), ns);
                allNamespaces.insert({nsName, ns});
            }
            else
                ns = allNamespaces[nsName];

            i++; // at open brace

            parseNamespaceInside(ns, t, i);

            ns = closing;
        }
        i++;
    }
}

void Parser::parseUserDefinedTypes(std::vector<Token> t)
{
    EnvNamespace* ns = currentNamespace;
    int i = 0;

    while (i < t.size())
    {
        parseNamespaceInside(ns, t, i);
    }
}

std::shared_ptr<Type> Parser::parseTypeName()
{
    std::shared_ptr<Type> type;
    TypeTag tag = getDataType();

    if (tag == TypeTag::ERROR)
        errorAtToken("Invalid type name.");

    if (tag <= TypeTag::ARRAY)
        type = std::make_shared<TypePrimitive>(tag);
    else if (tag == TypeTag::STRUCT)
        type = lastType;
    else
        errorAtToken("Initial type of any type must a primitive or a class type.");

    while ((tag = getDataType()) != TypeTag::ERROR)
    {
        if (tag < TypeTag::ARRAY)
            errorAtToken("Invalid type decleration.");
        else if (tag == TypeTag::ARRAY)
        {
            type = std::make_shared<TypeArray>(consumed().getInteger(), type);
            advance();
        }
        else if (tag == TypeTag::POINTER)
        {
            bool is_owner = consumed().type == TokenType::BIT_AND;
            type = std::make_shared<TypePointer>(is_owner, type);
        }
    }

    return type;
}

bool Parser::isTypeName(Token& token)
{
    switch (token.type)
    {
    case TokenType::INT:
    case TokenType::FLOAT:
    case TokenType::DOUBLE:
    case TokenType::CHAR:
    case TokenType::STRING:
    case TokenType::BOOL:
    case TokenType::VOID:
        return true;
    case TokenType::DOUBLE_COLON:
        return peekNext().type == TokenType::IDENTIFIER;
    case TokenType::IDENTIFIER:
    {
        if(peekNext().type == TokenType::DOUBLE_COLON)
            return true;
        else
            return userTypes.find(currentNamespace->getName() + "::" + token.getString()) != userTypes.end();
    }
    default:
        return false;
    }
}

TypeTag Parser::getDataType()
{
    Token token = peek();
    switch (token.type)
    {
    case TokenType::INT:
        advance();
        return TypeTag::INTEGER;
    case TokenType::FLOAT:
        advance();
        return TypeTag::FLOAT;
    case TokenType::DOUBLE:
        advance();
        return TypeTag::DOUBLE;
    case TokenType::CHAR:
        advance();
        return TypeTag::CHAR;
    case TokenType::STRING:
        advance();
        return TypeTag::STRING;
    case TokenType::BOOL:
        advance();
        return TypeTag::BOOL;
    case TokenType::VOID:
        advance();
        return TypeTag::VOID;
    case TokenType::STAR:
        advance();
        return TypeTag::POINTER;
    case TokenType::BIT_AND:
        advance();
        return TypeTag::POINTER;
    case TokenType::OPEN_BRACKET:
        advance();
        if (peek().type == TokenType::INTEGER_LITERAL && peekNext().type == TokenType::CLOSE_BRACKET)
        {
            advance();
            return TypeTag::ARRAY;
        }
        else
            return TypeTag::ERROR;
    case TokenType::IDENTIFIER:
        if (userTypes.find(currentNamespace->getName() + "::" + token.getString()) != userTypes.end())
        {
            advance();
            lastType = userTypes[currentNamespace->getName() + "::" + token.getString()];
            return TypeTag::STRUCT;
        }
        else
            return TypeTag::ERROR;
    default:
        return TypeTag::ERROR;
    }
}

void Parser::consume(TokenType type, const char* message)
{
    if (tokens[currentToken].type == type)
    {
        advance();
    }
    else
    {
        errorAtToken(message);
    }
}

void Parser::consumeNext(TokenType type, const char* message)
{
    if (tokens[currentToken + 1].type == type)
    {
        advance();
    }
    else
    {
        errorAtToken(message);
    }
}

bool Parser::match(TokenType type)
{
    TokenType next = tokens[currentToken].type;
    if (next == type)
    {
        currentToken++;
        return true;
    }

    return false;
}

bool Parser::match(std::vector<TokenType> types)
{
    for (auto& token : types)
    {
        if (match(token))
            return true;
    }
    return false;
}

bool Parser::matchCast()
{
    // TODO: fix
    return tokens[currentToken].type == TokenType::OPEN_PAREN && isTypeName(tokens[currentToken + 1]) && tokens[currentToken + 2].type == TokenType::CLOSE_PAREN;
}

std::shared_ptr<Type> Parser::getCast()
{
    advance();
    std::shared_ptr<Type> to = this->parseTypeName();
    consume(TokenType::CLOSE_PAREN, "Expect ')' after cast type.");
    return to;
}

inline Expr* Parser::typeError(const char* message)
{
    this->cont = false;
    std::cout << "At " << tokens[currentToken - 1] << message << std::endl;
    return nullptr;
}

inline void Parser::error(const char* message)
{
    this->cont = false;
    std::cout << message << std::endl;
    this->panic();
}

inline void Parser::errorAtToken(const char* message)
{
    this->cont = false;
    std::cout << "[line " << tokens[currentToken].line << "] Error " << message << std::endl;
    this->panic();
}

void Parser::panic()
{
    TokenType type = tokens[currentToken].type;
    while (type != TokenType::EOF_TOKEN)
    {
        if (type == TokenType::SEMI_COLON)
        {
            // advance();
            return;
        }

        switch (type)
        {
        case TokenType::USING:
        case TokenType::NAMESPACE:
        case TokenType::STRUCT:
        case TokenType::IF:
        case TokenType::INT:
        case TokenType::FLOAT:
        case TokenType::DOUBLE:
        case TokenType::CHAR:
        case TokenType::STRING:
        case TokenType::BOOL:
        case TokenType::VOID:
        case TokenType::WHILE:
        case TokenType::FOR:
        case TokenType::RETURN:
            return;

        case TokenType::IDENTIFIER:
            if (isTypeName(tokens[currentToken]))
            return;

        default:
            break;
        }

        advance();
        type = peek().type;
    }
}

Stmt* Parser::decleration()
{
    TokenType token = peek().type;
    if (token == TokenType::STRUCT)
    {
        return structDecleration();
    }
    else if (token == TokenType::NAMESPACE)
    {
        return namespaceDecleration();
    }
    else
    {
        std::shared_ptr<Type> typeName;
        if (peek().type == TokenType::VAR)
        {
            typeName = std::make_shared<TypePrimitive>(TypeTag::AUTO);
            advance();
        }
        else
            typeName = parseTypeName();

        if (peekNext().type == TokenType::OPEN_PAREN)
        {
            return functionDecleration(typeName);
        }
        else
        {
            return variableDecleration(typeName);
        }
    }

    return nullptr;
}

Stmt* Parser::structDecleration()
{
    Token classTok = advance();
    Token name = advance();
    std::shared_ptr<TypeStruct> type = userTypes[currentNamespace->getName() + "::" + name.getString()];
    consume(TokenType::OPEN_BRACE, "Expect '{' after a class decleration.");

    StmtStruct* stmtClass = new StmtStruct(type, name);
    std::unordered_map<std::string, StmtFunc*> methodes;

    while (isTypeName(peek()))
    {
        std::shared_ptr<Type> typeName = parseTypeName();
        Token varName = advance();
        consume(TokenType::SEMI_COLON, "Expect ';' after member variable decleration.");
        stmtClass->type->addMember(typeName, varName);
    }

    consume(TokenType::CLOSE_BRACE, "Expect '}' at the end of a class decleration.");

    return stmtClass;
}

Stmt* Parser::namespaceDecleration()
{
    advance();
    Token nameToken = advance();
    std::string name = nameToken.getString();

    std::vector<Stmt*> stmts;

    consume(TokenType::OPEN_BRACE, "Expect '{' after a namespace decleration.");

    currentNamespace = allNamespaces[currentNamespace->getName() + "::" + name];

    while(!match(TokenType::CLOSE_BRACE))
    {
        stmts.push_back(decleration());
    }

    currentNamespace = currentNamespace->closing;

    return new StmtNamespace(name, stmts);
}

Stmt* Parser::variableDecleration(std::shared_ptr<Type> type)
{
    Token varName = advance();
    Expr* init = nullptr;
    if (match(TokenType::EQUAL))
    {
        if (match(TokenType::HEAP))
        {
            Token token = consumed();
            std::shared_ptr<Type> type = parseTypeName();
            init = new ExprHeap(type, token);
        }
        else
            init = parseExpression();
    }

    consume(TokenType::SEMI_COLON, "Expect ';' after variable decleration.");

    if (depth == 0)
    {
        currentNamespace->define(varName.getString(), type, new Value(type));
    }

    return new StmtVarDecleration(type, varName, init);
}

Stmt* Parser::functionDecleration(std::shared_ptr<Type> type)
{
    Token funcName = advance();
    std::vector<Token> params;
    std::vector<std::shared_ptr<Type>> param_types;
    size_t totalSize = 0;

    this->depth++;
    advance(); // Open Paren (

    if (peek().type != TokenType::CLOSE_PAREN)
    {
        std::shared_ptr<Type> type = parseTypeName();
        Token name = advance();
        params.push_back(name);
        param_types.push_back(type);
        totalSize++;
    }
    while (!match(TokenType::CLOSE_PAREN))
    {
        consume(TokenType::COMMA, "Expect ',' between parameters."); // consume ','
        std::shared_ptr<Type> type = parseTypeName();
        Token name = advance();
        params.push_back(name);
        param_types.push_back(type);
        totalSize++;
    }

    consume(TokenType::OPEN_BRACE, "Expect '{' at function start."); // Opening {
    StmtBlock* body = (StmtBlock*)block();

    std::shared_ptr<TypeFunction> funcType = std::make_shared<TypeFunction>(TypeTag::FUNCTION, type, param_types, false);

    this->depth--;
    currentNamespace->define(funcName.getString(), funcType, new FuncValue(funcType));

    return new StmtFunc(funcName, body, funcType, params);
}

Stmt* Parser::statement()
{
    TokenType token = peek().type;
    if (match(TokenType::OPEN_BRACE))
    {
        return block();
    }
    else if (token == TokenType::IF)
    {
        return ifStatement();
    }
    else if (token == TokenType::FOR)
    {
        return forStatement();
    }
    else if (token == TokenType::WHILE)
    {
        return whileStatement();
    }
    else if (token == TokenType::RETURN)
    {
        advance();
        Expr* ret = parseExpression();
        consume(TokenType::SEMI_COLON, "Expect ';' after a return statement.");
        return new StmtReturn(ret);
    }
    else
    {
        Expr* expr = parseExpression();
        consume(TokenType::SEMI_COLON, "Expect ';' after an expression statement.");
        return new StmtExpr(expr);
    }
}

Stmt* Parser::block()
{
    this->depth++;

    std::vector<Stmt*> stmts;
    while (peek().type != TokenType::CLOSE_BRACE && peek().type != TokenType::EOF_TOKEN)
    {
        if (isTypeName(peek()))
            stmts.push_back(variableDecleration(parseTypeName()));
        else if (match(TokenType::VAR))
            stmts.push_back(variableDecleration(std::make_shared<TypePrimitive>(TypeTag::AUTO)));
        else
            stmts.push_back(statement());
    }
    consume(TokenType::CLOSE_BRACE, "Expect '}' after a block.");

    this->depth--;
    return new StmtBlock(stmts);
}

Stmt* Parser::ifStatement()
{
    advance();
    consume(TokenType::OPEN_PAREN, "Expect '(' after 'if' keyword.");
    Token paren = consumed();
    Expr* condition = parseExpression();
    consume(TokenType::CLOSE_PAREN, "Expect ')' after 'if' condition.");
    Stmt* then = statement();
    Stmt* els = nullptr;
    if (match(TokenType::ELSE))
        els = statement();
    return new StmtIf(condition, then, els, paren);
}

Stmt* Parser::forStatement()
{
    advance();
    consume(TokenType::OPEN_PAREN, "Expect '(' after 'for' keyword.");
    Token paren = consumed();
    Stmt* decl = nullptr;
    Expr* cond = nullptr;
    Expr* inc = nullptr;
    if (!match(TokenType::SEMI_COLON))
    {
        if (isTypeName(peek()))
            decl = variableDecleration(parseTypeName());
        else
        {
            decl = new StmtExpr(parseExpression());
            consume(TokenType::SEMI_COLON, "Expect ';' after first part of 'for' statement.");
        }
    }

    if (!match(TokenType::SEMI_COLON))
    {
        cond = parseExpression();
        consume(TokenType::SEMI_COLON, "Expect ';' after second part of 'for' statement.");
    }
    else
    {
        cond = new ExprLiteral(new Value(true));
    }

    if (peek().type != TokenType::CLOSE_PAREN)
    {
        inc = parseExpression();
    }

    consume(TokenType::CLOSE_PAREN, "Expect ')' after 'for' conditions.");

    Stmt* loop = statement();
    return new StmtFor(decl, cond, inc, loop, paren);
}

Stmt* Parser::whileStatement()
{
    advance();
    consume(TokenType::OPEN_PAREN, "Expect '(' after 'while' keyword.");
    Token paren = consumed();
    Expr* condition = parseExpression();
    consume(TokenType::CLOSE_PAREN, "Expect ')' after 'while' condition.");
    Stmt* loop = statement();
    return new StmtWhile(condition, loop, paren);
}

Expr* Parser::parseExpression()
{
    return assignment();
}

Expr* Parser::assignment()
{
    Expr* expr = logic_or();

    if (match({TokenType::EQUAL, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, TokenType::STAR_EQUAL, TokenType::SLASH_EQUAL,
               TokenType::BIT_AND_EQUAL, TokenType::BIT_OR_EQUAL, TokenType::BIT_XOR_EQUAL, TokenType::BITSHIFT_LEFT_EQUAL, TokenType::BITSHIFT_RIGHT_EQUAL}))
    {
        Token t = consumed();
        Expr* asgn;

        if (match(TokenType::HEAP))
        {
            Token token = consumed();
            std::shared_ptr<Type> type = parseTypeName();
            asgn = new ExprHeap(type, token);
        }
        else
            asgn = parseExpression();

        switch (t.type)
        {
        case TokenType::PLUS_EQUAL:
            t.type = TokenType::PLUS;
            break;
        case TokenType::MINUS_EQUAL:
            t.type = TokenType::MINUS;
            break;
        case TokenType::STAR_EQUAL:
            t.type = TokenType::STAR;
            break;
        case TokenType::SLASH_EQUAL:
            t.type = TokenType::SLASH;
            break;
        case TokenType::BIT_AND_EQUAL:
            t.type = TokenType::BIT_AND;
            break;
        case TokenType::BIT_OR_EQUAL:
            t.type = TokenType::BIT_OR;
            break;
        case TokenType::BIT_XOR_EQUAL:
            t.type = TokenType::BIT_XOR;
            break;
        case TokenType::BITSHIFT_LEFT:
            t.type = TokenType::BITSHIFT_LEFT;
            break;
        case TokenType::BITSHIFT_RIGHT_EQUAL:
            t.type = TokenType::BITSHIFT_RIGHT;
            break;
        }

        if (t.type != TokenType::EQUAL)
            asgn = new ExprBinary(expr, asgn, t);

        if (expr->instance == ExprType::Variable)
        {
            Token name = ((ExprVariable*)expr)->name;
            return new ExprAssignment(name, asgn);
        }
        else if (expr->instance == ExprType::ArrGet)
        {
            ExprArrGet* e = (ExprArrGet*)expr;
            expr = new ExprArrSet(e->callee, e->index, asgn, e->bracket);
            return expr;
        }
        else if (expr->instance == ExprType::GetDeref)
        {
            ExprGetDeref* e = (ExprGetDeref*)expr;
            expr = new ExprSetDeref(e->callee, asgn, e->token, t);
            return expr;
        }
        else if (expr->instance == ExprType::Get)
        {
            ExprGet* e = (ExprGet*)expr;
            expr = new ExprSet(e->callee, asgn, e->get);
            return expr;
        }

        errorAtToken("Invalid assignment target.");
    }

    return expr;
}

Expr* Parser::logic_or()
{
    Expr* left = logic_and();
    while (match(TokenType::OR))
    {
        Token op = consumed();
        Expr* right = logic_and();
        left = new ExprLogic(left, right, op);
    }
    return left;
}

Expr* Parser::logic_and()
{
    Expr* left = bit_or();
    while (match(TokenType::AND))
    {
        Token op = consumed();
        Expr* right = bit_or();
        left = new ExprLogic(left, right, op);
    }
    return left;
}

Expr* Parser::bit_or()
{
    Expr* left = bit_xor();
    while (match(TokenType::BIT_OR))
    {
        Token op = consumed();
        Expr* right = bit_xor();
        left = new ExprBinary(left, right, op);
    }
    return left;
}

Expr* Parser::bit_xor()
{
    Expr* left = bit_and();
    while (match(TokenType::BIT_XOR))
    {
        Token op = consumed();
        Expr* right = bit_and();
        left = new ExprBinary(left, right, op);
    }
    return left;
}

Expr* Parser::bit_and()
{
    Expr* left = equality();
    while (match(TokenType::BIT_AND))
    {
        Token op = consumed();
        Expr* right = equality();
        left = new ExprBinary(left, right, op);
    }
    return left;
}

Expr* Parser::equality()
{
    Expr* left = comparison();
    while (match(TokenType::EQUAL_EQUAL) || match(TokenType::BANG_EQUAL))
    {
        Token op = consumed();
        Expr* right = comparison();

        left = new ExprBinary(left, right, op);
    }
    return left;
}

Expr* Parser::comparison()
{
    Expr* left = bitshift();
    while (match(TokenType::LESS) || match(TokenType::LESS_EQUAL) || match(TokenType::GREAT) || match(TokenType::GREAT_EQUAL))
    {
        Token op = consumed();
        Expr* right = bitshift();

        left = new ExprBinary(left, right, op);
    }
    return left;
}

Expr* Parser::bitshift()
{
    Expr* left = addition();
    while (match(TokenType::BITSHIFT_LEFT) || match(TokenType::BITSHIFT_RIGHT))
    {
        Token op = consumed();
        Expr* right = addition();
        left = new ExprBinary(left, right, op);
    }
    return left;
}

Expr* Parser::addition()
{
    Expr* left = multiplication();
    while (match(TokenType::PLUS) || match(TokenType::MINUS))
    {
        Token op = consumed();
        Expr* right = multiplication();

        left = new ExprBinary(left, right, op);
    }

    return left;
}

Expr* Parser::multiplication()
{
    Expr* left = prefix();
    while (match(TokenType::STAR) || match(TokenType::SLASH) || match(TokenType::MODULUS))
    {
        Token op = consumed();
        Expr* right = prefix();

        left = new ExprBinary(left, right, op);
    }

    return left;
}

Expr* Parser::prefix()
{
    if (match(TokenType::MINUS) || match(TokenType::PLUS) || match(TokenType::MINUS_MINUS) || match(TokenType::PLUS_PLUS))
    {
        Token op = consumed();
        Expr* expr = unary();
        return new ExprUnary(expr, op);
    }
    else
        return unary();
}

Expr* Parser::unary()
{
    if (match(TokenType::BANG) || match(TokenType::TILDE))
    {
        Token op = consumed();
        Expr* expr = unary();
        return new ExprUnary(expr, op);
    }
    else if (match(TokenType::BIT_XOR))
    {
        Token token = consumed();
        Expr* expr = unary();
        return new ExprGetDeref(expr, token);
    }
    else if (match(TokenType::REF))
    {
        Token token = consumed();
        Expr* expr = unary();
        return new ExprRef(expr, token);
    }
    else if (match(TokenType::TAKE))
    {
        Token token = consumed();
        Expr* expr = unary();
        return new ExprTake(expr, token);
    }
    else if (match(TokenType::BIT_AND))
    {
        Token token = consumed();
        Expr* expr = unary();
        return new ExprAddr(expr, token);
    }
    else if (matchCast())
    {
        std::shared_ptr<Type> type = getCast();
        Expr* expr = unary();
        return new ExprCast(type, expr);
    }
    else
        return postfix();
}

Expr* Parser::postfix()
{
    // TODO: Wire it
    return call();
}

Expr* Parser::call()
{
    Expr* expr = primary();

    while (match({TokenType::OPEN_PAREN, TokenType::OPEN_BRACKET, TokenType::DOT, TokenType::ARROW}))
    {
        Token token = consumed();
        if (token.type == TokenType::OPEN_PAREN)
        {
            std::vector<Expr*> args;
            Expr* callee = expr;
            if (expr->instance == ExprType::Get)
            {
                ExprGet* get = (ExprGet*)expr;
                args.push_back(get->callee);
                callee = new ExprVariable(get->get);
            }

            if (peek().type != TokenType::CLOSE_PAREN)
            {
                do
                {
                    args.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::CLOSE_PAREN, "Expect ')' after arguments.");
            expr = new ExprCall(callee, args, token);
        }
        else if (token.type == TokenType::OPEN_BRACKET)
        {
            Expr* index = parseExpression();
            consume(TokenType::CLOSE_BRACKET, "Expect ']' after array indexing.");
            expr = new ExprArrGet(expr, index, token);
        }
        else if (token.type == TokenType::DOT)
        {
            Token get = advance();
            expr = new ExprGet(expr, get);
        }
        else if (token.type == TokenType::ARROW)
        {
            Token get = advance();
            expr = new ExprGet(new ExprGetDeref(expr, get), get);
        }
    }

    return expr;
}

Expr* Parser::primary()
{
    Token token = advance();

    switch (token.type)
    {
    case TokenType::TRUE:
        return new ExprLiteral(new Value(true));
    case TokenType::FALSE:
        return new ExprLiteral(new Value(false));
    case TokenType::INTEGER_LITERAL:
        return new ExprLiteral(new Value(token.getInteger()));
    case TokenType::DOUBLE_LITERAL:
        return new ExprLiteral(new Value(token.getDouble()));
    case TokenType::FLOAT_LITERAL:
        return new ExprLiteral(new Value(token.getFloat()));
    case TokenType::CHAR_LITERAL:
        return new ExprLiteral(new Value(token.getChar()));
    case TokenType::STRING_LITERAL:
        return new ExprLiteral(new Value(token.getString()));
    case TokenType::OPEN_PAREN:
    {
        Expr* a = parseExpression();
        if (match(TokenType::CLOSE_PAREN))
            return a;
        else
            return typeError("Expect ')' after a grouping expression.");
    }

    case TokenType::IDENTIFIER:
        return new ExprVariable(token);

    default:
        std::cout << "[Error]Invalid identifier: " << token.getString() << std::endl;
        return nullptr;
    }
}