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

void LexicalScanner::match(std::string s) {
    std::string toMatch = getName();
    if (s == toMatch) return;
    throw new std::exception();
}

Token* LexicalScanner::get() {
    std::string value = getName();
    if (value.empty()) return nullptr;
    char c = value[0];

    if (keywords.find(value) != keywords.end()) {
        return keywords[value];
    }
    if (operators.find(value) != operators.end()) {
        return operators[value];
    }
    if (std::isdigit(c)) {
        return new Token(value, TokenType::NUMBER);
    }
    if (std::isalpha(c) || c == '_') {
        return new Token(value, TokenType::IDENTIFIER);
    }

    throw new std::exception();
}

std::string LexicalScanner::getName() {
    passBlanks();
    std::string s = "";
    while (lookChar() && !std::isspace(lookChar())) s += getChar();
    return s;
}

char LexicalScanner::getChar() {
    char c = lookChar();
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

void inline LexicalScanner::passBlanks() {
    while (std::isspace(lookChar())) getChar();
}