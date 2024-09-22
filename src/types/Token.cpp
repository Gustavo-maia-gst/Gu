#include <string>
#define _token

enum TokenType {
    IF,
    WHILE,
    FOR,
    IDENTIFIER,
    RETURN,
    NUMBER,
    STRING,
    CHAR,
    TYPE,
    OPERATOR,
    BREAK,
};

class Token {
public:
    std::string value;
    TokenType type;

    Token(std::string value, TokenType type) {
        this->value = value;
        this->type = type;
    }

    bool operator==(const Token& other) const {
        return other.type == this->type && other.value == this->value;
    }
};

namespace std {
    template <>
    struct hash<Token> {
        std::size_t operator()(const Token& other) const {
            return std::hash<std::string>()(other.value);
        }
    };
}

std::unordered_map<std::string, Token*> keywords = {
    { "if", new Token("if", TokenType::IF) },
    { "for", new Token("for", TokenType::FOR) },
    { "while", new Token("while", TokenType::WHILE) },
    { "return", new Token("return", TokenType::RETURN) },
    { "break", new Token("break", TokenType::BREAK) },
    { "byte", new Token("byte", TokenType::TYPE) },
    { "char", new Token("char", TokenType::TYPE) },
    { "short", new Token("short", TokenType::TYPE) },
    { "int", new Token("int", TokenType::TYPE) },
    { "long", new Token("long", TokenType::TYPE) },
};

std::unordered_map<std::string, Token*> operators = {
    { "=", new Token("=", TokenType::OPERATOR) },
    { ">=", new Token(">=", TokenType::OPERATOR) },
    { "<=", new Token("<=", TokenType::OPERATOR) },
    { "<", new Token("<", TokenType::OPERATOR) },
    { ">", new Token(">", TokenType::OPERATOR) },
    { "&&", new Token("&&", TokenType::OPERATOR) },
    { "||", new Token("||", TokenType::OPERATOR) },
    { "&", new Token("&", TokenType::OPERATOR) },
    { "|", new Token("|", TokenType::OPERATOR) },
    { "^", new Token("^", TokenType::OPERATOR) },
    { "!", new Token("!", TokenType::OPERATOR) },
    { "+", new Token("+", TokenType::OPERATOR) },
    { "-", new Token("-", TokenType::OPERATOR) },
    { "*", new Token("*", TokenType::OPERATOR) },
    { "/", new Token("/", TokenType::OPERATOR) },
    { ":=", new Token(":=", TokenType::OPERATOR) },
};