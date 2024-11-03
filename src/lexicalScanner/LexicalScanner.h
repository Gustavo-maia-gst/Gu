#ifndef _scanner
#define _scanner
#include "../types/Base.cpp"
#include "../types/Token.h"

class LexicalScanner
{
public:
    LexicalScanner(std::string path);
    ~LexicalScanner();
    void match(std::string s);
    void expect(TokenType type, std::string description);
    Token *get();
    Token *get_expected(TokenType type, std::string description);
    void unget(Token *token);
    char lookChar();

private:
    std::ifstream *file;
    std::queue<char> buff;
    Token* ungetted;

    std::string getName();
    char getChar();
    bool reloadBuffer();
    void passBlanks();
};
#endif