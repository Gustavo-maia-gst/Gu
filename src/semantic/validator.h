#ifndef _validator
#define _validator

#include "../ast/ast.h"
#include "../parser/parser.h"
#include <iostream>
#include <map>
#include <ostream>
#include <stack>
#include <string>
#include <vector>

class SemanticValidator : public BaseVisitor {
public:
  SemanticValidator(bool validateMain);

  void visitProgram(ProgramNode *node) override;
  void visitFunction(FunctionNode *node) override;
  void visitStructDef(StructDefNode *node) override;
  void visitBody(BodyNode *node) override;
  void visitVarDef(VarDefNode *node) override;
  void visitTypeDefNode(TypeDefNode *node) override;

  void visitIf(IfNode *node) override;
  void visitWhile(WhileNode *node) override;
  void visitFor(ForNode *node) override;
  void visitBreakNode(BreakNode *node) override;
  void visitReturnNode(ReturnNode *node) override;

  void visitExprVarRef(ExprVarRefNode *node) override;
  void visitExprConstant(ExprConstantNode *node) override;
  void visitExprUnaryOp(ExprUnaryNode *node) override;
  void visitExprBinaryOp(ExprBinaryNode *node) override;
  void visitMemberAccess(ExprMemberAccess *node) override;
  void visitIndexAccess(ExprIndex *node) override;
  void visitExprCall(ExprCallNode *node) override;
  void visitSizeOfCall(ExprCallNode *node);

  const std::vector<std::string> getErrors() { return errors; };

private:
  std::vector<std::string> errors;
  bool validateMain = true;

  void validateArgs(FunctionNode *node,
                    std::vector<ExprNode *> &args);

  void unexpected_error(std::string msg, AstNode *node);
  void compile_error(std::string msg, AstNode *node);
  void type_error(std::string msg, AstNode *node);
  void name_error(std::string msg, AstNode *node);

  VarDefNode *resolveVar(std::string &varName);
  StructDefNode *resolveStruct(std::string &structName);
  FunctionNode *resolveFunction(std::string &funcName);

  ProgramNode *program;
  FunctionNode *function;
  bool outWithReturn;
};

#endif