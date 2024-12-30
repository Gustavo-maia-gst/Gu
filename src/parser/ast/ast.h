#ifndef _ast
#define _ast
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

enum class NodeType {
  PROGRAM,
  FUNCTION,
  BODY,
  IF,
  WHILE,
  FOR,
  VAR_ASSIGN,
  VAR_DEF,
  TYPE_DEF,
  STRUCT_DEF,
  RETURN,
  BREAK,

  EXPR,
  EXPR_BINARY,
  EXPR_UNARY,
  EXPR_CONSTANT,
  FUNCCALL,
  VAR_REF,
};
enum class RawDataType {
  ERROR,

  STRUCT,
  POINTER,
  ARRAY,

  CHAR,
  SHORT,
  INT,
  LONG,
};


class AstNode;

class ProgramNode;
class FunctionNode;
class BlockNode;
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
class ExprUnaryNode;

class ExprAssignNode;
class ExprVarRefNode;
class ExprCallNode;
class ExprConstantNode;

class BaseVisitor {
public:
  virtual void visitProgram(ProgramNode *node) {};
  virtual void visitFunction(FunctionNode *node) {};
  virtual void visitBody(BlockNode *node) {};
  virtual void visitIf(IfNode *node) {};
  virtual void visitWhile(WhileNode *node) {};
  virtual void visitFor(ForNode *node) {};
  virtual void visitVarDef(VarDefNode *node) {};
  virtual void visitStructDef(StructDefNode *node) {};
  virtual void visitTypeDefNode(TypeDefNode *node) {};
  virtual void visitBreakNode(BreakNode *node) {};
  virtual void visitReturnNode(ReturnNode *node) {};

  virtual void visitExpr(ExprNode *node) {};

  virtual void visitExprBinaryOp(ExprBinaryNode *node) {};
  virtual void visitExprUnaryOp(ExprUnaryNode *node) {};

  virtual void visitExprAssign(ExprAssignNode *node) {};
  virtual void visitExprVarRef(ExprVarRefNode *node) {};
  virtual void visitExprCall(ExprCallNode *node) {};
  virtual void visitExprConstant(ExprConstantNode *node) {};
};


struct DataType {
  DataType(RawDataType rawType);
  DataType(TypeDefNode *node);
  ~DataType();

  RawDataType type;
  DataType *inner;
  std::string ident;
  int size;
  int arrLength; // Filled for ARRAY RawDataType

  bool operator==(const DataType &other) const {
    return type == other.type && ident == other.ident && inner == other.inner;
  }

  static DataType *getResultType(DataType *left, std::string &op, DataType *right);
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

  NodeType getNodeType();
  AstNode *_parent;
  std::vector<AstNode *> _children;

  int _line;
  int _startCol;

private:
  NodeType nodeType;
};

class ProgramNode : public AstNode {
public:
  std::unordered_map<std::string, FunctionNode *> funcs;
  std::unordered_map<std::string, VarDefNode *> globalVars;
  std::unordered_map<std::string, StructDefNode *> structDefs;

  ProgramNode(int line, int startCol)
      : AstNode(NodeType::PROGRAM, line, startCol, nullptr) {
    this->_children = _children;
  }
};

class FunctionNode : public AstNode {
public:
  std::string _name;
  std::vector<VarDefNode *> _params;
  BlockNode *_body;
  TypeDefNode *_retType;
  std::unordered_map<std::string, VarDefNode *> localVars;

  FunctionNode(int line, int startCol, AstNode *parent, std::string ident)
      : AstNode(NodeType::FUNCTION, line, startCol, parent) {
    this->_name = ident;
  }
};

class StructDefNode : public AstNode {
public:
  std::vector<AstNode *> _members;
  std::string _name;

  std::unordered_map<std::string, VarDefNode *> membersDef;
  std::unordered_map<std::string, FunctionNode *> funcMembers;
  std::unordered_map<std::string, int> membersOffset;
  int size;

  StructDefNode(int line, int startCol, AstNode *parent, std::string name)
      : AstNode(NodeType::STRUCT_DEF, line, startCol, parent) {
    this->_name = name;
  }
};

