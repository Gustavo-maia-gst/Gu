#include "parser.h"
#include "ast/ast.h"
#include <cstdlib>
#include <stack>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>

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

    {"=", ProgramTokenType::ASSIGN},
    {"+", ProgramTokenType::PLUS},
    {"-", ProgramTokenType::MINUS},
    {"*", ProgramTokenType::MULT},
    {"/", ProgramTokenType::DIV},
    {"%", ProgramTokenType::MOD},
    {"+=", ProgramTokenType::PLUS_EQ},
    {"-=", ProgramTokenType::MINUS_EQ},
    {"*=", ProgramTokenType::MULT_EQ},
    {"/=", ProgramTokenType::DIV_EQ},
    {"%=", ProgramTokenType::MOD_EQ},
    {"==", ProgramTokenType::EQ},
    {"!=", ProgramTokenType::NEQ},
    {">", ProgramTokenType::GT},
    {">=", ProgramTokenType::GTE},
    {"<", ProgramTokenType::LT},
    {"<=", ProgramTokenType::LTE},
    {"!", ProgramTokenType::NOT},
    {"||", ProgramTokenType::OR},
    {"&&", ProgramTokenType::AND},
    {"|", ProgramTokenType::B_OR},
    {"&", ProgramTokenType::B_AND},
    {"^", ProgramTokenType::B_XOR},
    {">>", ProgramTokenType::B_RSHIFT},
    {"<<", ProgramTokenType::B_LSHIFT},
    {"|=", ProgramTokenType::B_OR_EQ},
    {"&=", ProgramTokenType::B_AND_EQ},
    {"^=", ProgramTokenType::B_XOR_EQ},
    {">>=", ProgramTokenType::B_RSHIFT_EQ},
    {"<<=", ProgramTokenType::B_LSHIFT_EQ},
    {"~", ProgramTokenType::COMPL},

    {"(", ProgramTokenType::OPEN_PAR},
    {")", ProgramTokenType::CLOSE_PAR},
    {"[", ProgramTokenType::OPEN_BRACKETS},
    {"]", ProgramTokenType::CLOSE_BRACKETS},
    {"{", ProgramTokenType::OPEN_BRACES},
    {"}", ProgramTokenType::CLOSE_BRACES},
    {".", ProgramTokenType::DOT},
    {",", ProgramTokenType::COMMA},
    {";", ProgramTokenType::SEMICOLON},
    {"->", ProgramTokenType::RET_TYPE},
    {":", ProgramTokenType::IND_TYPE},

    {"_LEXER__NUMBER__", ProgramTokenType::LEX_NUMBER},
    {"_LEXER__STRING__", ProgramTokenType::LEX_STRING},
    {"_LEXER__CHAR__", ProgramTokenType::LEX_CHAR},
    {"_LEXER__IDENTIFIER__", ProgramTokenType::IDENTIFIER},
};

std::unordered_map<std::string, int> binaryOpsPrecedence = {
    {"*", 1},   {"/", 1},   {"%", 1},

    {"+", 2},   {"-", 2},

    {"<<", 3},  {">>", 3},

    {">", 4},   {">=", 4},  {"<", 4},   {"<=", 4},

    {"!=", 5},  {"==", 5},

    {"&", 6},

    {"|", 7},

    {"&&", 8},

    {"||", 9},

    {"=", 10},  {"+=", 10}, {"-=", 10}, {"*=", 10},  {"%=", 10},
    {"^=", 10}, {"&=", 10}, {"|=", 10}, {"<<=", 10}, {">>=", 10},
};

std::unordered_set<std::string> unaryOps = {"+", "-", "!", "&", "*"};

std::unordered_map<std::string, std::string> assignTransformMap{
    {"=", ""},   {"+=", "+"}, {"-=", "-"}, {"*=", "*"},   {"/=", "/"},
    {"^=", "^"}, {"&=", "&"}, {"|=", "|"}, {"<<=", "<<"}, {">>=", ">>"},
};

AstParser::AstParser(Lexer *lexer) {
  this->lexer = lexer;
  auto typeMapperRef = &typeMapper;
  lexer->setTypeMapper(typeMapperRef);
}

/*
  PROGRAM: (FUNCTION | VAR_DEF)*
*/
ProgramNode *AstParser::parseProgram() {
  auto node = new ProgramNode;

  while (lexer->get().rawType != TokenType::END_OF_INPUT) {
    auto current = lexer->look();

    if (current.mappedType == FUNC)
      parseFunction(node);
    else if (current.mappedType == VAR)
      parseVarDef(node);
    else if (current.mappedType == STRUCT)
      parseStruct(node);
    else
      sintax_error("Unexpected token : " + current.raw);
  }

  return node;
}

