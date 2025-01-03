#include "parser.h"
#include "ast/ast.h"

std::map<std::string, int> baseTypeMapper = {
    {"func", ProgramTokenType::FUNC},
    {"var", ProgramTokenType::VAR},
    {"const", ProgramTokenType::CONST},
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

std::map<std::string, int> binaryOpsPrecedence = {
    {".", 0},   {"[", 0},   {"(", 0},

    {"*", 1},   {"/", 1},   {"%", 1},

    {"+", 2},   {"-", 2},

    {"<<", 3},  {">>", 3},

    {">", 4},   {">=", 4},  {"<", 4},   {"<=", 4},

    {"!=", 5},  {"==", 5},

    {"&", 6},

    {"|", 7},

    {"&&", 8},

    {"||", 9},

    {"=", 10},  
};

std::unordered_set<std::string> unaryOps = {"+", "-", "!", "&", "*"};

std::map<std::string, std::string> assignTransformMap{
    {"=", ""},   {"+=", "+"}, {"-=", "-"}, {"*=", "*"},   {"/=", "/"},
    {"^=", "^"}, {"&=", "&"}, {"|=", "|"}, {"<<=", "<<"}, {">>=", ">>"},
};

AstParser::AstParser(Lexer *lexer) {
  this->lexer = lexer;
  auto typeMapperRef = &baseTypeMapper;
  lexer->setTypeMapper(typeMapperRef);
}

/*
  PROGRAM: (FUNCTION | VAR_DEF)*
*/
ProgramNode *AstParser::parseProgram() {
  auto node = new ProgramNode(currLine(), currCol());

  while (lexer->get().rawType != TokenType::END_OF_INPUT) {
    auto current = lexer->look();

    if (current.mappedType == FUNC)
      parseFunction(node);
    else if (current.mappedType == VAR || current.mappedType == CONST)
      parseVarDef(node, current.mappedType == CONST);
    else if (current.mappedType == STRUCT)
      parseStruct(node);
    else
      sintax_error("Unexpected token: " + current.raw);
  }

  delete lexer;

  return node;
}

/*
  FUNCTION: IDENTIFIER OPEN_PAR VAR_DEF? [ COMMA VAR_DEF ]* CLOSE_PAR RET_TYPE
  TYPE BLOCK
*/
FunctionNode *AstParser::parseFunction(AstNode *parent) {
  Token ident = nextExpected(IDENTIFIER, "Expecting identifier");

  auto node = new FunctionNode(currLine(), currCol(), parent, ident.raw);
  function = node;

  nextExpected(OPEN_PAR, "Expecting (");

  if (lexer->get().mappedType != CLOSE_PAR) {
    lexer->unget();
    node->_params.push_back(parseVarDef(node));

    while (lexer->get().mappedType == COMMA) {
      node->_params.push_back(parseVarDef(node));
    }
  }
  lexer->unget();

  nextExpected(CLOSE_PAR, "Expecting )");
  nextExpected(RET_TYPE, "Expecting ->");

  node->_retTypeDef = parseTypeDef(node);
  node->_body = parseBlock(node);

  return node;
}

/*
  STRUCT: IDENTIFIER OPEN_BRACES [ (VAR_DEF | FUNCTION) SEMICOLON ]*
  CLOSE_BRACES
*/
StructDefNode *AstParser::parseStruct(AstNode *parent) {
  Token token = nextExpected(IDENTIFIER, "Expecting identifier");
  auto node = new StructDefNode(currLine(), currCol(), parent, token.raw);
  nextExpected(OPEN_BRACES, "Expecting {");

  while ((token = lexer->get()).mappedType != CLOSE_BRACES) {
    if (lexer->look().mappedType == FUNC) {
      node->_members.push_back(parseFunction(node));
    } else {
      lexer->unget();
      node->_members.push_back(parseVarDef(node));
    }
    nextExpected(SEMICOLON, "Expecting semicolon");
  }

  return node;
}

/*
  BODY: OPEN_BRACES STATEMENT*  CLOSE_BRACES
*/
BlockNode *AstParser::parseBlock(AstNode *parent) {
  nextExpected(OPEN_BRACES, "Expecting {");

  auto node = new BlockNode(currLine(), currCol(), parent);

  while (lexer->get().mappedType != CLOSE_BRACES) {
    if (lexer->look().rawType == TokenType::END_OF_INPUT)
      sintax_error("Unexpected EOF reading block");

    lexer->unget();
    node->_statements.push_back(parseStatement(node));
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
  case CONST:
  case VAR: {
    auto block = (BlockNode *) parent;

    bool constant = token.mappedType == CONST;
    auto node = parseVarDef(parent, constant);
    function->_innerVars.push_back(node);

    while ((lexer->get()).mappedType != SEMICOLON) {
      lexer->unget();
      nextExpected(COMMA, "Expecting , or ;");
      block->_statements.push_back(parseVarDef(parent, constant));
    }

    return node;
  }
  case BREAK: {
    auto node = new BreakNode(currLine(), currCol(), parent);
    nextExpected(SEMICOLON, "Expecting semicolon after break");
    return node;
  }
  case RETURN: {
    auto node = new ReturnNode(currLine(), currCol(), parent);
    node->_expr = parseExpr(node);
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

  auto node = new IfNode(currLine(), currCol(), parent);

  node->_expr = parseExpr(node);
  node->_ifBody = parseBlock(node);

  auto token = lexer->get();
  if (token.mappedType == ELSE)
    node->_elseBody = parseBlock(node);
  else
    lexer->unget();

  return node;
}

/*
  FOR: FOR OPEN_PAR EXPR SEMICOLON EXPR SEMICOLON EXPR CLOSE_PAR BLOCK
*/
ForNode *AstParser::parseFor(AstNode *parent) {
  nextExpected(FOR, "Expecting FOR");

  auto node = new ForNode(currLine(), currCol(), parent);

  nextExpected(OPEN_PAR, "Expecting (");

  node->_start = parseExpr(node);
  nextExpected(SEMICOLON, "Expecting ;");
  node->_cond = parseExpr(node);
  nextExpected(SEMICOLON, "Expecting ;");
  node->_inc = parseExpr(node);

  nextExpected(CLOSE_PAR, "Expecting )");

  node->_body = parseBlock(node);

  return node;
}

/*
  WHILE: WHILE EXPR BLOCK
*/
WhileNode *AstParser::parseWhile(AstNode *parent) {
  nextExpected(WHILE, "Expecting WHILE");
  auto node = new WhileNode(currLine(), currCol(), parent);

  node->_expr = parseExpr(node);
  node->_body = parseBlock(node);

  return node;
}

/*
  VAR_DEF: IDENTIFIER TYPE TYPE_DEF [ ASSIGN EXPR]?
*/
VarDefNode *AstParser::parseVarDef(AstNode *parent, bool constant) {
  auto token = nextExpected(IDENTIFIER, "Expecting identifier");

  auto node =
      new VarDefNode(currLine(), currCol(), parent, token.raw, constant);
  nextExpected(IND_TYPE, "Expecting type indicator");
  node->_typeDef = parseTypeDef(node);

  token = lexer->get();
  if (token.mappedType == ASSIGN) {
    node->_defaultVal = parseExpr(node);
  } else
    lexer->unget();

  return node;
}

/*
  TYPE_DEF: IDENTIFIER [ OPEN_BRACKETS LEX_NUMBER CLOSE_BRACKETS ]* |
            MULT OPEN_PAR TYPE_DEF CLOSE_PAR
*/
TypeDefNode *AstParser::parseTypeDef(AstNode *parent) {
  auto token = lexer->get();

  TypeDefNode *node;

  if (token.mappedType == MULT) {
    token = lexer->get();

    bool matchParam = token.mappedType == OPEN_PAR;

    if (token.mappedType != OPEN_PAR)
      lexer->unget();

    node =
        TypeDefNode::buildPointer(parseTypeDef(parent), currLine(), currCol());
    node->_parent = parent;
    parent->_children.push_back(node);

    if (matchParam)
      nextExpected(CLOSE_PAR, "Expecting )");

    return node;
  }

  if (token.mappedType != IDENTIFIER)
    std::cerr << "sintax error: " << "Expecting identifer" << " at line "
              << std::to_string(token.line) << ':'
              << std::to_string(token.start) << std::endl;

  node = TypeDefNode::build(token.raw, currLine(), currCol());

  while ((token = lexer->get()).mappedType == OPEN_BRACKETS) {
    token = nextExpected(LEX_NUMBER, "Expecting array size");
    node = TypeDefNode::buildArray(node, std::stoi(token.raw), currLine(),
                                   currCol());
    nextExpected(CLOSE_BRACKETS, "Expecting ]");
  }
  lexer->unget();

  node->_parent = parent;
  parent->_children.push_back(node);

  return node;
}

/*
  EXPR: EXPR OP EXPR | ATOM | ATOM OPEN_PAR [EXPR]* CLOSE_PAR | ATOM
  OPEN_BRACKETS EXPR CLOSE_BRACKETS
*/
ExprNode *AstParser::parseExpr(AstNode *parent) {
  if (lexer->get().mappedType == SEMICOLON) {
    lexer->unget();
    return nullptr;
  }
  lexer->unget();

  std::stack<ExprNode *> operands;
  std::stack<Token> operators;

  operands.push(parseAtom(nullptr));

  auto clearStack = [&](std::stack<ExprNode *> &operands,
                        std::stack<Token> &operators, int level = 0xFF) {
    while (!operators.empty() &&
           binaryOpsPrecedence[operators.top().raw] < level) {
      Token op = operators.top();
      operators.pop();
      auto right = operands.top();
      operands.pop();
      auto left = operands.top();
      operands.pop();

      operands.push(new ExprBinaryNode(currLine(), currCol(), nullptr, left,
                                       op.raw, op.mappedType, right));
    }
  };

  auto processIndex = [&](std::stack<ExprNode *> &operands) {
    auto index = parseExpr();
    auto expr = operands.top();
    operands.pop();
    auto node = new ExprIndex(currLine(), currCol(), nullptr, expr, index);
    nextExpected(CLOSE_BRACKETS, "Expecting ]");
    operands.push(node);
  };

  auto processCall = [&](std::stack<ExprNode *> &operands) {
    auto funcRef = operands.top();
    operands.pop();
    auto node = new ExprCallNode(currLine(), currCol(), nullptr, funcRef);

    if ((lexer->get()).mappedType == CLOSE_PAR) {
      operands.push(node);
      return;
    }

    while ((lexer->get()).mappedType != CLOSE_PAR) {
      lexer->unget();
      nextExpected(COMMA, "Expecting ,");
      node->_args.push_back(parseExpr(node));
    }

    operands.push(node);
  };

  auto processMemberAccess = [&](std::stack<ExprNode *> &operands) {
    auto structRef = operands.top();
    operands.pop();
    auto ident = nextExpected(IDENTIFIER, "Expecting identifier");
    auto node = new ExprMemberAccess(currLine(), currCol(), nullptr, structRef,
                                     ident.raw);
    operands.push(node);
  };

  Token token;
  while (true) {
    token = lexer->get();
    auto precedenceIt = binaryOpsPrecedence.find(token.raw);

    if (precedenceIt == binaryOpsPrecedence.end()) {
      lexer->unget();
      clearStack(operands, operators);
      break;
    }

    int level = precedenceIt->second;
    clearStack(operands, operators, level);

    if (level > 0) {
      operators.push(token);
      operands.push(parseAtom(nullptr));
    } else {
      switch (token.raw[0]) {
      case '.':
        processMemberAccess(operands);
        break;
      case '[':
        processIndex(operands);
        break;
      case '(':
        processCall(operands);
        break;
      }
    }
  }

  auto node = operands.top();
  node->_parent = parent;
  if (parent)
    parent->_children.push_back(node);
  return node;
}

/*
  ATOM: OPEN_PAR EXPR CLOSE_PAR | IDENTIFIER | CONSTANT | OP EXPR
  CONSTANT: LEX_NUMBER | LEX_STRING | LEX_CHAR
*/
ExprNode *AstParser::parseAtom(AstNode *parent) {
  Token token = lexer->get();

  switch (token.mappedType) {
  case LEX_CHAR:
  case LEX_STRING:
  case LEX_NUMBER:
    return new ExprConstantNode(currLine(), currCol(), parent, token.raw,
                                token.mappedType);
  case OPEN_PAR: {
    auto node = parseExpr(parent);
    nextExpected(CLOSE_PAR, "Expecting )");
    return node;
  }
  case IDENTIFIER: {
    auto node = new ExprVarRefNode(currLine(), currCol(), nullptr, token.raw);

    node->_parent = parent;
    if (parent)
      parent->_children.push_back(node);

    return node;
  }
  default: {
    if (unaryOps.find(token.raw) == unaryOps.end())
      sintax_error("Unexpected value reading expression :" + token.raw);

    auto atom = parseAtom(parent);
    return new ExprUnaryNode(currLine(), currCol(), parent, token.raw,
                             token.mappedType, atom);
  }
  }
}

const Token &AstParser::nextExpected(ProgramTokenType expectedType,
                                     std::string errorMsg) {
  auto token = lexer->get();
  if (token.mappedType != expectedType)
    sintax_error(errorMsg + ", got " + token.raw);
  return lexer->look();
}

void AstParser::sintax_error(std::string msg) {
  auto current = lexer->look();
  std::cerr << "sintax error: " << msg << " at line "
            << std::to_string(current.line) << ':'
            << std::to_string(current.start) << std::endl;
  exit(1);
}