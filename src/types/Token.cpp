#include "Token.h"

// Definições das variáveis
std::unordered_map<std::string, Token *> keywords = {
    {"if", new Token("if", TokenType::IF)},
    {"for", new Token("for", TokenType::FOR)},
    {"while", new Token("while", TokenType::WHILE)},
    {"return", new Token("return", TokenType::RETURN)},
    {"break", new Token("break", TokenType::BREAK)},
    {"func", new Token("func", TokenType::FUNC)},
    {"var", new Token("var", TokenType::VAR)},
    {"byte", new Token("byte", TokenType::TYPE)},
    {"char", new Token("char", TokenType::TYPE)},
    {"short", new Token("short", TokenType::TYPE)},
    {"int", new Token("int", TokenType::TYPE)},
    {"long", new Token("long", TokenType::TYPE)},
};

std::unordered_map<std::string, Token *> operators = {
    {"==", new Token("==", TokenType::OPERATOR_EQ)},
    {">=", new Token(">=", TokenType::OPERATOR_GTE)},
    {"<=", new Token("<=", TokenType::OPERATOR_LTE)},
    {"<", new Token("<", TokenType::OPERATOR_LT)},
    {">", new Token(">", TokenType::OPERATOR_GT)},
    {"!=", new Token("!=", TokenType::OPERATOR_NEQ)},
    {"&&", new Token("&&", TokenType::OPERATOR_LOG_AND)},
    {"||", new Token("||", TokenType::OPERATOR_LOG_OR)},
    {"&", new Token("&", TokenType::OPERATOR_BIN_AND)},
    {"|", new Token("|", TokenType::OPERATOR_BIN_OR)},
    {"^", new Token("^", TokenType::OPERATOR_BIN_XOR)},
    {">>", new Token(">>", TokenType::OPERATOR_BIN_LSHIFT)},
    {"<<", new Token("<<", TokenType::OPERATOR_BIN_RSHIFT)},
    {"!", new Token("!", TokenType::OPERATOR_NEG)},
    {"+", new Token("+", TokenType::OPERATOR_PLUS)},
    {"-", new Token("-", TokenType::OPERATOR_MINUS)},
    {"*", new Token("*", TokenType::OPERATOR_MULT)},
    {"%", new Token("%", TokenType::OPERATOR_MOD)},
    {"/", new Token("/", TokenType::OPERATOR_DIV)},
    {"=", new Token("=", TokenType::OPERATOR_ASSIGN)},
    {":", new Token(":", TokenType::OPERATOR_TYPE)},
    {";", new Token(";", TokenType::END_EXPR)},
};