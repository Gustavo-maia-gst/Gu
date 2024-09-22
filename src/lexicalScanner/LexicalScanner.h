#ifndef base
#include "../types/Base.cpp"
#endif
#ifndef _token
#include "../types/Token.cpp"
#endif
#define _scanner

class LexicalScanner {
public:
    LexicalScanner(std::string path);
    ~LexicalScanner();
    void expected(std::string expected, std::string got);
    void error(std::string msg);
    void match(std::string s);
    Token* get();

private:
    std::ifstream* file;
    std::queue<char> buff;

    std::string getName();
    char getChar();
    char lookChar();
    bool reloadBuffer();
    void passBlanks();
};
