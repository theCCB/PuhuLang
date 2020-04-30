#include "Scanner.h"
#include <sstream>
#include <string_view>
#include <iomanip>

Scanner::Scanner(std::string& source)
	:source(source), startPosition(0), currentPosition(0), line(1)
{}

std::vector<Token> Scanner::scanTokens()
{
	std::vector<Token> tokens;

	do
	{
		tokens.push_back(scanToken());
	} while (tokens.back().type != TokenType::EOF_TOKEN);

	return tokens;
}

char Scanner::advance()
{
	return source[this->currentPosition++];
}

bool Scanner::match(char c)
{
	if (this->isAtEnd() || source[this->currentPosition] != c)
		return false;

	this->currentPosition++;
	return true;
}

char Scanner::peekPrev()
{
	return source[this->currentPosition - 1];
}

char Scanner::peek()
{
	return source[this->currentPosition];
}

char Scanner::peekNext()
{
	if (this->isAtEnd())
		return '\0';
	return source[this->currentPosition + 1];
}

bool Scanner::isAtEnd()
{
	return this->currentPosition == source.length();
}

Token Scanner::scanToken()
{
	skipWhitespace();

	this->startPosition = currentPosition;
	if (this->isAtEnd())
		return makeToken(TokenType::EOF_TOKEN);

	char c = advance();
	switch (c)
	{
	case '{':
		return makeToken(TokenType::OPEN_BRACE);
	case '}':
		return makeToken(TokenType::CLOSE_BRACE);
	case '[':
		return makeToken(TokenType::OPEN_BRACKET);
	case ']':
		return makeToken(TokenType::CLOSE_BRACKET);
	case '(':
		return makeToken(TokenType::OPEN_PAREN);
	case ')':
		return makeToken(TokenType::CLOSE_PAREN);
	case '<':
		if (match('='))
			return makeToken(TokenType::LESS_EQUAL);
		if (match('<'))
			return makeToken(TokenType::BITSHIFT_LEFT);
		return makeToken(TokenType::LESS);
	case '>':
		if (match('='))
			return makeToken(TokenType::GREAT_EQUAL);
		if (match('>'))
			return makeToken(TokenType::BITSHIFT_RIGHT);
		return makeToken(TokenType::GREAT);
	case '.':
		return makeToken(TokenType::DOT);
	case ',':
		return makeToken(TokenType::COMMA);
	case ';':
		return makeToken(TokenType::SEMI_COLON);
	case ':':
		return makeToken(TokenType::COLON);
	case '*':
		if (match('='))
			return makeToken(TokenType::STAR_EQUAL);
		return makeToken(TokenType::STAR);
	case '+':
		if (match('+'))
			return makeToken(TokenType::PLUS_PLUS);
		if (match('='))
			return makeToken(TokenType::PLUS_EQUAL);
		return makeToken(TokenType::PLUS);
	case '-':
		if (match('-'))
			return makeToken(TokenType::MINUS_MINUS);
		if (match('='))
			return makeToken(TokenType::MINUS_EQUAL);
		if (match('>'))
			return makeToken(TokenType::ARROW);
		return makeToken(TokenType::MINUS);
	case '/':
		if (match('='))
			return makeToken(TokenType::SLASH_EQUAL);
		return makeToken(TokenType::SLASH);
	case '%':
		return makeToken(TokenType::MODULUS);
	case '!':
		if (match('='))
			return makeToken(TokenType::BANG_EQUAL);
		return makeToken(TokenType::BANG);
	case '=':
		if (match('='))
			return makeToken(TokenType::EQUAL_EQUAL);
		return makeToken(TokenType::EQUAL);
	case '&':
		if (match('='))
			return makeToken(TokenType::BIT_AND_EQUAL);
		if (match('&'))
			return makeToken(TokenType::AND);
		return makeToken(TokenType::BIT_AND);
	case '|':
		if (match('='))
			return makeToken(TokenType::BIT_OR_EQUAL);
		if (match('|'))
			return makeToken(TokenType::OR);
		return makeToken(TokenType::BIT_OR);
	case '^':
		if (match('='))
			return makeToken(TokenType::BIT_XOR_EQUAL);
		return makeToken(TokenType::BIT_XOR);
	case '~':
		return makeToken(TokenType::TILDE);

	case '"':
		return stringLiteral();
	case '\'':
		return charLiteral();

	default:
		if (this->isAlpha(c))
			return identifierLiteral();
		else if (this->isDigit(c))
			return numberLiteral();
		return errorToken("Unexpected character");
	}
}

Token Scanner::makeToken(TokenType type)
{
	return Token(type, line, &source[startPosition], currentPosition - startPosition);
}

Token Scanner::stringLiteral()
{
	while (peek() != '"' && !this->isAtEnd())
	{
		advance();
	}

	if (isAtEnd())
		return errorToken("Unterminated string.");

	advance();
	return Token(TokenType::STRING_LITERAL, line, &source[startPosition + 1], currentPosition - startPosition - 2);
}

