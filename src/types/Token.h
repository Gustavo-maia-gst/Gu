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
    LET,
    NUMBER,
    STRING,
    CHAR,
    TYPE,
    BREAK,
    END_EXPR,
    OPEN_PAR,
    CLOSE_PAR,
    OPEN_BRACKETS,
    CLOSE_BRACKETS,
    OPEN_BRACES,
    CLOSE_BRACES,
    TYPE_INDICATOR,
    COMMA,

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

    inline bool operator==(const Token &other) const
    {
        return other.type == this->type && other.value == this->value;
    }

    inline bool operator<(const Token &other) const
    {
        return other.type == this->type && other.value < this->value;
    }

    inline bool operator>(const Token &other) const
    {
        return other.type == this->type && other.value > this->value;
    }

    inline Token* copy()
    {
        return new Token(this->value, this->type);
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
extern std::unordered_map<char, Token *> grammarChars;
extern std::unordered_map<TokenType, std::string> reverseOperators;

#endif