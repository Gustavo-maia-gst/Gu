#ifndef types
#include "../types/Base.cpp"
#include "../types/Token.cpp"
#endif

class LexicalScanner {
public:
    LexicalScanner(std::string path);
    ~LexicalScanner();
    void expected(std::string expected, std::string got);
    void error(std::string msg);
    Token* look();
    void match(std::string s);
    Token* get();

private:
    std::ifstream* file;
    std::queue<char> buff;

    std::unordered_map<std::string, Token*> keywords = {
        { "if", new Token("if", TokenType::IF) },
        { "for", new Token("for", TokenType::FOR) },
        { "while", new Token("while", TokenType::WHILE) },
        { "return", new Token("return", TokenType::RETURN) },
        { "break", new Token("break", TokenType::BREAK) },
    };

    std::unordered_map<std::string, Token*> operators = {
        { "=", new Token("=", TokenType::OPERATOR) },
        { ">=", new Token(">=", TokenType::OPERATOR) },
        { "<=", new Token("<=", TokenType::OPERATOR) },
        { "<", new Token("<", TokenType::OPERATOR) },
        { ">", new Token(">", TokenType::OPERATOR) },
        { "&&", new Token("&&", TokenType::OPERATOR) },
        { "||", new Token("||", TokenType::OPERATOR) },
        { "!", new Token("!", TokenType::OPERATOR) },
        { "+", new Token("+", TokenType::OPERATOR) },
        { "-", new Token("-", TokenType::OPERATOR) },
        { "*", new Token("*", TokenType::OPERATOR) },
        { "/", new Token("/", TokenType::OPERATOR) },
        { ":=", new Token(":=", TokenType::OPERATOR) },
    };

    std::unordered_set<char> blanks = { ' ', '\t', '\r', '\n'};

    std::string getName();
    char getChar();
    char lookChar();
    bool reloadBuffer();
};
