#include "LexicalScanner.h"

LexicalScanner::LexicalScanner(std::string filename)
{
    std::ifstream *file = new std::ifstream(filename);
    if (!file->is_open())
        compile_error("File could not be opened");
    this->file = file;
}

LexicalScanner::~LexicalScanner()
{
    delete this->file;
}

void LexicalScanner::match(std::string s)
{
    std::string toMatch = getName();
    if (s == toMatch)
        return;
    sintax_error("expected: " + s + ", got: " + toMatch);
}

void LexicalScanner::expect(TokenType type, std::string description)
{
    Token *token = this->get();
    if (token->type != type)
        sintax_error("expecting " + description + ", got " + token->value);
}

Token *LexicalScanner::get_expected(TokenType type, std::string description)
{
    Token *token = this->get();
    if (!token)
        sintax_error("Unexpected EOF when expecting " + description);
    if (token->type != type)
        sintax_error("Expecting " + description + ", got " + token->value);

    return token;
}

void LexicalScanner::unget(Token* token)
{
    if (this->ungetted)
    {
        std::cerr << "Ungetted two tokens" << std::endl;
        exit(1);
    }
    this->ungetted = token;
}

Token *LexicalScanner::get()
{
    if (this->ungetted)
    {
        Token* token = this->ungetted;
        this->ungetted = nullptr;
        return token;
    }

    std::string value = getName();
    if (value.empty())
        return nullptr;
    char c = value[0];

    if (keywords.find(value) != keywords.end())
    {
        return keywords[value];
    }
    if (operators.find(value) != operators.end())
    {
        return operators[value];
    }
    if (std::isdigit(c))
    {
        return new Token(value, TokenType::NUMBER);
    }
    if (std::isalpha(c) || c == '_')
    {
        return new Token(value, TokenType::IDENTIFIER);
    }

    if (!value.empty())
        return new Token(value, TokenType::ANY);
    
    return nullptr;
}

std::string LexicalScanner::getName()
{
    passBlanks();
    std::string s = "";
    while (lookChar() && !std::isspace(lookChar()))
        s += getChar();
    return s;
}

char LexicalScanner::getChar()
{
    char c = lookChar();
    buff.pop();
    return c;
}

char LexicalScanner::lookChar()
{
    if (buff.empty())
    {
        bool hasData = reloadBuffer();
        if (!hasData)
            return '\0';
    }
    char c = buff.front();
    return c;
}

bool LexicalScanner::reloadBuffer()
{
    char buff[1024];
    file->read(buff, 1024);
    int readen = file->gcount();
    if (!readen)
        return false;
    for (int i = 0; i < readen; i++)
    {
        this->buff.push(buff[i]);
    }
    return true;
}

void inline LexicalScanner::passBlanks()
{
    while (std::isspace(lookChar()))
        getChar();
}