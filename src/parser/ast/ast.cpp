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
  case NodeType::BODY:
    visitor->visitBody((BodyNode *)this);
    break;
  case NodeType::STATEMENT:
    visitor->visitStatement((StatementNode *)this);
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
  case NodeType::EXPR_PLUS_MINUS:
    visitor->visitExprPlusMinus((ExprPlusMinusNode *)this);
    break;
  case NodeType::EXPR_MULT_DIV:
    visitor->visitExprMultDiv((ExprMultDivNode *)this);
    break;
  case NodeType::EXPR_COMPARISON:
    visitor->visitExprComparison((ExprComparisonNode *)this);
    break;
  case NodeType::EXPR_BOOLEAN:
    visitor->visitExprBoolean((ExprBooleanNode *)this);
    break;
  case NodeType::EXPR_BIN:
    visitor->visitExprBin((ExprBinNode *)this);
    break;
  case NodeType::EXPR_CONSTANT:
    visitor->visitExprConstant((ExprConstantNode *)this);
    break;
  case NodeType::EXPR:
    visitor->visitExpr((ExprNode *)this);
    break;
  }
}