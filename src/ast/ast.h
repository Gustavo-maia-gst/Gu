#ifndef _ast
#define _ast
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

typedef long unsigned int ulint;

enum class NodeType {
  PROGRAM,
  FUNCTION,
  BODY,
  IF,
  WHILE,
  FOR,
  VAR_DEF,
  TYPE_DEF,
  STRUCT_DEF,
  RETURN,
  BREAK,

  EXPR_BINARY,
  EXPR_UNARY,
  EXPR_CONSTANT,

  INDEX_ACCESS,
  MEMBER_ACCESS,
  FUNCCALL,
  VAR_REF,
};

enum class RawDataType {
  ERROR,

  FUNCTION,
  STRUCT,
  POINTER,
  ARRAY,
  VOID,

  CHAR,
  SHORT,
  INT,
  LONG,

  FLOAT,
  DOUBLE,
};

class AstNode;

class ProgramNode;
class FunctionNode;
class BodyNode;
class IfNode;
class WhileNode;
class ForNode;
class TypeDefNode;
class VarDefNode;
class StructDefNode;
class BreakNode;
class ReturnNode;

class ExprNode;

class ExprBinaryNode;
class ExprIndex;
class ExprMemberAccess;
class ExprUnaryNode;

class ExprAssignNode;
class ExprVarRefNode;
class ExprCallNode;
class ExprConstantNode;

class BaseVisitor {
public:
  virtual void visitProgram(ProgramNode *node) {};
  virtual void visitFunction(FunctionNode *node) {};
  virtual void visitBody(BodyNode *node) {};
  virtual void visitIf(IfNode *node) {};
  virtual void visitWhile(WhileNode *node) {};
  virtual void visitFor(ForNode *node) {};
  virtual void visitVarDef(VarDefNode *node) {};
  virtual void visitStructDef(StructDefNode *node) {};
  virtual void visitTypeDefNode(TypeDefNode *node) {};
  virtual void visitBreakNode(BreakNode *node) {};
  virtual void visitReturnNode(ReturnNode *node) {};

  virtual void visitExprBinaryOp(ExprBinaryNode *node) {};
  virtual void visitMemberAccess(ExprMemberAccess *node) {};
  virtual void visitIndexAccess(ExprIndex *node) {};
  virtual void visitExprCall(ExprCallNode *node) {};
  virtual void visitExprUnaryOp(ExprUnaryNode *node) {};

  virtual void visitExprVarRef(ExprVarRefNode *node) {};
  virtual void visitExprConstant(ExprConstantNode *node) {};
};

class DataType {
public:
  DataType(TypeDefNode *node);
  ~DataType();

  RawDataType raw;
  DataType *inner;
  std::string ident;

  ulint size;
  ulint arrLength;

  static DataType *getResultType(DataType *left, std::string op,
                                 DataType *right);

  static bool isNumeric(RawDataType &raw);
  static bool isInt(RawDataType &raw);
  static bool isAddress(RawDataType &raw);
  static bool isFloat(RawDataType &raw);

  static DataType *build(RawDataType type);
  static DataType *build(TypeDefNode *node);
  static DataType *buildPointer(DataType *type);

  static DataType *fromNumber(std::string &num);
  static DataType *fromString(std::string &str);
  static DataType *fromChar(std::string &ch);

  bool equals(DataType *other);

private:
  DataType();
};

/* Attributes starting with _ are filled in parsing time */
class AstNode {
public:
  AstNode(NodeType nodeType, int line, int startCol,
          AstNode *parent = nullptr) {
    this->nodeType = nodeType;
    this->_line = line;
    this->_startCol = startCol;

    this->_parent = parent;
    if (parent)
      parent->_children.push_back(this);
  };

  void visit(BaseVisitor *visitor);
  void visitChildren(BaseVisitor *visitor);

  inline NodeType getNodeType() { return nodeType; };
  bool isLoopNode();
  bool isExpr();

  AstNode *_parent;
  std::vector<AstNode *> _children;

  int _line;
  int _startCol;

private:
  NodeType nodeType;
};

