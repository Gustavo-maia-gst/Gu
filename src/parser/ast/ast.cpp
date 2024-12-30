#include "ast.h"

void AstNode::visitChildren(BaseVisitor *visitor) {
  for (auto child : this->_children)
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

const int ADDRESS_SIZE = 8;
DataType *ErrorType = new DataType(RawDataType::ERROR);

inline bool isNumeric(RawDataType &raw) {
  return raw == RawDataType::CHAR || raw == RawDataType::SHORT ||
         raw == RawDataType::INT || raw == RawDataType::LONG;
}

DataType::DataType(TypeDefNode *node) {
  if (node->_pointsTo) {
    this->type = RawDataType::POINTER;
    this->inner = new DataType(node->_pointsTo);
    this->size = ADDRESS_SIZE;
    return;
  }

  if (node->_arrayOf) {
    this->type = RawDataType::ARRAY;
    this->inner = new DataType(node->_arrayOf);
    this->arrLength = node->_arrSize;
    this->size = ADDRESS_SIZE;
    return;
  }

  if (node->_rawIdent == "char") {
    this->type = RawDataType::CHAR;
    this->size = 1;
  } else if (node->_rawIdent == "short") {
    this->type = RawDataType::SHORT;
    this->size = 2;
  } else if (node->_rawIdent == "int") {
    this->type = RawDataType::INT;
    this->size = 4;
  } else if (node->_rawIdent == "long") {
    this->type = RawDataType::LONG;
    this->size = ADDRESS_SIZE;
  } else {
    this->type = RawDataType::STRUCT;
    this->size = ADDRESS_SIZE;
  }

  this->ident = node->_rawIdent;
}

DataType *DataType::getResultType(DataType *left, std::string &op, DataType *right) {
  if (left->type == RawDataType::ERROR || left->type == RawDataType::ARRAY ||
      left->type == RawDataType::STRUCT)
    return ErrorType;
  if (right->type == RawDataType::ERROR || right->type == RawDataType::ARRAY ||
      right->type == RawDataType::STRUCT)
    return ErrorType;

  if (isNumeric(left->type) && isNumeric(right->type)) {
    if (left->type == RawDataType::LONG)
      return left;
    if (right->type == RawDataType::LONG)
      return right;
    if (left->type == RawDataType::INT)
      return left;
    if (right->type == RawDataType::INT)
      return right;
    if (left->type == RawDataType::SHORT)
      return left;
    if (right->type == RawDataType::SHORT)
      return right;
    return left;
  }

  if (left->type == RawDataType::POINTER && isNumeric(right->type)) {
    if (op == "+")
      return left;
    return ErrorType;
  }
  if (right->type == RawDataType::POINTER && isNumeric(left->type)) {
    if (op == "+")
      return right;
    return ErrorType;
  }

  if (left->type == RawDataType::POINTER &&
      right->type == RawDataType::POINTER) {
    if (op == "-")
      new DataType(RawDataType::LONG);
    return ErrorType;
  }

  return ErrorType;
}