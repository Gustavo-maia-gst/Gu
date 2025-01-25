#ifndef _generics
#define _generics

#include "astCloner.h"
#include <random>
#include <string>
#include <utility>

class TemplatesVisitor : public BaseVisitor {
public:
  TemplatesVisitor(AstCloner *astCloner) { this->astCloner = astCloner; }

  void visitTypeDefNode(TypeDefNode *node) override {
    node->visitChildren(this);
    if (node->genericArgsDefs.empty())
      return;

    StructDefNode *_struct;
    ulint _iStruct;

    for (ulint i = 0; i < program->_children.size(); i++) {
      auto child = program->_children[i];
      if (child->getNodeType() != NodeType::STRUCT_DEF)
        continue;
      auto structChild = (StructDefNode *)child;
      if (node->_rawIdent != structChild->_name)
        continue;

      _struct = structChild;
      _iStruct = i;
      break;
    }

    if (!_struct)
      error("Type " + node->_rawIdent + " does not exists.");

    auto structGenericParams = _struct->_genericArgNames;

    if (structGenericParams.size() != node->genericArgsDefs.size())
      error("Invalid template argument size for struct " + node->_rawIdent +
            " expecting " + std::to_string(structGenericParams.size()));

    auto implHash =
        _struct->_name + "-" + generateArgsHash(node->genericArgsDefs);
    if (implementationMap.find(implHash) != implementationMap.end()) {
      auto structName = implementationMap[implHash];
      StructDefNode *structDef = nullptr;

      for (auto node : program->_children) {
        if (node->getNodeType() != NodeType::STRUCT_DEF)
          continue;
        auto _struct = (StructDefNode *)node;
        if (_struct->_name != structName)
          continue;
        structDef = _struct;
        break;
      }

      node->_rawIdent = structDef->_name;
      return;
    }

    astCloner->clearUpdates();
    astCloner->clearPrefix();
    auto prefix = generateHash();
    astCloner->setPrefix(prefix);

    for (ulint i = 0; i < structGenericParams.size(); i++)
      astCloner->setUpdateType(structGenericParams[i],
                               node->genericArgsDefs[i]);

    astCloner->visitStructDef(_struct);
    auto clonedStruct = (StructDefNode *)astCloner->getCloned();

    clonedStruct->_parent = program;
    ulint index = program->_children.size();
    program->_children.push_back(clonedStruct);
    while (--index > _iStruct) {
      auto temp = program->_children[index];
      program->_children[index] = program->_children[index + 1];
      program->_children[index + 1] = temp;
    }

    implementationMap[implHash] = clonedStruct->_name;
    node->_rawIdent = clonedStruct->_name;
    node->genericArgsDefs.clear();
  };

  void visitStructDef(StructDefNode *node) override {
    if (!node->_genericArgNames.empty())
      return;
    node->visitChildren(this);
  };

  void visitProgram(ProgramNode *node) override {
    program = node;
    node->visitChildren(this);
  };

  void visitFunction(FunctionNode *node) override {
    node->visitChildren(this);
  };
  void visitBody(BodyNode *node) override { node->visitChildren(this); };
  void visitIf(IfNode *node) override { node->visitChildren(this); };
  void visitWhile(WhileNode *node) override { node->visitChildren(this); };
  void visitFor(ForNode *node) override { node->visitChildren(this); };
  void visitVarDef(VarDefNode *node) override { node->visitChildren(this); };
  void visitBreakNode(BreakNode *node) override { node->visitChildren(this); };
  void visitReturnNode(ReturnNode *node) override {
    node->visitChildren(this);
  };

  void visitExprBinaryOp(ExprBinaryNode *node) override {
    node->visitChildren(this);
  };
  void visitMemberAccess(ExprMemberAccess *node) override {
    node->visitChildren(this);
  };
  void visitIndexAccess(ExprIndex *node) override {
    node->visitChildren(this);
  };
  void visitExprCall(ExprCallNode *node) override {
    node->visitChildren(this);
  };
  void visitExprUnaryOp(ExprUnaryNode *node) override {
    node->visitChildren(this);
  };

  void visitExprVarRef(ExprVarRefNode *node) override {
    node->visitChildren(this);
  };

private:
  AstCloner *astCloner;
  ProgramNode *program;
  std::map<std::string, std::string> implementationMap;

  void error(std::string msg) {
    std::cerr << "template usage error: " << msg << std::endl;
    exit(1);
  }

  std::string generateArgsHash(std::vector<TypeDefNode *> &args) {
    std::string hash = "";
    for (auto arg : args) {
      hash += generateTypeHash(arg);
      hash += '-';
    }
    return hash;
  }

  std::string generateTypeHash(TypeDefNode *typeDef) {
    if (typeDef->_pointsTo)
      return "*" + generateTypeHash(typeDef->_pointsTo);
    if (typeDef->_arrayOf)
      return "[]" + generateTypeHash(typeDef->_arrayOf);
    return typeDef->_rawIdent;
  }

  std::string generateHash() {
    std::string hash = "";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0x61, 0x7a);

    for (int i = 0; i < 20; i++)
      hash += dis(gen);

    return hash;
  }
};

#endif