Token Scanner::charLiteral()
{
	if (peekNext() == '\'')
	{
		advance();
		advance();
		return Token(TokenType::CHAR_LITERAL, line, &source[startPosition + 1], currentPosition - startPosition - 2);
	}
	else
	{
		return errorToken("Unterminated character.");
	}
}

Token Scanner::identifierLiteral()
{
	char c = peekPrev();
	std::stringstream lexeme;
	while ((this->isAlpha(peek()) || this->isDigit(peek())))
	{
		lexeme << peekPrev();
		c = advance();
	}
	lexeme << peekPrev();

	if (lexeme.str() == "using")
		return makeToken(TokenType::USING);
	if (lexeme.str() == "namespace")
		return makeToken(TokenType::NAMESPACE);
	if (lexeme.str() == "class")
		return makeToken(TokenType::CLASS);
	if (lexeme.str() == "if")
		return makeToken(TokenType::IF);
	if (lexeme.str() == "else")
		return makeToken(TokenType::ELSE);
	if (lexeme.str() == "int")
		return makeToken(TokenType::INT);
	if (lexeme.str() == "float")
		return makeToken(TokenType::FLOAT);
	if (lexeme.str() == "double")
		return makeToken(TokenType::DOUBLE);
	if (lexeme.str() == "char")
		return makeToken(TokenType::CHAR);
	if (lexeme.str() == "string")
		return makeToken(TokenType::STRING);
	if (lexeme.str() == "bool")
		return makeToken(TokenType::BOOL);
	if (lexeme.str() == "void")
		return makeToken(TokenType::VOID);
	if (lexeme.str() == "while")
		return makeToken(TokenType::WHILE);
	if (lexeme.str() == "for")
		return makeToken(TokenType::FOR);
	if (lexeme.str() == "true")
		return makeToken(TokenType::TRUE);
	if (lexeme.str() == "false")
		return makeToken(TokenType::FALSE);
	if (lexeme.str() == "null")
		return makeToken(TokenType::NULL_TOKEN);
	if (lexeme.str() == "return")
		return makeToken(TokenType::RETURN);

	return makeToken(TokenType::IDENTIFIER);
}

Token Scanner::numberLiteral()
{
	while (this->isDigit(peek()))
	{
		advance();
	}
	if (match('.'))
	{
		while (this->isDigit(peek()))
		{
			advance();
		}
		if (match('f') || match('F'))
			return makeToken(TokenType::FLOAT_LITERAL);
		else
			return makeToken(TokenType::DOUBLE_LITERAL);
	}
	else if (match('f') || match('F'))
		return makeToken(TokenType::FLOAT_LITERAL);
	return makeToken(TokenType::INTEGER_LITERAL);
}

Token Scanner::errorToken(const char* msg)
{
	return Token(TokenType::ERROR, line, msg, strlen(msg));
}

void Scanner::skipWhitespace()
{
	while (true)
	{
		char c = peek();
		switch (c)
		{
		case ' ':
		case '\t':
		case '\r':
			advance();
			break;
		case '\n':
			this->line++;
			advance();
			break;

		case '/':
			if (peekNext() == '/')
			{
				while (peek() != '\n' && !this->isAtEnd())
					advance();
				skipWhitespace();
			}
			else if (peekNext() == '*')
			{
				while (!(peek() == '*' && peekNext() == '/') && !this->isAtEnd())
				{
					char x = advance();
					if (x == '\n')
						this->line++;
				}
				this->currentPosition += 2;
				skipWhitespace();
			}

		default:
			return;
		}
	}

}


std::ostream& operator<<(std::ostream& os, const Token& token)
{
	os << "Token length: " << token.length << ", line: " << std::setw(2) << (int)token.line << ", type: " << std::setw(2) << (int)token.type << ", lexeme: " << std::string_view(token.start, token.length);
	return os;
}

int Token::getInteger()
{
	char* lexeme = new char[length + 1];
	memcpy(lexeme, start, sizeof(char) * length);
	lexeme[length] = 0;
	return std::atoi(lexeme);
}

double Token::getDouble()
{
	char* lexeme = new char[length + 1];
	memcpy(lexeme, start, sizeof(char) * length);
	lexeme[length] = 0;
	return std::stod(lexeme);
}

float Token::getFloat()
{
	char* lexeme = new char[length];
	memcpy(lexeme, start, sizeof(char) * (length - 1));
	lexeme[length - 1] = 0;
	return std::atof(lexeme);
}

std::string Token::getString()
{
	char* lexeme = new char[length + 1];
	memcpy(lexeme, start, sizeof(char) * length);
	lexeme[length] = 0;
	return std::string(lexeme);;
}

char Token::getChar()
{
	return *start;
}
