#include "astCloner.h"

void AstCloner::visitProgram(ProgramNode *node) {}

void AstCloner::visitStructDef(StructDefNode *node) {
  auto newNode =
      new StructDefNode(node->_filename, node->_line, node->_startCol, nullptr,
                        prefix + node->_name);
  for (auto member : node->_members) {
    member->visit(this);
    cloned->_parent = newNode;
    newNode->_children.push_back(cloned);
    newNode->_members.push_back(member);
  }
  newNode->_export = node->_export;

  cloned = newNode;
}

void AstCloner::visitFunction(FunctionNode *node) {
  auto newNode = new FunctionNode(node->_filename, node->_line, node->_startCol,
                                  nullptr, prefix + node->_name);

  for (auto param : node->_params) {
    param->visit(this);
    cloned->_parent = newNode;
    newNode->_children.push_back(cloned);
    newNode->_params.push_back((VarDefNode *)cloned);
  }
  for (auto innerVar : node->_innerVars) {
    innerVar->visit(this);
    cloned->_parent = newNode;
    newNode->_children.push_back(cloned);
    newNode->_innerVars.push_back((VarDefNode *)cloned);
  }
  node->_body->visit(this);
  newNode->_body = (BodyNode *)cloned;
  cloned->_parent = newNode;
  newNode->_children.push_back(cloned);
  node->_retTypeDef->visit(this);
  newNode->_retTypeDef = (TypeDefNode *)cloned;
  cloned->_parent = newNode;
  newNode->_children.push_back(cloned);

  cloned = newNode;
}

void AstCloner::visitBody(BodyNode *node) {
  auto newNode =
      new BodyNode(node->_filename, node->_line, node->_startCol, nullptr);
  for (auto statement : node->_statements) {
    statement->visit(this);
    cloned->_parent = newNode;
    newNode->_children.push_back(cloned);
  }

  cloned = newNode;
}

void AstCloner::visitIf(IfNode *node) {
  auto newNode =
      new IfNode(node->_filename, node->_line, node->_startCol, nullptr);

  if (node->_expr) {
    node->_expr->visit(this);
    newNode->_expr = (ExprNode *)cloned;
    cloned->_parent = newNode;
    newNode->_children.push_back(cloned);
  }
  node->_ifBody->visit(this);
  newNode->_ifBody = (BodyNode *)cloned;
  cloned->_parent = newNode;
  newNode->_children.push_back(cloned);
  if (node->_elseBody) {
    node->_elseBody->visit(this);
    newNode->_elseBody = (BodyNode *)cloned;
    cloned->_parent = newNode;
    newNode->_children.push_back(cloned);
  }

  cloned = newNode;
}

void AstCloner::visitWhile(WhileNode *node) {
  auto newNode =
      new WhileNode(node->_filename, node->_line, node->_startCol, nullptr);

  if (node->_expr) {
    node->_expr->visit(this);
    newNode->_expr = (ExprNode *)cloned;
    cloned->_parent = newNode;
    newNode->_children.push_back(cloned);
  }
  node->_body->visit(this);
  newNode->_body = (BodyNode *)cloned;
  cloned->_parent = newNode;
  newNode->_children.push_back(cloned);
}