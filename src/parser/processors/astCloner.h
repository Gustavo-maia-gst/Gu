#ifndef _astCloner
#define _astCloner

#include "../../ast/ast.h"

class AstCloner : public BaseVisitor {
public:
  void visitProgram(ProgramNode *node) override;

  void visitFunction(FunctionNode *node) override;
  void visitStructDef(StructDefNode *node) override;
  void visitBody(BodyNode *node) override;
  void visitIf(IfNode *node) override;
  void visitWhile(WhileNode *node) override;
  void visitFor(ForNode *node) override;
  void visitVarDef(VarDefNode *node) override;
  void visitTypeDefNode(TypeDefNode *node) override;
  void visitBreakNode(BreakNode *node) override;
  void visitReturnNode(ReturnNode *node) override;

  void visitExprBinaryOp(ExprBinaryNode *node) override;
  void visitMemberAccess(ExprMemberAccess *node) override;
  void visitIndexAccess(ExprIndex *node) override;
  void visitExprCall(ExprCallNode *node) override;
  void visitExprUnaryOp(ExprUnaryNode *node) override;

  void visitExprVarRef(ExprVarRefNode *node) override;

  void setUpdateType(std::string original, TypeDefNode *newType);
  void clearUpdates();
  void setPrefix(std::string);
  void clearPrefix();
  AstNode *getCloned() { return cloned; };

private:
  AstNode *cloned = nullptr;
  std::string prefix = "";
  std::map<std::string, TypeDefNode *> realTypeMapper;
  std::map<AstNode *, AstNode *> cloneCache;
};

#endif