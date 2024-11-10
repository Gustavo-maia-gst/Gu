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

void LexicalScanner::expect(TokenType type, std::string description)
{
    Token *token = this->get();
    if (token->type != type)
        sintax_error("expecting " + description + ", got " + token->value);
    delete token;
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

void LexicalScanner::unget(Token *token)
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
        Token *token = this->ungetted;
        this->ungetted = nullptr;
        return token;
    }

    return getToken();
}

Token *LexicalScanner::getToken()
{
    passBlanks();

    if (isalpha(lookChar()) || lookChar() == '_')
        return getTokenIndentifier();
    if (isdigit(lookChar()))
        return getTokenNumber();
    if (isOperator(lookChar()))
        return getTokenOperator();
    if (lookChar())
    {
        char c = getChar();
        auto token = grammarChars[c];
        if (!token)
            compile_error("Unexpected character " + c);
        return token->copy();
    }

    return nullptr;
}

Token *LexicalScanner::getTokenIndentifier()
{
    std::string s = "";
    while (isalnum((lookChar())) || lookChar() == '_')
        s += getChar();
    if (s.empty())
        return nullptr;

    if (keywords.find(s) != keywords.end())
        return keywords[s]->copy();

    return new Token(s, TokenType::IDENTIFIER);
}

Token *LexicalScanner::getTokenNumber()
{
    std::string s = "";
    while (isdigit(lookChar()))
        s += getChar();
    if (s.empty())
        return nullptr;
    return new Token(s, TokenType::NUMBER);
}

Token *LexicalScanner::getTokenOperator()
{
    std::string s = "";
    while (isOperator(lookChar()))
        s += getChar();

    auto token = operators[s];
    if (!token)
        compile_error("Unexpected value " + s);
    return token->copy();
}

bool LexicalScanner::isOperator(char c)
{
    static const std::unordered_set<char> operators = {'+', '-', '*', '/', '%', '=', '<', '>', '&', '|', '^', '!', '~'};
    return operators.find(c) != operators.end();
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