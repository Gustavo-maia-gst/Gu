#ifndef _parser
#define _parser
#include "../lexer/lexer.h"
#include "ast/ast.h"

enum ProgramTokenType {
  UNMAPPED,

  FUNC,
  VAR,
  CONST,
  IF,
  ELSE,
  WHILE,
  FOR,
  RETURN,
  BREAK,

  IDENTIFIER,
  ENUM,
  STRUCT,

  LEX_NUMBER,
  LEX_STRING,
  LEX_CHAR,

  ASSIGN,
  PLUS,
  MINUS,
  MULT,
  DIV,
  MOD,
  PLUS_EQ,
  MINUS_EQ,
  MULT_EQ,
  DIV_EQ,
  MOD_EQ,
  EQ,
  NEQ,
  GT,
  GTE,
  LT,
  LTE,
  NOT,
  OR,
  AND,
  B_OR,
  B_AND,
  B_XOR,
  B_LSHIFT,
  B_RSHIFT,
  B_OR_EQ,
  B_AND_EQ,
  B_XOR_EQ,
  B_LSHIFT_EQ,
  B_RSHIFT_EQ,
  COMPL,

  OPEN_PAR,
  CLOSE_PAR,
  OPEN_BRACKETS,
  CLOSE_BRACKETS,
  OPEN_BRACES,
  CLOSE_BRACES,
  COMMA,
  DOT,
  SEMICOLON,
  IND_TYPE,
  RET_TYPE,
};

class AstParser {
public:
  AstParser(Lexer *lexer);

  ProgramNode *parseProgram();
  ExprNode *parseExpr(AstNode *parent = nullptr);

private:
  Lexer *lexer;

  FunctionNode *parseFunction(AstNode *parent);
  StructDefNode *parseStruct(AstNode *parent);
  BlockNode *parseBlock(AstNode *parent);
  AstNode *parseStatement(AstNode *parent);
  IfNode *parseIf(AstNode *parent);
  WhileNode *parseWhile(AstNode *parent);
  ForNode *parseFor(AstNode *parent);
  VarDefNode *parseVarDef(AstNode *parent, bool constant = false);
  TypeDefNode *parseTypeDef(AstNode *parent);

  AstNode *parseAtom(AstNode *parent);
  AstNode *parseIdentifier(AstNode *parent);

  const Token &nextExpected(ProgramTokenType expectedType,
                            std::string errorMsg);
  void sintax_error(std::string msg);

  int currLine() { return this->lexer->look().line; }
  int currCol() { return this->lexer->look().start; }
};

#endif