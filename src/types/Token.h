#ifndef _token
#define _token

#include <unordered_map>
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
    ANY,


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

extern std::unordered_map<std::string, Token *> keywords;
extern std::unordered_map<std::string, Token *> operators;

#endif