/*
  FUNCTION: IDENTIFIER OPEN_PAR VAR_DEF? [ COMMA VAR_DEF ]* CLOSE_PAR RET_TYPE
  TYPE BLOCK
*/
FunctionNode *AstParser::parseFunction(AstNode *parent) {
  Token ident = nextExpected(IDENTIFIER, "Expecting identifier");

  auto node = new FunctionNode(parent, ident.raw);

  nextExpected(OPEN_PAR, "Expecting (");

  if (lexer->get().mappedType != CLOSE_PAR) {
    lexer->unget();
    node->params.push_back(parseVarDef(node));

    while (lexer->get().mappedType == COMMA) {
      node->params.push_back(parseVarDef(node));
    }
  }
  lexer->unget();

  nextExpected(CLOSE_PAR, "Expecting )");
  nextExpected(RET_TYPE, "Expecting ->");

  node->retType = parseTypeDef(node);
  node->body = parseBlock(node);

  return node;
}

/*
  STRUCT: IDENTIFIER OPEN_BRACES [ VAR_DEF SEMICOLON ]* CLOSE_BRACES
*/
StructDefNode *AstParser::parseStruct(AstNode *parent) {
  Token token = nextExpected(IDENTIFIER, "Expecting identifier");
  auto node = new StructDefNode(parent, token.raw);
  nextExpected(OPEN_BRACES, "Expecting {");

  while ((token = lexer->get()).mappedType != CLOSE_BRACES) {
    lexer->unget();
    node->members.push_back(parseVarDef(node));
    nextExpected(SEMICOLON, "Expecting semicolon");
  }

  return node;
}

/*
  BODY: OPEN_BRACES STATEMENT*  CLOSE_BRACES
*/
BlockNode *AstParser::parseBlock(AstNode *parent) {
  nextExpected(OPEN_BRACES, "Expecting {");

  auto node = new BlockNode(parent);

  while (lexer->get().mappedType != CLOSE_BRACES) {
    if (lexer->look().rawType == TokenType::END_OF_INPUT)
      sintax_error("Unexpected EOF reading block");

    lexer->unget();
    node->statements.push_back(parseStatement(node));
  }

  return node;
}

/*
  STATEMENT: IF | FOR | WHILE | VAR_DEF | BREAK | RETURN | EXPR
*/
AstNode *AstParser::parseStatement(AstNode *parent) {
  auto token = lexer->get();

  switch (token.mappedType) {
  case IF:
    lexer->unget();
    return parseIf(parent);
  case FOR:
    lexer->unget();
    return parseFor(parent);
  case WHILE:
    lexer->unget();
    return parseWhile(parent);
  case VAR: {
    auto node = parseVarDef(parent);
    nextExpected(SEMICOLON, "Expecting semicolon after variable definition");
    return node;
  }
  case BREAK: {
    auto node = new BreakNode(parent);
    nextExpected(SEMICOLON, "Expecting semicolon after break");
    return node;
  }
  case RETURN: {
    auto node = new ReturnNode(parent);
    node->expr = parseExpr(node);
    nextExpected(SEMICOLON, "Expecting semicolon after return");
    return node;
  }
  default: {
    lexer->unget();
    auto node = parseExpr(parent);
    nextExpected(SEMICOLON, "Expecting semicolon after expression");
    return node;
  }
  }
}

/*
  IF: IF EPXR BLOCK [ELSE BLOCK]?
*/
IfNode *AstParser::parseIf(AstNode *parent) {
  nextExpected(IF, "Expecting IF");

  auto node = new IfNode(parent);

  node->expr = parseExpr(node);
  node->ifBody = parseBlock(node);

  auto token = lexer->get();
  if (token.mappedType == ELSE)
    node->elseBody = parseBlock(node);
  else
    lexer->unget();

  return node;
}

/*
  FOR: FOR OPEN_PAR EXPR SEMICOLON EXPR SEMICOLON EXPR CLOSE_PAR BLOCK
*/
ForNode *AstParser::parseFor(AstNode *parent) {
  nextExpected(FOR, "Expecting FOR");

  auto node = new ForNode(parent);

  nextExpected(OPEN_PAR, "Expecting (");

  node->start = parseExpr(node);
  nextExpected(SEMICOLON, "Expecting ;");
  node->cond = parseExpr(node);
  nextExpected(SEMICOLON, "Expecting ;");
  node->inc = parseExpr(node);

  nextExpected(CLOSE_PAR, "Expecting )");

  node->body = parseBlock(node);

  return node;
}

/*
  WHILE: WHILE OPEN_PAR EXPR CLOSE_PAR BLOCK
*/
WhileNode *AstParser::parseWhile(AstNode *parent) {
  nextExpected(WHILE, "Expecting WHILE");
  auto node = new WhileNode(parent);

  nextExpected(OPEN_PAR, "Expecting (");
  node->expr = parseExpr(node);
  nextExpected(CLOSE_PAR, "Expecting )");
  node->body = parseBlock(node);

  return node;
}

/*
  VAR_DEF: IDENTIFIER TYPE TYPE_DEF [ ASSIGN EXPR]?
*/
VarDefNode *AstParser::parseVarDef(AstNode *parent) {
  auto token = nextExpected(IDENTIFIER, "Expecting identifier");

  auto node = new VarDefNode(parent, token.raw);
  nextExpected(IND_TYPE, "Expecting type indicator");
  node->type = parseTypeDef(node);

  token = lexer->get();
  if (token.mappedType == ASSIGN) {
    node->defaultVal = parseExpr(node);
  } else
    lexer->unget();

  return node;
}

