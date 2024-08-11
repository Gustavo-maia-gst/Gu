#include <string>

enum TokenType {
    IF,
    WHILE,
    FOR,
    IDENTIFIER,
    RETURN,
    NUMBER,
    STRING,
    CHAR,
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
        std::size_t operator()(const Token& token) const {
            return std::hash<std::string>()(token.value);
        }
    };
}
