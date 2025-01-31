#ifndef _ast
#define _ast

#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <utility>
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

extern std::set<std::string> logicalOperators;

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
  static DataType *getOperationType(DataType *left, std::string op,
                                 DataType *right);

  static bool isComparable(DataType *left, DataType *right);

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
  AstNode(NodeType nodeType, std::string &filename, int line, int startCol,
          AstNode *parent = nullptr) {
    this->nodeType = nodeType;
    this->_line = line;
    this->_startCol = startCol;
    this->_filename = filename;

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
  std::string _filename;

private:
  NodeType nodeType;
};

const std::set<std::string> reservedFunctions = {"sizeof"};
const std::string MAIN_FUNC = "main";
const std::string INIT_FUNC = "init";

class ProgramNode : public AstNode {
public:
  std::map<std::string, FunctionNode *> funcs;
  std::map<std::string, VarDefNode *> globalVars;
  std::map<std::string, StructDefNode *> structDefs;
  std::vector<std::pair<std::string, bool>> imports;

  ProgramNode(std::string &filename, int line, int startCol)
      : AstNode(NodeType::PROGRAM, filename, line, startCol, nullptr) {
    this->_children = _children;
  }

  // Destructive operation deletes the other node and some attributes may be
  // loosen
  void merge(ProgramNode *other) {
    for (auto child : other->_children) {
      child->_parent = this;
      this->_children.push_back(child);
    }

    delete other;
  }
};

class FunctionNode : public AstNode {
public:
  std::string _name;
  std::string _externName = "";
  std::vector<VarDefNode *> _params;
  std::vector<VarDefNode *> _innerVars;
  BodyNode *_body;
  TypeDefNode *_retTypeDef;
  bool _export = false;
  bool _external = false;

  std::map<std::string, VarDefNode *> localVars;
  DataType *retType = nullptr;

  FunctionNode(std::string &filename, int line, int startCol, AstNode *parent,
               std::string ident)
      : AstNode(NodeType::FUNCTION, filename, line, startCol, parent) {
    _name = ident;
    _body = nullptr;
    _retTypeDef = nullptr;
  }
};

class StructDefNode : public AstNode {
public:
  std::vector<AstNode *> _members;
  std::vector<std::string> _genericArgNames;
  std::string _name;
  bool _export = false;
  bool _external = false;

  std::map<std::string, VarDefNode *> membersDef;
  std::map<std::string, FunctionNode *> funcMembers;
  std::map<std::string, int> membersOffset;
  int size;

  StructDefNode(std::string &filename, int line, int startCol, AstNode *parent,
                std::string name)
      : AstNode(NodeType::STRUCT_DEF, filename, line, startCol, parent) {
    this->_name = name;
  }
};

class BodyNode : public AstNode {
public:
  std::vector<AstNode *> _statements;
  BodyNode(std::string &filename, int line, int startCol, AstNode *parent)
      : AstNode(NodeType::BODY, filename, line, startCol, parent) {}
};

class IfNode : public AstNode {
public:
  ExprNode *_expr;
  BodyNode *_ifBody;
  BodyNode *_elseBody;

  IfNode(std::string &filename, int line, int startCol, AstNode *parent)
      : AstNode(NodeType::IF, filename, line, startCol, parent) {
    _expr = nullptr;
    _ifBody = nullptr;
    _elseBody = nullptr;
  }
};

class WhileNode : public AstNode {
public:
  ExprNode *_expr;
  BodyNode *_body;

