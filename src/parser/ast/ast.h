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

class AstNode {
public:
  AstNode(NodeType nodeType, AstNode *parent = nullptr) {
    this->nodeType = nodeType;
    this->parent = parent;
    if (parent)
      parent->children.push_back(this);
  };

  void visit(BaseVisitor *visitor);
  void visitChildren(BaseVisitor *visitor);

  NodeType getNodeType();
  AstNode *parent;
  std::vector<AstNode *> children;

private:
  NodeType nodeType;
};

class ProgramNode : public AstNode {
public:
  std::unordered_map<std::string, FunctionNode *> funcs;
  std::unordered_map<std::string, VarDefNode *> globalVars;

  ProgramNode() : AstNode(NodeType::PROGRAM, nullptr) {
    this->children = children;
  }
};

class FunctionNode : public AstNode {
public:
  std::vector<VarDefNode *> params;
  BlockNode *body;
  TypeDefNode *retType;
  std::unordered_map<std::string, VarDefNode *> localVars;

  FunctionNode(AstNode *parent, std::string ident)
      : AstNode(NodeType::FUNCTION, parent) {
    this->name = ident;
  }

  std::string getName() { return name; }

private:
  std::string name;
};

class BlockNode : public AstNode {
public:
  std::vector<AstNode *> statements;
  BlockNode(AstNode *parent) : AstNode(NodeType::BODY, parent) {}
};

class IfNode : public AstNode {
public:
  ExprNode *expr;
  BlockNode *ifBody;
  BlockNode *elseBody;

  IfNode(AstNode *parent) : AstNode(NodeType::IF, parent) {}
};

class WhileNode : public AstNode {
public:
  ExprNode *expr;
  BlockNode *body;

  WhileNode(AstNode *parent) : AstNode(NodeType::WHILE, parent) {}
};

class ForNode : public AstNode {
public:
  ExprNode *start;
  ExprNode *cond;
  ExprNode *inc;
  BlockNode *body;

  ForNode(AstNode *parent) : AstNode(NodeType::FOR, parent) {}
};

class VarDefNode : public AstNode {
public:
  std::string name;
  TypeDefNode *type;
  ExprNode *defaultVal;

  VarDefNode(AstNode *parent, std::string name)
      : AstNode(NodeType::VAR_DEF, parent) {
    this->name = name;
  }
};

class StructDefNode : public AstNode {
public:
  std::string name;
  std::vector<VarDefNode *> members;

  StructDefNode(AstNode *parent, std::string name)
      : AstNode(NodeType::STRUCT_DEF, parent) {
    this->name = name;
  }
};

class BreakNode : public AstNode {
public:
  BreakNode(AstNode *parent) : AstNode(NodeType::BREAK, parent) {}
};

class ReturnNode : public AstNode {
public:
  ExprNode *expr;
  ReturnNode(AstNode *parent) : AstNode(NodeType::RETURN, parent) {}
};

class TypeDefNode : public AstNode {
public:
  enum Kind {
    ARRAY,
    POINTER,
    REGULAR,
  };

  static TypeDefNode *build(std::string type) {
    auto typeDef = new TypeDefNode();
    typeDef->type = type;
    return typeDef;
  }
  static TypeDefNode *buildPointer(TypeDefNode *innerType) {
    auto typeDef = new TypeDefNode();
    typeDef->pointsTo = innerType;
    innerType->parent = typeDef;
    typeDef->children.push_back(innerType);
    return typeDef;
  }
  static TypeDefNode *buildArray(TypeDefNode *innerType, int size) {
    auto typeDef = new TypeDefNode();
    typeDef->arrayOf = innerType;
    typeDef->size = size;
    innerType->parent = typeDef;
    typeDef->children.push_back(innerType);
    return typeDef;
  }

  std::string getBaseType() { return type; }

  TypeDefNode *getInnerType() {
    if (pointsTo)
      return pointsTo;
    if (arrayOf)
      return arrayOf;
    return nullptr;
  }

  TypeDefNode::Kind getKind() {
    if (this->pointsTo)
      return TypeDefNode::Kind::POINTER;
    if (this->arrayOf)
      return TypeDefNode::Kind::ARRAY;
    return TypeDefNode::Kind::REGULAR;
  }

private:
  TypeDefNode() : AstNode(NodeType::TYPE_DEF, nullptr) {}
  TypeDefNode *pointsTo;
  TypeDefNode *arrayOf;
  int size;
  std::string type;
};

class ExprCallNode : public AstNode {
public:
  std::vector<AstNode *> params;
  ExprVarRefNode *ref;

  ExprCallNode(AstNode *parent, ExprVarRefNode *ref = nullptr)
      : AstNode(NodeType::FUNCCALL, parent) {
    this->ref = ref;
  }
};

class ExprAssignNode : public AstNode {
public:
  std::string varName;
  ExprNode *expr;

  ExprAssignNode(AstNode *parent) : AstNode(NodeType::VAR_ASSIGN, parent) {}
};

class ExprVarRefNode : public AstNode {
public:
  std::string varName;
  ExprVarRefNode *ref;
  bool arrIndexed;
  ExprNode *arrIndex;

  ExprVarRefNode(AstNode *parent, std::string varName,
                 ExprVarRefNode *ref = nullptr, ExprNode *arrIndex = nullptr)
      : AstNode(NodeType::VAR_REF, parent) {
    this->varName = varName;
    this->ref = ref;
    if (arrIndex) {
      arrIndexed = true;
      arrIndex = arrIndex;
    } else {
      arrIndexed = false;
      arrIndex = nullptr;
    }
  }
};

class ExprNode : public AstNode {
public:
  ExprNode(AstNode *parent) : AstNode(NodeType::EXPR, parent) {}
};

class ExprBinaryNode : public AstNode {
public:
  AstNode *left;
  std::string op;
  AstNode *right;

  ExprBinaryNode(AstNode *left, std::string op, AstNode *right, AstNode *parent)
      : AstNode(NodeType::EXPR_BINARY, parent) {
    this->left = left;
    this->op = op;
    this->right = right;
  }
};

class ExprUnaryNode : public AstNode {
public:
  std::string op;
  AstNode *expr;

  ExprUnaryNode(std::string op, AstNode *expr, AstNode *parent)
      : AstNode(NodeType::EXPR_UNARY, parent) {
    this->op = op;
    this->expr = expr;
  }
};

class ExprConstantNode : public AstNode {
public:
  std::string rawValue;
  int rawType;

  ExprConstantNode(std::string rawValue, int rawType, AstNode *parent)
      : AstNode(NodeType::EXPR_CONSTANT, parent) {
    this->rawValue = rawValue;
    this->rawType = rawType;
  }
};

#endif