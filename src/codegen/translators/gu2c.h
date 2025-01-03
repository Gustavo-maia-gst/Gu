#include "../../parser/ast/ast.h"
#include <cstdio>

class Gu2CVisitor : public BaseVisitor {
public:
  virtual void visitProgram(ProgramNode *node);
  virtual void visitFunction(FunctionNode *node);
  virtual void visitBody(BlockNode *node);
  virtual void visitIf(IfNode *node);
  virtual void visitWhile(WhileNode *node);
  virtual void visitFor(ForNode *node);
  virtual void visitVarDef(VarDefNode *node);
  virtual void visitStructDef(StructDefNode *node);
  virtual void visitTypeDefNode(TypeDefNode *node);
  virtual void visitBreakNode(BreakNode *node);
  virtual void visitReturnNode(ReturnNode *node);

  virtual void visitExprBinaryOp(ExprBinaryNode *node);
  virtual void visitMemberAccess(ExprMemberAccess *node);
  virtual void visitIndexAccess(ExprIndex *node);
  virtual void visitExprCall(ExprCallNode *node);
  virtual void visitExprUnaryOp(ExprUnaryNode *node) ;

  virtual void visitExprVarRef(ExprVarRefNode *node);
  virtual void visitExprConstant(ExprConstantNode *node);

private:
    int identLevel = 0;
    int funcCounter = 0;
    std::map<FunctionNode *, std::string> funcNameMapper;

    void writeText(std::string text);
};