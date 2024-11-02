#ifndef _token
#define _token
#include <string>

enum class TokenType
{
    IF,
    WHILE,
    FOR,
    IDENTIFIER,
    RETURN,
    FUNC,
    VAR,
    NUMBER,
    STRING,
    CHAR,
    TYPE,
    BREAK,
    END_EXPR,
    OPERATOR_EQ,
    OPERATOR_NEQ,
    OPERATOR_NEG,
    OPERATOR_GT,
    OPERATOR_GTE,
    OPERATOR_LT,
    OPERATOR_LTE,
    OPERATOR_LOG_AND,
    OPERATOR_LOG_OR,
    OPERATOR_BIN_AND,
    OPERATOR_BIN_OR,
    OPERATOR_BIN_XOR,
    OPERATOR_BIN_LSHIFT,
    OPERATOR_BIN_RSHIFT,
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_MULT,
    OPERATOR_DIV,
    OPERATOR_MOD,
    OPERATOR_ASSIGN,
    OPERATOR_TYPE,
};

class Token
{
public:
    std::string value;
    TokenType type;

    Token(std::string value, TokenType type)
    {
        this->value = value;
        this->type = type;
    }

    bool operator==(const Token &other) const
    {
        return other.type == this->type && other.value == this->value;
    }

    bool operator<(const Token &other) const
    {
        return other.type == this->type && other.value < this->value;
    }

    bool operator>(const Token &other) const
    {
        return other.type == this->type && other.value > this->value;
    }
};

namespace std
{
    template <>
    struct hash<Token>
    {
        std::size_t operator()(const Token &other) const
        {
            return std::hash<std::string>()(other.value);
        }
    };
}

std::unordered_map<std::string, Token *> keywords = {
    {"if", new Token("if", TokenType::IF)},
    {"for", new Token("for", TokenType::FOR)},
    {"while", new Token("while", TokenType::WHILE)},
    {"return", new Token("return", TokenType::RETURN)},
    {"break", new Token("break", TokenType::BREAK)},
    {"func", new Token("func", TokenType::FUNC)},
    {"var", new Token("var", TokenType::VAR)},
    {"byte", new Token("byte", TokenType::TYPE)},
    {"char", new Token("char", TokenType::TYPE)},
    {"short", new Token("short", TokenType::TYPE)},
    {"int", new Token("int", TokenType::TYPE)},
    {"long", new Token("long", TokenType::TYPE)},
};

std::unordered_map<std::string, Token *> operators = {
    {"==", new Token("==", TokenType::OPERATOR_EQ)},
    {">=", new Token(">=", TokenType::OPERATOR_GTE)},
    {"<=", new Token("<=", TokenType::OPERATOR_LTE)},
    {"<", new Token("<", TokenType::OPERATOR_LT)},
    {">", new Token(">", TokenType::OPERATOR_GT)},
    {"!=", new Token("!=", TokenType::OPERATOR_NEQ)},
    {"&&", new Token("&&", TokenType::OPERATOR_LOG_AND)},
    {"||", new Token("||", TokenType::OPERATOR_LOG_OR)},
    {"&", new Token("&", TokenType::OPERATOR_BIN_AND)},
    {"|", new Token("|", TokenType::OPERATOR_BIN_OR)},
    {"^", new Token("^", TokenType::OPERATOR_BIN_XOR)},
    {">>", new Token(">>", TokenType::OPERATOR_BIN_LSHIFT)},
    {"<<", new Token("<<", TokenType::OPERATOR_BIN_RSHIFT)},
    {"!", new Token("!", TokenType::OPERATOR_NEG)},
    {"+", new Token("+", TokenType::OPERATOR_PLUS)},
    {"-", new Token("-", TokenType::OPERATOR_MINUS)},
    {"*", new Token("*", TokenType::OPERATOR_MULT)},
    {"%", new Token("%", TokenType::OPERATOR_MOD)},
    {"/", new Token("/", TokenType::OPERATOR_DIV)},
    {"=", new Token("=", TokenType::OPERATOR_ASSIGN)},
    {":", new Token(":", TokenType::OPERATOR_TYPE)},
    {";", new Token(";", TokenType::END_EXPR)},
};

#endif