#ifndef _generics
#define _generics

#include "astCloner.h"
#include <random>
#include <string>
#include <utility>

class GenericsVisitor : public BaseVisitor {
public:
  GenericsVisitor(AstCloner *astCloner) { this->astCloner = astCloner; }

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
    }

    if (!_struct)
      error("Type " + node->_rawIdent + " does not exists.");

    auto structGenericParams = _struct->_genericArgNames;

    if (structGenericParams.size() != node->genericArgsDefs.size())
      error("Invalid template argument size for struct " + node->_rawIdent +
            " expecting " + std::to_string(structGenericParams.size()));

    auto implHash = generateArgsHash(node->genericArgsDefs);
    if (implementationMap.find(implHash) != implementationMap.end()) {
      auto structName = implementationMap[implHash];
      auto structDef = program->structDefs[structName];
      node->_rawIdent = structDef->_name;
      node->genericArgsDefs.clear();
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
    program->structDefs[clonedStruct->_name] = clonedStruct;
    clonedStruct->_parent = program;

    ulint index = program->_children.size();
    program->_children.push_back(clonedStruct);
    while (index > _iStruct) {
      std::swap(program->_children[index-1], program->_children[index]);
      index--;
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

  std::string generateTypeHash(TypeDefNode *typeDef) {
    if (typeDef->_pointsTo)
      return "*" + generateTypeHash(typeDef);
    if (typeDef->_arrayOf)
      return "[]" + generateTypeHash(typeDef);
    return typeDef->_rawIdent;
  }

  std::string generateArgsHash(std::vector<TypeDefNode *> &args) {
    std::string hash = "";
    for (auto arg : args) {
      hash += generateTypeHash(arg);
      hash += '-';
    }
    return hash;
  }

  std::string generateHash() {
    std::string hash = "";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0x60, 0x7a);

    for (int i = 0; i < 20; i++)
      hash += dis(gen);

    return hash;
  }
};

#endif