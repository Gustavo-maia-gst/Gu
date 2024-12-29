#include "parser.h"
#include "ast/ast.h"
#include <string>
#include <unordered_map>

std::unordered_map<std::string, int> typeMapper = {
    {"func", ProgramTokenType::FUNC},
    {"var", ProgramTokenType::VAR},
    {"if", ProgramTokenType::IF},
    {"while", ProgramTokenType::WHILE},
    {"for", ProgramTokenType::FOR},
    {"return", ProgramTokenType::RETURN},
    {"break", ProgramTokenType::BREAK},

    {"enum", ProgramTokenType::ENUM},
    {"struct", ProgramTokenType::STRUCT},

    {"NUMBER", ProgramTokenType::NUMBER},
    {"STRING", ProgramTokenType::STRING},
    {"CHAR", ProgramTokenType::CHAR},

    {"(", ProgramTokenType::OPEN_PAR},
    {")", ProgramTokenType::CLOSE_PAR},
    {"[", ProgramTokenType::OPEN_BRACKETS},
    {"]", ProgramTokenType::CLOSE_BRACKETS},
    {"{", ProgramTokenType::OPEN_BRACES},
    {"}", ProgramTokenType::CLOSE_BRACES},
    {".", ProgramTokenType::DOT},
    {",", ProgramTokenType::COMMA},
    {";", ProgramTokenType::SEMICOLON},
};

AstParser::AstParser(Lexer *lexer) { 
  this->lexer = lexer;
  auto typeMapperRef = &typeMapper;
  lexer->setTypeMapper(typeMapperRef);
}

ProgramNode *AstParser::parseProgram() {
  auto node = new ProgramNode;

  while (lexer->next().rawType != TokenType::END_OF_INPUT) {
    auto current = lexer->look();

    if (current.mappedType == FUNC)
      node->children.push_back(parseFunction(node));
    else if (current.mappedType == VAR)
      node->children.push_back(parseVarDef(node));
    else
      sintax_error("Unexpected token : " + current.raw);
  }

  return node;
}

FunctionNode *AstParser::parseFunction(AstNode *parent) {
  Token ident = nextExpected(IDENTIFIER, "Expecting identifier");

  auto node = new FunctionNode(parent, ident.raw);

  nextExpected(OPEN_PAR, "Expecting (");
}

const Token &AstParser::nextExpected(ProgramTokenType expectedType,
                                     std::string errorMsg) {
  auto token = lexer->next();
  if (token.mappedType != expectedType)
    sintax_error(errorMsg);
  return lexer->look();
}

void AstParser::sintax_error(std::string msg) {
  auto current = lexer->look();
  std::cerr << "sintax error: " << msg << " at line "
            << std::to_string(current.line) << ':'
            << std::to_string(current.start) << std::endl;
  exit(1);
}