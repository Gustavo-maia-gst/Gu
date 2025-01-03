#include "ast.h"
#include <cerrno>
#include <map>
#include <string>

void AstNode::visitChildren(BaseVisitor *visitor) {
  for (auto child : this->_children)
    child->visit(visitor);
}

bool AstNode::isLoopNode() {
  auto nodeType = getNodeType();
  return nodeType == NodeType::WHILE || nodeType == NodeType::FOR;
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
  case NodeType::VAR_DEF:
    visitor->visitVarDef((VarDefNode *)this);
    break;
  case NodeType::VAR_REF:
    visitor->visitExprVarRef((ExprVarRefNode *)this);
    break;
  case NodeType::TYPE_DEF:
    visitor->visitTypeDefNode((TypeDefNode *)this);
    break;
  case NodeType::EXPR_BINARY:
    visitor->visitExprBinaryOp((ExprBinaryNode *)this);
    break;
  case NodeType::INDEX_ACCESS:
    visitor->visitIndexAccess((ExprIndex *)this);
    break;
  case NodeType::MEMBER_ACCESS:
    visitor->visitMemberAccess((ExprMemberAccess *)this);
    break;
  case NodeType::EXPR_UNARY:
    visitor->visitExprUnaryOp((ExprUnaryNode *)this);
    break;
  case NodeType::EXPR_CONSTANT:
    visitor->visitExprConstant((ExprConstantNode *)this);
    break;
  }
}

inline bool DataType::isNumeric(RawDataType &raw) {
  return raw == RawDataType::CHAR || raw == RawDataType::SHORT ||
         raw == RawDataType::INT || raw == RawDataType::LONG ||
         raw == RawDataType::FLOAT || raw == RawDataType::DOUBLE;
}

inline bool DataType::isInt(RawDataType &raw) {
  return raw == RawDataType::CHAR || raw == RawDataType::SHORT ||
         raw == RawDataType::INT || raw == RawDataType::LONG;
}

bool DataType::isAddress(RawDataType &raw) {
  return raw == RawDataType::POINTER || raw == RawDataType::ARRAY;
}

static std::map<RawDataType, DataType *> rawTypesMap;
static std::map<RawDataType, int> sizesMap{
    {RawDataType::CHAR, 1},    {RawDataType::SHORT, 2},
    {RawDataType::INT, 4},     {RawDataType::FLOAT, 4},
    {RawDataType::LONG, 8},    {RawDataType::DOUBLE, 8},
    {RawDataType::POINTER, 8}, {RawDataType::ARRAY, 8},
    {RawDataType::STRUCT, 8},
};

DataType::DataType() { inner = nullptr; }

DataType *DataType::build(RawDataType raw) {
  if (rawTypesMap.find(raw) != rawTypesMap.end())
    return rawTypesMap[raw];
  auto datatype = new DataType();
  datatype->raw = raw;
  rawTypesMap[raw] = datatype;
  rawTypesMap[raw]->size = sizesMap[raw];
  return rawTypesMap[raw];
}

DataType *DataType::buildPointer(DataType *type) {
  if (type->raw == RawDataType::ERROR)
    return DataType::build(RawDataType::ERROR);

  auto datatype = build(RawDataType::POINTER);
  datatype->inner = type;
  datatype->size = 8;
  return datatype;
}

