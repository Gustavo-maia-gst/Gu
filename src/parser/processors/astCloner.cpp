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

  cloned = newNode;
}

void AstCloner::visitFor(ForNode *node) {
  auto newNode =
      new ForNode(node->_filename, node->_line, node->_startCol, nullptr);

  if (node->_start) {
    node->_start->visit(this);
    newNode->_start = (ExprNode *)cloned;
    newNode->_children.push_back(cloned);
    cloned->_parent = newNode;
  }
  if (node->_cond) {
    node->_cond->visit(this);
    newNode->_cond = (ExprNode *)cloned;
    newNode->_children.push_back(cloned);
    cloned->_parent = newNode;
  }
  if (node->_inc) {
    node->_inc->visit(this);
    newNode->_inc = (ExprNode *)cloned;
    newNode->_children.push_back(cloned);
    cloned->_parent = newNode;
  }

  node->_body->visit(this);
  newNode->_body = (BodyNode *)cloned;
  newNode->_children.push_back(cloned);
  cloned->_parent = newNode;

  cloned = newNode;
}

void AstCloner::visitVarDef(VarDefNode *node) {
  auto newNode = new VarDefNode(node->_filename, node->_line, node->_startCol,
                                nullptr, node->_name, node->_constant);
  newNode->_export = node->_export;
  newNode->_external = node->_external;

  if (node->_defaultVal) {
    node->_defaultVal->visit(this);
    newNode->_defaultVal = (ExprNode *)cloned;
    newNode->_children.push_back(cloned);
    cloned->_parent = newNode;
  }
  if (node->_typeDef) {
    node->_typeDef->visit(this);
    newNode->_typeDef = (TypeDefNode *)cloned;
    newNode->_children.push_back(cloned);
    cloned->_parent = newNode;
  }

  cloned = newNode;
}

void AstCloner::visitTypeDefNode(TypeDefNode *node) {
  if (realTypeMapper.find(node->_rawIdent) != realTypeMapper.end())
    return realTypeMapper[node->_rawIdent]->visit(this);

  auto newNode = new TypeDefNode(node->_filename, node->_line, node->_startCol);

  newNode->_rawIdent = node->_rawIdent;
  newNode->_arrSize = node->_arrSize;

  if (node->_arrayOf) {
    node->_arrayOf->visit(this);
    newNode->_arrayOf = (TypeDefNode *)cloned;
    newNode->_children.push_back(cloned);
    cloned->_parent = newNode;
  }
  if (node->_pointsTo) {
    node->_pointsTo->visit(this);
    newNode->_pointsTo = (TypeDefNode *)cloned;
    newNode->_children.push_back(cloned);
    cloned->_parent = newNode;
  }

  cloned = newNode;
}

void AstCloner::visitBreakNode(BreakNode *node) {
  auto newNode =
      new BreakNode(node->_filename, node->_line, node->_startCol, nullptr);

  cloned = newNode;
}

void AstCloner::visitReturnNode(ReturnNode *node) {
  auto newNode =
      new ReturnNode(node->_filename, node->_line, node->_startCol, nullptr);
  if (node->_expr) {
    node->_expr->visit(this);
    newNode->_children.push_back(cloned);
    cloned->_parent = newNode;
  }

  cloned = newNode;
}

void AstCloner::visitExprBinaryOp(ExprBinaryNode *node) {
  node->_left->visit(this);
  auto left = (ExprNode *)cloned;
  node->_right->visit(this);
  auto right = (ExprNode *)cloned;

  auto newNode =
      new ExprBinaryNode(node->_filename, node->_line, node->_startCol, nullptr,
                         left, node->_op, node->_opNum, right);

  newNode->_children.push_back(left);
  newNode->_children.push_back(right);
  left->_parent = newNode;
  right->_parent = newNode;

  cloned = newNode;
}

void AstCloner::visitMemberAccess(ExprMemberAccess *node) {
  node->_struct->visit(this);
  auto newNode =
      new ExprMemberAccess(node->_filename, node->_line, node->_startCol,
                           nullptr, (ExprNode *)cloned, node->_memberName);
  cloned = newNode;
}

void AstCloner::visitIndexAccess(ExprIndex *node) {
  node->_inner->visit(this);
  auto inner = (ExprNode *)cloned;
  node->_index->visit(this);
  auto index = (ExprNode *)cloned;

  auto newNode = new ExprIndex(node->_filename, node->_line, node->_startCol,
                               nullptr, inner, index);

  cloned = newNode;
}

void AstCloner::visitExprCall(ExprCallNode *node) {
  node->_ref->visit(this);
  auto ref = (ExprNode *)cloned;

  auto newNode = new ExprCallNode(node->_filename, node->_line, node->_startCol,
                                  nullptr, ref);

  for (auto arg : node->_args) {
    arg->visit(this);
    newNode->_args.push_back((ExprNode *)cloned);
    newNode->_children.push_back(cloned);
    cloned->_parent = newNode;
  }

  cloned = newNode;
}

void AstCloner::visitExprUnaryOp(ExprUnaryNode *node) {
  node->_expr->visit(this);

  auto newNode =
      new ExprUnaryNode(node->_filename, node->_line, node->_startCol, nullptr,
                        node->_op, node->_opNum, (ExprNode *)cloned);

  cloned = newNode;
}

void AstCloner::visitExprVarRef(ExprVarRefNode *node) {
  auto newNode = new ExprVarRefNode(node->_filename, node->_line,
                                    node->_startCol, nullptr, node->_ident);
  cloned = newNode;
}

void AstCloner::setUpdateType(std::string original, TypeDefNode *newType) {
  this->realTypeMapper[original] = newType;
}

void AstCloner::clearUpdates() {
  this->realTypeMapper.clear();
}

void AstCloner::setPrefix(std::string prefix) {
  this->prefix = prefix;
}

void AstCloner::clearPrefix() {
  this->prefix = "";
}