class BlockNode : public AstNode {
public:
  std::vector<AstNode *> _statements;
  BlockNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::BODY, line, startCol, parent) {}
};

class IfNode : public AstNode {
public:
  ExprNode *_expr;
  BlockNode *_ifBody;
  BlockNode *_elseBody;

  IfNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::IF, line, startCol, parent) {}
};

class WhileNode : public AstNode {
public:
  ExprNode *_expr;
  BlockNode *_body;

  WhileNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::WHILE, line, startCol, parent) {}
};

class ForNode : public AstNode {
public:
  ExprNode *_start;
  ExprNode *_cond;
  ExprNode *_inc;
  BlockNode *_body;

  ForNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::FOR, line, startCol, parent) {}
};

class VarDefNode : public AstNode {
public:
  std::string _name;
  TypeDefNode *_type;
  ExprNode *_defaultVal;

  VarDefNode(int line, int startCol, AstNode *parent, std::string name,
             bool constant = false)
      : AstNode(NodeType::VAR_DEF, line, startCol, parent) {
    this->_name = name;
  }
};

class BreakNode : public AstNode {
public:
  BreakNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::BREAK, line, startCol, parent) {}
};

class ReturnNode : public AstNode {
public:
  ExprNode *expr;
  ReturnNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::RETURN, line, startCol, parent) {}
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
      : AstNode(NodeType::TYPE_DEF, line, startCol, nullptr) {}
};

class ExprCallNode : public AstNode {
public:
  std::vector<AstNode *> _args;
  ExprVarRefNode *_ref;

  ExprCallNode(int line, int startCol, AstNode *parent,
               ExprVarRefNode *ref = nullptr)
      : AstNode(NodeType::FUNCCALL, line, startCol, parent) {
    this->_ref = ref;
  }
};

class ExprAssignNode : public AstNode {
public:
  std::string _varName;
  ExprNode *_expr;

  ExprAssignNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::VAR_ASSIGN, line, startCol, parent) {}
};

class ExprVarRefNode : public AstNode {
public:
  std::string _varName;
  ExprVarRefNode *_ref;
  bool _arrIndexed;
  ExprNode *_arrIndex;

  ExprVarRefNode(int line, int startCol, AstNode *parent, std::string varName,
                 ExprVarRefNode *ref = nullptr, ExprNode *arrIndex = nullptr)
      : AstNode(NodeType::VAR_REF, line, startCol, parent) {
    this->_varName = varName;
    this->_ref = ref;
    if (arrIndex) {
      _arrIndexed = true;
      arrIndex = arrIndex;
    } else {
      _arrIndexed = false;
      arrIndex = nullptr;
    }
  }
};

class ExprNode : public AstNode {
public:
  ExprNode(int line, int startCol, AstNode *parent)
      : AstNode(NodeType::EXPR, line, startCol, parent) {}
};

class ExprBinaryNode : public AstNode {
public:
  AstNode *_left;
  std::string _op;
  AstNode *_right;

  ExprBinaryNode(int line, int startCol, AstNode *parent, AstNode *left,
                 std::string op, AstNode *right)
      : AstNode(NodeType::EXPR_BINARY, line, startCol, parent) {
    this->_left = left;
    this->_op = op;
    this->_right = right;
  }
};

class ExprUnaryNode : public AstNode {
public:
  std::string _op;
  AstNode *_expr;

  ExprUnaryNode(int line, int startCol, AstNode *parent, std::string op,
                AstNode *expr)
      : AstNode(NodeType::EXPR_UNARY, line, startCol, parent) {
    this->_op = op;
    this->_expr = expr;
  }
};

class ExprConstantNode : public AstNode {
public:
  std::string _rawValue;
  int _rawType;

  ExprConstantNode(int line, int startCol, AstNode *parent,
                   std::string rawValue, int rawType)
      : AstNode(NodeType::EXPR_CONSTANT, line, startCol, parent) {
    this->_rawValue = rawValue;
    this->_rawType = rawType;
  }
};

#endif