class ProgramNode : public AstNode {
public:
  std::map<std::string, FunctionNode *> funcs;
  std::map<std::string, VarDefNode *> globalVars;
  std::map<std::string, StructDefNode *> structDefs;

  ProgramNode(int line, int startCol)
      : AstNode(NodeType::PROGRAM, line, startCol, nullptr) {
    this->_children = _children;
  }
};

class FunctionNode : public AstNode {
public:
  std::string _name;
  std::vector<VarDefNode *> _params;
  std::vector<VarDefNode *> _innerVars;
  BodyNode *_body;
  TypeDefNode *_retTypeDef;

  std::map<std::string, VarDefNode *> localVars;
  DataType *retType = nullptr;
  bool external = false;

  FunctionNode(int line, int startCol, AstNode *parent, std::string ident)
      : AstNode(NodeType::FUNCTION, line, startCol, parent) {
    _name = ident;
    _body = nullptr;
    _retTypeDef = nullptr;
  }
};

class StructDefNode : public AstNode {
public:
  std::vector<AstNode *> _members;
  std::string _name;

  std::map<std::string, VarDefNode *> membersDef;
  std::map<std::string, FunctionNode *> funcMembers;
  std::map<std::string, int> membersOffset;
  int size;

  StructDefNode(int line, int startCol, AstNode *parent, std::string name)
      : AstNode(NodeType::STRUCT_DEF, line, startCol, parent) {
    this->_name = name;
  }
};

class BodyNode : public AstNode {
public:
  std::vector<AstNode *> _statements;
  BodyNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::BODY, line, startCol, parent) {}
};

class IfNode : public AstNode {
public:
  ExprNode *_expr;
  BodyNode *_ifBody;
  BodyNode *_elseBody;

  IfNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::IF, line, startCol, parent) {
    _expr = nullptr;
    _ifBody = nullptr;
    _elseBody = nullptr;
  }
};

class WhileNode : public AstNode {
public:
  ExprNode *_expr;
  BodyNode *_body;

  WhileNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::WHILE, line, startCol, parent) {
    _expr = nullptr;
    _body = nullptr;
  }
};

class ForNode : public AstNode {
public:
  ExprNode *_start;
  ExprNode *_cond;
  ExprNode *_inc;
  BodyNode *_body;

  ForNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::FOR, line, startCol, parent) {
    _start = nullptr;
    _cond = nullptr;
    _body = nullptr;
    _inc = nullptr;
  }
};

class VarDefNode : public AstNode {
public:
  std::string _name;
  TypeDefNode *_typeDef;
  ExprNode *_defaultVal;
  bool _constant;

  DataType *type = nullptr;

  VarDefNode(int line, int startCol, AstNode *parent, std::string name,
             bool constant = false)
      : AstNode(NodeType::VAR_DEF, line, startCol, parent) {
    this->_name = name;
    _constant = constant;
    _typeDef = nullptr;
    _defaultVal = nullptr;
  }
};

class BreakNode : public AstNode {
public:
  AstNode *target;

  BreakNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::BREAK, line, startCol, parent) {
    target = nullptr;
  }
};

class ReturnNode : public AstNode {
public:
  ExprNode *_expr;
  DataType *retType;

  ReturnNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::RETURN, line, startCol, parent) {
    _expr = nullptr;
  }
};

class TypeDefNode : public AstNode {
public:
  TypeDefNode *_pointsTo;
  TypeDefNode *_arrayOf;
  int _arrSize;
  std::string _rawIdent;
  DataType *dataType;

  static TypeDefNode *build(std::string type, int line, int startCol) {
    auto typeDef = new TypeDefNode(line, startCol);
    typeDef->_rawIdent = type;
    return typeDef;
  }
  static TypeDefNode *buildPointer(TypeDefNode *innerType, int line,
                                   int startCol) {
    auto typeDef = new TypeDefNode(line, startCol);
    typeDef->_pointsTo = innerType;
    innerType->_parent = typeDef;
    typeDef->_children.push_back(innerType);
    return typeDef;
  }
  static TypeDefNode *buildArray(TypeDefNode *innerType, int size, int line,
                                 int startCol) {
    auto typeDef = new TypeDefNode(line, startCol);
    typeDef->_arrayOf = innerType;
    typeDef->_arrSize = size;
    innerType->_parent = typeDef;
    typeDef->_children.push_back(innerType);
    return typeDef;
  }

private:
  TypeDefNode(int line, int startCol)
      : AstNode(NodeType::TYPE_DEF, line, startCol, nullptr) {
    _pointsTo = nullptr;
    _arrayOf = nullptr;
    dataType = nullptr;
  }
};

