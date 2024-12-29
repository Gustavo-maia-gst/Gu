#ifndef _ast
#define _ast
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

enum class NodeType {
  PROGRAM,
  FUNCTION,
  BODY,
  STATEMENT,
  IF,
  WHILE,
  FOR,
  FUNCCALL,
  VAR_ASSIGN,
  VAR_DEF,
  VAR_REF,
  TYPE_DEF,

  EXPR,
  EXPR_PLUS_MINUS,
  EXPR_MULT_DIV,
  EXPR_BOOLEAN,
  EXPR_BIN,
  EXPR_COMPARISON,
  EXPR_CONSTANT,
};

class AstNode;

class ProgramNode;
class FunctionNode;
class BodyNode;
class StatementNode;
class IfNode;
class WhileNode;
class ForNode;
class TypeDefNode;
class VarDefNode;

class ExprAssignNode;
class ExprVarRefNode;
class ExprCallNode;
class ExprNode;
class ExprPlusMinusNode;
class ExprMultDivNode;
class ExprBinNode;
class ExprBooleanNode;
class ExprComparisonNode;
class ExprConstantNode;

class BaseVisitor {
public:
  virtual void visitProgram(ProgramNode *node) {};
  virtual void visitFunction(FunctionNode *node) {};
  virtual void visitBody(BodyNode *node) {};
  virtual void visitStatement(StatementNode *node) {};
  virtual void visitIf(IfNode *node) {};
  virtual void visitWhile(WhileNode *node) {};
  virtual void visitFor(ForNode *node) {};
  virtual void visitVarDef(VarDefNode *node) {};
  virtual void visitTypeDefNode(TypeDefNode *node) {};

  virtual void visitExpr(ExprNode *node) {};
  virtual void visitExprAssign(ExprAssignNode *node) {};
  virtual void visitExprVarRef(ExprVarRefNode *node) {};
  virtual void visitExprCall(ExprCallNode *node) {};
  virtual void visitExprPlusMinus(ExprPlusMinusNode *node) {};
  virtual void visitExprMultDiv(ExprMultDivNode *node) {};
  virtual void visitExprBoolean(ExprBooleanNode *node) {};
  virtual void visitExprBin(ExprBinNode *node) {};
  virtual void visitExprComparison(ExprComparisonNode *node) {};
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
  BodyNode *body;
  std::unordered_map<std::string, VarDefNode *> localVars;

  FunctionNode(AstNode *parent, std::string ident)
      : AstNode(NodeType::FUNCTION, parent) {
    this->name = ident;
  }

  std::string getName() { return name; }

private:
  std::string name;
};

class BodyNode : public AstNode {
  std::vector<StatementNode *> statements;

  BodyNode(AstNode *parent) : AstNode(NodeType::BODY, parent) {}
};

class StatementNode : public AstNode {
  StatementNode(AstNode *parent) : AstNode(NodeType::STATEMENT, parent) {}
};

class IfNode : public AstNode {
  ExprNode *expr;
  BodyNode *ifBody;
  BodyNode *elseBody;

  IfNode(AstNode *parent) : AstNode(NodeType::IF, parent) {}
};

class WhileNode : public AstNode {
  ExprNode *expr;
  BodyNode *body;

  WhileNode(AstNode *parent) : AstNode(NodeType::WHILE, parent) {}
};

class ForNode : public AstNode {
  ExprNode *start;
  ExprNode *cond;
  ExprNode *inc;
  BodyNode *body;

  ForNode(AstNode *parent) : AstNode(NodeType::FOR, parent) {}
};

class VarDefNode : public AstNode {};

class TypeDefNode : public AstNode {
public:
  enum Kind {
    ARRAY,
    POINTER,
    REGULAR,
  };

  static TypeDefNode *build(std::string type, AstNode *parent) {
    auto typeDef = new TypeDefNode(parent);
    typeDef->type = type;
    return typeDef;
  }
  static TypeDefNode *buildPointer(TypeDefNode *type, AstNode *parent) {
    auto typeDef = new TypeDefNode(parent);
    typeDef->pointsTo = type;
    return typeDef;
  }
  static TypeDefNode *buildArray(TypeDefNode *type, AstNode *parent) {
    auto typeDef = new TypeDefNode(parent);
    typeDef->arrayOf = type;
    return typeDef;
  }

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
  TypeDefNode(AstNode *parent) : AstNode(NodeType::TYPE_DEF, parent) {}
  TypeDefNode *pointsTo;
  TypeDefNode *arrayOf;
  std::string type;
};

class ExprCallNode : public AstNode {
  std::string funcName;
  std::vector<ExprNode *> params;

  ExprCallNode(AstNode *parent) : AstNode(NodeType::FUNCCALL, parent) {}
};

class ExprAssignNode : public AstNode {
  std::string varName;
  ExprNode *expr;

  ExprAssignNode(AstNode *parent) : AstNode(NodeType::VAR_ASSIGN, parent) {}
};

class ExprVarRefNode : public AstNode {
  std::string varName;

  ExprVarRefNode(AstNode *parent) : AstNode(NodeType::VAR_REF, parent) {}
};

class ExprNode : public AstNode {
  ExprNode(AstNode *parent) : AstNode(NodeType::EXPR, parent) {}
};

class ExprPlusMinusNode : public AstNode {
  AstNode *left;
  std::string op;
  AstNode *right;

  ExprPlusMinusNode(AstNode *left, std::string op, AstNode *right,
                    AstNode *parent)
      : AstNode(NodeType::EXPR_PLUS_MINUS, parent) {}
};

class ExprMultDivNode : public AstNode {
  AstNode *left;
  std::string op;
  AstNode *right;

  ExprMultDivNode(AstNode *left, std::string op, AstNode *right,
                  AstNode *parent)
      : AstNode(NodeType::EXPR_MULT_DIV, parent) {}
};

class ExprComparisonNode : public AstNode {
  AstNode *left;
  std::string op;
  AstNode *right;

  ExprComparisonNode(AstNode *left, std::string op, AstNode *right,
                     AstNode *parent)
      : AstNode(NodeType::EXPR_COMPARISON, parent) {}
};

class ExprBooleanNode : public AstNode {
  AstNode *left;
  std::string op;
  AstNode *right;

  ExprBooleanNode(AstNode *left, std::string op, AstNode *right,
                  AstNode *parent)
      : AstNode(NodeType::EXPR_BOOLEAN, parent) {}
};

class ExprBinNode : public AstNode {
  AstNode *left;
  std::string op;
  AstNode *right;

  ExprBinNode(AstNode *left, std::string op, AstNode *right, AstNode *parent)
      : AstNode(NodeType::EXPR_BIN, parent) {}
};

class ExprConstantNode : public AstNode {
  std::string rawValue;

  ExprConstantNode(std::string rawValue, AstNode *parent)
      : AstNode(NodeType::EXPR_CONSTANT, parent) {
    this->rawValue = rawValue;
  }
};

#endif