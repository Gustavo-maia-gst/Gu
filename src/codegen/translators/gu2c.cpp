#include "gu2c.h"
#include <string>

void Gu2CVisitor::visitProgram(ProgramNode *node) {
  node->visitChildren(this);
};

void Gu2CVisitor::visitFunction(FunctionNode *node) {
  node->_retTypeDef->visit(this);

  std::string funcName = "func" + std::to_string(funcCounter++);
  if (node->_name == "main")
    funcName = node->_name;

  writeText(" " + funcName + "(");

  if (!node->_params.empty()) {
    node->_params[0]->visit(this);
    for (int i = 1; i < node->_params.size(); i++) {
      writeText(", ");
      node->_params[i]->visit(this);
    }
  }

  writeText(")\n");

  node->_body->visit(this);

  writeText("\n\n");
};

void Gu2CVisitor::visitStructDef(StructDefNode *node) {
  writeText("struct " + node->_name + " {");
  for (auto [_, member] : node->membersDef) {
    writeText("\n\t");
    member->visit(this);
    writeText(";");
  }
  writeText("\n}\n\n");

  for (auto [_, func] : node->funcMembers)
    if (func)
        func->visit(this);
};

void Gu2CVisitor::visitBody(BlockNode *node) {
  int l = identLevel;
  std::string s0 = "";
  while (l--)
    s0 += "\t";
  std::string s1 = s0 + "\t";

  identLevel++;

  writeText(s0 + "{\n");
  for (auto statement : node->_statements) {
    writeText(s1);
    statement->visit(this);
    if (statement->getNodeType() == NodeType::VAR_DEF || statement->isExpr())
      writeText(";");
    writeText("\n");
  }
  writeText(s0 + "}");

  identLevel--;
};

void Gu2CVisitor::visitIf(IfNode *node) {
  writeText("if (");
  node->_expr->visit(this);
  writeText(")\n");
  node->_ifBody->visit(this);

  if (node->_elseBody) {
    writeText(" else ");
    node->_elseBody->visit(this);
  }
};

void Gu2CVisitor::visitWhile(WhileNode *node) {
  writeText("while (");
  node->_expr->visit(this);
  writeText(") ");
  node->_body->visit(this);
};

void Gu2CVisitor::visitFor(ForNode *node) {
  writeText("for (");
  node->_start->visit(this);
  writeText(";");
  node->_cond->visit(this);
  writeText(";");
  node->_inc->visit(this);
  writeText(") ");
  node->_body->visit(this);
};

void Gu2CVisitor::visitVarDef(VarDefNode *node) {
  node->_typeDef->visit(this);
  writeText(" " + node->_name);
  if (node->_defaultVal) {
    writeText(" = ");
    node->_defaultVal->visit(this);
  }
};

void Gu2CVisitor::visitTypeDefNode(TypeDefNode *node) {
  if (node->_pointsTo) {
    node->_pointsTo->visit(this);
    writeText("*");
  } else if (node->_arrayOf) {
    node->_arrayOf->visit(this);
    writeText("[" + std::to_string(node->_arrSize) + "]");
  } else {
    if (node->dataType->raw == RawDataType::STRUCT)
      writeText("struct ");
    writeText(node->_rawIdent);
  }
};

void Gu2CVisitor::visitBreakNode(BreakNode *node) { writeText("break;"); };

void Gu2CVisitor::visitReturnNode(ReturnNode *node) {
  if (node->_expr) {
    writeText("return ");
    node->_expr->visit(this);
    writeText(";");
  } else {
    writeText("return;");
  }
};

void Gu2CVisitor::visitExprBinaryOp(ExprBinaryNode *node) {
  writeText("(");
  node->_left->visit(this);
  writeText(")");
  writeText(node->_op);
  writeText("(");
  node->_right->visit(this);
  writeText(")");
};

void Gu2CVisitor::visitMemberAccess(ExprMemberAccess *node) {
  node->_struct->visit(this);
  writeText(".");
  writeText(node->_memberName);
};

void Gu2CVisitor::visitIndexAccess(ExprIndex *node) {
  node->_inner->visit(this);
  writeText("[");
  node->_index->visit(this);
  writeText("]");
};

void Gu2CVisitor::visitExprCall(ExprCallNode *node) {
  writeText(node->func->_name);
  writeText("(");
  if (!node->_args.empty()) {
    node->_args[0]->visit(this);
    for (int i = 1; i < node->_args.size(); i++) {
      writeText(", ");
      node->_args[i]->visit(this);
    }
  }
  writeText(")");
};

void Gu2CVisitor::visitExprUnaryOp(ExprUnaryNode *node) {
  writeText(node->_op);
  writeText("(");
  node->_expr->visit(this);
  writeText(")");
};

void Gu2CVisitor::visitExprVarRef(ExprVarRefNode *node) {
  writeText(node->_ident);
};

void Gu2CVisitor::visitExprConstant(ExprConstantNode *node) {
  writeText(node->_rawValue);
};

void Gu2CVisitor::writeText(std::string text) { std::cout << text; }