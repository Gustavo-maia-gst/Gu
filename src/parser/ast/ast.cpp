#include "ast.h"

void AstNode::visitChildren(BaseVisitor *visitor) {
  for (auto child : this->children)
    child->visit(visitor);
}

void AstNode::visit(BaseVisitor *visitor) {
  switch (this->nodeType) {
  case NodeType::PROGRAM:
    visitor->visitProgram((ProgramNode *)this);
    break;
  case NodeType::FUNCTION:
    visitor->visitFunction((FunctionNode *)this);
    break;
  case NodeType::STRUCT_DEF:
    visitor->visitStructDef((StructDefNode *)this);
    break;
  case NodeType::BODY:
    visitor->visitBody((BlockNode *)this);
    break;
  case NodeType::IF:
    visitor->visitIf((IfNode *)this);
    break;
  case NodeType::WHILE:
    visitor->visitWhile((WhileNode *)this);
    break;
  case NodeType::FOR:
    visitor->visitFor((ForNode *)this);
    break;
  case NodeType::FUNCCALL:
    visitor->visitExprCall((ExprCallNode *)this);
    break;
  case NodeType::BREAK:
    visitor->visitBreakNode((BreakNode *)this);
    break;
  case NodeType::RETURN:
    visitor->visitReturnNode((ReturnNode *)this);
    break;
  case NodeType::VAR_ASSIGN:
    visitor->visitExprAssign((ExprAssignNode *)this);
    break;
  case NodeType::VAR_DEF:
    visitor->visitVarDef((VarDefNode *)this);
    break;
  case NodeType::VAR_REF:
    visitor->visitExprVarRef((ExprVarRefNode *)this);
    break;
  case NodeType::TYPE_DEF:
    visitor->visitTypeDefNode((TypeDefNode *)this);
    break;
  case NodeType::EXPR:
    visitor->visitExpr((ExprNode *)this);
    break;
  case NodeType::EXPR_BINARY:
    visitor->visitExprBinaryOp((ExprBinaryNode *)this);
    break;
  case NodeType::EXPR_UNARY:
    visitor->visitExprUnaryOp((ExprUnaryNode *)this);
    break;
  case NodeType::EXPR_CONSTANT:
    visitor->visitExprConstant((ExprConstantNode *)this);
    break;
  }
}