  WhileNode(std::string &filename, int line, int startCol, AstNode *parent)
      : AstNode(NodeType::WHILE, filename, line, startCol, parent) {
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

  ForNode(std::string &filename, int line, int startCol, AstNode *parent)
      : AstNode(NodeType::FOR, filename, line, startCol, parent) {
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
  std::vector<ExprNode *> _initArgs;
  bool _constant;
  bool _export;
  bool _external;

  DataType *type = nullptr;

  VarDefNode(std::string &filename, int line, int startCol, AstNode *parent,
             std::string name, bool constant = false)
      : AstNode(NodeType::VAR_DEF, filename, line, startCol, parent) {
    this->_name = name;
    _constant = constant;
    _typeDef = nullptr;
    _defaultVal = nullptr;
  }
};

class BreakNode : public AstNode {
public:
  AstNode *target;

  BreakNode(std::string &filename, int line, int startCol, AstNode *parent)
      : AstNode(NodeType::BREAK, filename, line, startCol, parent) {
    target = nullptr;
  }
};

class ReturnNode : public AstNode {
public:
  ExprNode *_expr;
  DataType *retType;

  ReturnNode(std::string &filename, int line, int startCol, AstNode *parent)
      : AstNode(NodeType::RETURN, filename, line, startCol, parent) {
    _expr = nullptr;
  }
};

class TypeDefNode : public AstNode {
public:
  std::vector<TypeDefNode *> _genericArgsDefs;
  TypeDefNode *_pointsTo;
  TypeDefNode *_arrayOf;
  int _arrSize;
  std::string _rawIdent;
  DataType *dataType;

  TypeDefNode(std::string &filename, int line, int startCol)
      : AstNode(NodeType::TYPE_DEF, filename, line, startCol, nullptr) {
    _pointsTo = nullptr;
    _arrayOf = nullptr;
    dataType = nullptr;
  }

  static TypeDefNode *build(std::string type, std::string &filename, int line,
                            int startCol) {
    auto typeDef = new TypeDefNode(filename, line, startCol);
    typeDef->_rawIdent = type;
    return typeDef;
  }
  static TypeDefNode *buildPointer(TypeDefNode *innerType,
                                   std::string &filename, int line,
                                   int startCol) {
    auto typeDef = new TypeDefNode(filename, line, startCol);
    typeDef->_pointsTo = innerType;
    innerType->_parent = typeDef;
    typeDef->_children.push_back(innerType);
    return typeDef;
  }
  static TypeDefNode *buildArray(TypeDefNode *innerType, std::string &filename,
                                 int size, int line, int startCol) {
    auto typeDef = new TypeDefNode(filename, line, startCol);
    typeDef->_arrayOf = innerType;
    typeDef->_arrSize = size;
    innerType->_parent = typeDef;
    typeDef->_children.push_back(innerType);
    return typeDef;
  }
};

class ExprNode : public AstNode {
public:
  DataType *type;

  VarDefNode *var;
  FunctionNode *func;

  ExprNode(NodeType nodeType, std::string &filename, int line, int startCol,
           AstNode *parent)
      : AstNode(nodeType, filename, line, startCol, parent) {
    type = nullptr;
    var = nullptr;
    func = nullptr;
  }
};

class ExprVarRefNode : public ExprNode {
public:
  std::string _ident;

  ExprVarRefNode(std::string &filename, int line, int startCol, AstNode *parent,
                 std::string varName)
      : ExprNode(NodeType::VAR_REF, filename, line, startCol, parent) {
    this->_ident = varName;
  }
};

class ExprIndex : public ExprNode {
public:
  ExprNode *_inner;
  ExprNode *_index;

  ExprIndex(std::string &filename, int line, int startCol, AstNode *parent,
            ExprNode *inner, ExprNode *index)
      : ExprNode(NodeType::INDEX_ACCESS, filename, line, startCol, parent) {
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

  ExprMemberAccess(std::string &filename, int line, int startCol,
                   AstNode *parent, ExprNode *_struct, std::string memberName)
      : ExprNode(NodeType::MEMBER_ACCESS, filename, line, startCol, parent) {
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

  ExprCallNode(std::string &filename, int line, int startCol, AstNode *parent,
               ExprNode *ref)
      : ExprNode(NodeType::FUNCCALL, filename, line, startCol, parent) {
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

  ExprBinaryNode(std::string &filename, int line, int startCol, AstNode *parent,
                 ExprNode *left, std::string op, int opNum, ExprNode *right)
      : ExprNode(NodeType::EXPR_BINARY, filename, line, startCol, parent) {
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

  ExprUnaryNode(std::string &filename, int line, int startCol, AstNode *parent,
                std::string op, int opNum, ExprNode *expr)
      : ExprNode(NodeType::EXPR_UNARY, filename, line, startCol, parent) {
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

  ExprConstantNode(std::string &filename, int line, int startCol,
                   AstNode *parent, std::string rawValue, int rawType)
      : ExprNode(NodeType::EXPR_CONSTANT, filename, line, startCol, parent) {
    this->_rawValue = rawValue;
    this->_rawType = rawType;
  }
};

#endif