DataType *DataType::build(TypeDefNode *node) {
  if (node->_pointsTo) {
    auto datatype = build(RawDataType::POINTER);
    datatype->inner = build(node->_pointsTo);
    return datatype;
  }

  if (node->_arrayOf) {
    auto datatype = build(RawDataType::ARRAY);
    datatype->inner = build(node->_arrayOf);
    datatype->arrLength = node->_arrSize;
    return datatype;
  }

  DataType *datatype;

  if (node->_rawIdent == "char") {
    datatype = build(RawDataType::CHAR);
  } else if (node->_rawIdent == "short") {
    datatype = build(RawDataType::SHORT);
  } else if (node->_rawIdent == "int") {
    datatype = build(RawDataType::INT);
  } else if (node->_rawIdent == "long") {
    datatype = build(RawDataType::LONG);
  } else if (node->_rawIdent == "float") {
    datatype = build(RawDataType::FLOAT);
  } else if (node->_rawIdent == "double") {
    datatype = build(RawDataType::DOUBLE);
  } else if (node->_rawIdent == "void") {
    datatype = build(RawDataType::VOID);
  } else {
    datatype = build(RawDataType::STRUCT);
  }

  datatype->ident = node->_rawIdent;
  return datatype;
}

DataType *DataType::fromNumber(std::string &num) {
  bool isDouble = false;

  for (char c : num)
    if (c == '.')
      isDouble = true;

  auto dataType = build(isDouble ? RawDataType::DOUBLE : RawDataType::LONG);
  dataType->ident = num;
  dataType->size = 8;

  return dataType;
}

DataType *DataType::fromString(std::string &str) {
  auto dataType = build(RawDataType::ARRAY);
  dataType->inner = build(RawDataType::CHAR);
  dataType->arrLength = str.size();
  dataType->ident = str;
  dataType->size = str.size();
  return dataType;
}

DataType *DataType::fromChar(std::string &ch) {
  int c = ch[0];
  auto dataType = build(RawDataType::CHAR);
  dataType->ident = std::to_string(c);
  dataType->size = 1;
  return dataType;
}

DataType *DataType::getResultType(DataType *left, std::string op,
                                  DataType *right) {
  if (!left || !right)
    return DataType::build(RawDataType::ERROR);

  if (op == "=") {
    if (left == right)
      return left;
    if (left->raw == RawDataType::POINTER && DataType::isAddress(right->raw)) {
      if (left->inner->raw == RawDataType::CHAR)
        return left;
      if (getResultType(left->inner, op, right->inner) == left->inner)
        return left;
      return DataType::build(RawDataType::ERROR);
    }
    if (left->raw == RawDataType::ARRAY && right->raw == RawDataType::ARRAY) {
      if (left == right)
        return left;
    }
  }

  if (left->raw == RawDataType::ERROR || left->raw == RawDataType::ARRAY ||
      left->raw == RawDataType::STRUCT)
    return DataType::build(RawDataType::ERROR);
  if (right->raw == RawDataType::ERROR || right->raw == RawDataType::ARRAY ||
      right->raw == RawDataType::STRUCT)
    return DataType::build(RawDataType::ERROR);

  if (isNumeric(left->raw) && isNumeric(right->raw)) {
    if (op == "=")
      return left;

    if (left->raw == RawDataType::DOUBLE)
      return left;
    if (left->raw == RawDataType::DOUBLE)
      return right;
    if (left->raw == RawDataType::FLOAT)
      return left;
    if (left->raw == RawDataType::FLOAT)
      return right;
    if (left->raw == RawDataType::LONG)
      return left;
    if (right->raw == RawDataType::LONG)
      return right;
    if (left->raw == RawDataType::INT)
      return left;
    if (right->raw == RawDataType::INT)
      return right;
    if (left->raw == RawDataType::SHORT)
      return left;
    if (right->raw == RawDataType::SHORT)
      return right;
    return left;
  }

  if (left->raw == RawDataType::POINTER && isInt(right->raw)) {
    if (op == "+")
      return left;
    return DataType::build(RawDataType::ERROR);
  }
  if (right->raw == RawDataType::POINTER && isInt(left->raw)) {
    if (op == "+")
      return right;
    return DataType::build(RawDataType::ERROR);
  }

  if (left->raw == RawDataType::POINTER && right->raw == RawDataType::POINTER) {
    if (op == "-")
      build(RawDataType::LONG);
    return DataType::build(RawDataType::ERROR);
  }

  return DataType::build(RawDataType::ERROR);
}