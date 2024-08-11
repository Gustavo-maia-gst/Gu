#include "LexicalScanner.h"

LexicalScanner::LexicalScanner(std::string filename) {
    std::ifstream* file = new std::ifstream(filename);
    if (file == nullptr)
        throw new std::exception();
    this->file = file;
}

LexicalScanner::~LexicalScanner() {
    delete this->file;
}

void LexicalScanner::expected(std::string expected, std::string got) {
    std::cerr <<  "expected: " << expected << ", found: " << got << std::endl;
    exit(1);
}

void LexicalScanner::error(std::string msg) {
    std::cerr <<  "syntax error: " << msg << std::endl;
    exit(1);
}

Token* LexicalScanner::look() {
        
}

void LexicalScanner::match(std::string s) {

}

Token* LexicalScanner::get() {
    if (std::isdigit(lookChar()) || lookChar() == '_') {
        std::string value = getName();
        return new Token(value, TokenType::IDENTIFIER);
    }
}

std::string LexicalScanner::getName() {
    if (blanks.couy)
    std::string name = getChar();
}

char LexicalScanner::getChar() {
    if (buff.empty()) {
        bool hasData = reloadBuffer();
        if (!hasData) return '\0';
    }
    char c = buff.front();
    buff.pop();
    return c;
}

char LexicalScanner::lookChar() {
    if (buff.empty()) {
        bool hasData = reloadBuffer();
        if (!hasData) return '\0';
    }
    char c = buff.front();
    return c;
}

bool LexicalScanner::reloadBuffer() {
    char buff[1024];
    file->read(buff, 1024);
    int readen = file->gcount();
    if (!readen) return false;
    for (int i = 0; i < readen; i++)
        this->buff.push(buff[i]);
    return true;
}
