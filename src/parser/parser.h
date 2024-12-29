#ifndef _parser
#define _parser
#include "../lexer/lexer.h"
#include "ast/ast.h"

enum ProgramTokenType {
  UNMAPPED,

  FUNC,
  VAR,
  IF,
  WHILE,
  FOR,
  RETURN,
  BREAK,

  ENUM,
  STRUCT,
  NUMBER,
  STRING,
  CHAR,
  IDENTIFIER,

  OPEN_PAR,
  CLOSE_PAR,
  OPEN_BRACKETS,
  CLOSE_BRACKETS,
  OPEN_BRACES,
  CLOSE_BRACES,
  COMMA,
  DOT,
  SEMICOLON,
};

class AstParser {
public:
  AstParser(Lexer *lexer);

  ProgramNode *parseProgram();
  ExprNode *parseExpr();

private:
  Lexer *lexer;
  ProgramNode *program;

  FunctionNode *parseFunction(AstNode *parent);
  BodyNode *parseBody(AstNode *parent);
  StatementNode *parseStatement(AstNode *parent);
  IfNode *parseIf(AstNode *parent);
  WhileNode *parseWhile(AstNode *parent);
  ForNode *parseFor(AstNode *parent);
  ExprCallNode *parseFuncCall(AstNode *parent);
  ExprAssignNode *parseVarAssign(AstNode *parent);
  VarDefNode *parseVarDef(AstNode *parent);
  ExprVarRefNode *parseVarRef(AstNode *parent);
  TypeDefNode *parseTypeDef(AstNode *parent);
  ExprNode *parseAtom(AstNode *parent);

  const Token &nextExpected(ProgramTokenType expectedType, std::string errorMsg);
  void sintax_error(std::string msg);
};

#endif