class ExprNode : public AstNode {
public:
  DataType *type;

  VarDefNode *var;
  FunctionNode *func;

  ExprNode(NodeType nodeType, int line, int startCol, AstNode *parent)
      : AstNode(nodeType, line, startCol, parent) {
    type = nullptr;
    var = nullptr;
    func = nullptr;
  }
};

class ExprVarRefNode : public ExprNode {
public:
  std::string _ident;

  ExprVarRefNode(int line, int startCol, AstNode *parent, std::string varName)
      : ExprNode(NodeType::VAR_REF, line, startCol, parent) {
    this->_ident = varName;
  }
};

class ExprIndex : public ExprNode {
public:
  ExprNode *_inner;
  ExprNode *_index;

  ExprIndex(int line, int startCol, AstNode *parent, ExprNode *inner,
            ExprNode *index)
      : ExprNode(NodeType::INDEX_ACCESS, line, startCol, parent) {
    if (!inner || !index) {
      std::cerr << "Was not possible to open the file\n";
      exit(1);
    }

    _inner = inner;
    inner->_parent = this;
    this->_children.push_back(inner);

    _index = index;
    index->_parent = this;
    this->_children.push_back(index);
  }
};

class ExprMemberAccess : public ExprNode {
public:
  ExprNode *_struct;
  std::string _memberName;

  StructDefNode *structDef;

  ExprMemberAccess(int line, int startCol, AstNode *parent, ExprNode *_struct,
                   std::string memberName)
      : ExprNode(NodeType::MEMBER_ACCESS, line, startCol, parent) {
    if (!_struct) {
      std::cerr << "struct cannot be null\n";
      exit(1);
    }

    this->_memberName = memberName;
    this->_struct = _struct;
    this->_struct->_parent = this;
    this->_children.push_back(_struct);
  }
};

class ExprCallNode : public ExprNode {
public:
  std::vector<ExprNode *> _args;
  ExprNode *_ref;

  FunctionNode *func;

  ExprCallNode(int line, int startCol, AstNode *parent, ExprNode *ref)
      : ExprNode(NodeType::FUNCCALL, line, startCol, parent) {
    this->_ref = ref;
    this->_children.push_back(ref);
    ref->_parent = this;
  }
};

class ExprBinaryNode : public ExprNode {
public:
  ExprNode *_left;
  std::string _op;
  int _opNum;
  ExprNode *_right;

  ExprBinaryNode(int line, int startCol, AstNode *parent, ExprNode *left,
                 std::string op, int opNum, ExprNode *right)
      : ExprNode(NodeType::EXPR_BINARY, line, startCol, parent) {
    this->_left = left;
    this->_op = op;
    this->_opNum = opNum;
    this->_right = right;

    left->_parent = this;
    this->_children.push_back(left);
    right->_parent = this;
    this->_children.push_back(right);
  }
};

class ExprUnaryNode : public ExprNode {
public:
  std::string _op;
  int _opNum;
  ExprNode *_expr;

  ExprUnaryNode(int line, int startCol, AstNode *parent, std::string op,
                int opNum, ExprNode *expr)
      : ExprNode(NodeType::EXPR_UNARY, line, startCol, parent) {
    this->_op = op;
    this->_opNum = opNum;
    this->_expr = expr;
    expr->_parent = this;
    this->_children.push_back(expr);
  }
};

class ExprConstantNode : public ExprNode {
public:
  std::string _rawValue;
  int _rawType;

  ExprConstantNode(int line, int startCol, AstNode *parent,
                   std::string rawValue, int rawType)
      : ExprNode(NodeType::EXPR_CONSTANT, line, startCol, parent) {
    this->_rawValue = rawValue;
    this->_rawType = rawType;
  }
};

#endif