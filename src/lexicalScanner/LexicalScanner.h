#ifndef _scanner
#define _scanner
#include "../types/Base.cpp"
#include "../types/Token.cpp"

class LexicalScanner
{
public:
    LexicalScanner(std::string path);
    ~LexicalScanner();
    void match(std::string s);
    void expect(TokenType type, std::string description);
    Token *get();
    Token *get_expected(TokenType type, std::string description);
    char lookChar();

private:
    std::ifstream *file;
    std::queue<char> buff;

    std::string getName();
    char getChar();
    bool reloadBuffer();
    void passBlanks();
};
#endif