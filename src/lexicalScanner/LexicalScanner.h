#ifndef _base
#include "../types/Base.cpp"
#endif
#ifndef _token
#include "../types/Token.cpp"
#endif
#define _scanner

class LexicalScanner
{
public:
    LexicalScanner(std::string path);
    ~LexicalScanner();
    void match(std::string s);
    void expect(TokenType type, std::string description);
    Token *get();
    Token *get_expected(TokenType type, std::string description);

private:
    std::ifstream *file;
    std::queue<char> buff;

    std::string getName();
    char getChar();
    char lookChar();
    bool reloadBuffer();
    void passBlanks();
};