/*
  TYPE_DEF: IDENTIFIER [ MULT ]* [ OPEN_BRACKETS LEX_NUMBER CLOSE_BRACKETS ]
*/
TypeDefNode *AstParser::parseTypeDef(AstNode *parent) {
  auto token = nextExpected(IDENTIFIER, "Expecting identifier");

  auto node = TypeDefNode::build(token.raw);

  while ((token = lexer->get()).mappedType == MULT) {
    node = TypeDefNode::buildPointer(node);
  }
  if (token.mappedType == OPEN_BRACKETS) {
    token = nextExpected(LEX_NUMBER, "Expecting array size");
    node = TypeDefNode::buildArray(node, std::stoi(token.raw));
    nextExpected(CLOSE_BRACKETS, "Expecting ]");
  } else
    lexer->unget();

  return node;
}

/*
  EXPR: EXPR OP EXPR | ATOM
*/
ExprNode *AstParser::parseExpr(AstNode *parent) {
  if (lexer->get().mappedType == SEMICOLON) {
    lexer->unget();
    return nullptr;
  }
  lexer->unget();

  std::stack<AstNode *> operands;
  std::stack<std::string> operators;

  auto node = new ExprNode(parent);

  operands.push(parseAtom(node));

  auto clearStack = [&](int level = 0xFF) {
    while (!operators.empty() && binaryOpsPrecedence[operators.top()] < level) {
      std::string op = operators.top();
      operators.pop();
      auto right = operands.top();
      operands.pop();
      auto left = operands.top();
      operands.pop();

      operands.push(new ExprBinaryNode(left, op, right, node));
    }
  };

  Token token;
  while (true) {
    token = lexer->get();
    auto precedenceIt = binaryOpsPrecedence.find(token.raw);

    if (precedenceIt == binaryOpsPrecedence.end()) {
      lexer->unget();
      clearStack();
      break;
    }

    int level = precedenceIt->second;

    clearStack(level);
    operators.push(token.raw);
    operands.push(parseAtom(node));
  }

  operands.top()->parent = node;
  node->children.push_back(operands.top());
  return node;
}

/*
  ATOM: OPEN_PAR EXPR CLOSE_PAR | REFERENCE | CONSTANT | OP EXPR
  CONSTANT: LEX_NUMBER | LEX_STRING | LEX_CHAR
*/
AstNode *AstParser::parseAtom(AstNode *parent) {
  Token token = lexer->get();

  switch (token.mappedType) {
  case LEX_CHAR:
  case LEX_STRING:
  case LEX_NUMBER:
    return new ExprConstantNode(token.raw, token.mappedType, parent);
  case OPEN_PAR: {
    auto node = parseExpr(parent);
    nextExpected(CLOSE_PAR, "Expecting )");
    return node;
  }
  case IDENTIFIER:
    lexer->unget();
    return parseIdentifier(parent);
  default: {
    if (unaryOps.find(token.raw) == unaryOps.end())
      sintax_error("Unexpected value reading expression :" + token.raw);

    auto atom = parseAtom(parent);
    return new ExprUnaryNode(token.raw, atom, parent);
  }
  }
}

/*
  REFERENCE: BASE_REFERENCE [ OPEN_PAR [ EXPR ]? [ COMMA EXPR ]* CLOSE_PAR ] |
  BASE_REFRENCE OPEN_BRACKETS EXPR CLOSE_BRACKETS BASE_REFERENCE: IDENTIFIER [
  DOT IDENTIFIER ]*
*/
AstNode *AstParser::parseIdentifier(AstNode *parent) {
  auto token = nextExpected(IDENTIFIER, "Expecting identifier");

  AstNode *finalNode;

  ExprVarRefNode *node;
  ExprCallNode *callNode;

  node = new ExprVarRefNode(nullptr, token.raw);
  while ((token = lexer->get()).mappedType == DOT) {
    token = nextExpected(IDENTIFIER, "Expecting identifier");
    node = new ExprVarRefNode(nullptr, token.raw, node);
  }
  finalNode = node;

  if (token.mappedType == OPEN_PAR) {
    callNode = new ExprCallNode(nullptr, node);

    if (lexer->get().mappedType != CLOSE_PAR) {
      lexer->unget();
      callNode->params.push_back(parseExpr(node));
    }
    while (lexer->get().mappedType == COMMA)
      callNode->params.push_back(parseExpr(node));
    lexer->unget();

    nextExpected(CLOSE_PAR, "Expecting )");

    finalNode = callNode;
  } else if (token.mappedType == OPEN_BRACKETS) {
    auto indexExpr = parseExpr();
    node->arrIndexed = true;
    node->arrIndex = indexExpr;
    nextExpected(CLOSE_BRACKETS, "Expecting ]");
  } else
    lexer->unget();

  finalNode->parent = parent;
  parent->children.push_back(finalNode);

  return finalNode;
}

const Token &AstParser::nextExpected(ProgramTokenType expectedType,
                                     std::string errorMsg) {
  auto token = lexer->get();
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