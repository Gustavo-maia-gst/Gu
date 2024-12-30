#include "validator.h"

void SemanticValidator::visitProgram(ProgramNode *node) {
  if (this->program)
    unexpected_error("Duplicated ProgramNode");

  this->program = node;

  for (auto child : node->_children) {
    auto childType = child->getNodeType();
    switch (childType) {
    case NodeType::FUNCTION: {
      auto funcNode = (FunctionNode *)child;
      if (node->funcs.find(funcNode->_name) != node->funcs.end())
        compile_error("Duplicated function name: " + funcNode->_name);
      node->funcs[funcNode->_name] = funcNode;
      break;
    }
    case NodeType::STRUCT_DEF: {
      auto structNode = (StructDefNode *)child;
      if (node->structDefs.find(structNode->_name) != node->structDefs.end())
        compile_error("Duplicated function name: " + structNode->_name);
      node->structDefs[structNode->_name] = structNode;
      break;
    }
    case NodeType::VAR_DEF: {
      auto varDefNode = (VarDefNode *)child;
      if (node->globalVars.find(varDefNode->_name) != node->globalVars.end())
        compile_error("Duplicated function name: " + varDefNode->_name);
      node->globalVars[varDefNode->_name] = varDefNode;
      break;
    }
    default:
      unexpected_error("Invalid nodeType in ProgramNode children");
      break;
    }
  }

  node->visitChildren(this);
}

void SemanticValidator::visitFunction(FunctionNode *node) {
  this->function = node;

  bool defaultParamStarted = false;
  for (auto param : node->_params) {
    if (node->localVars.find(param->_name) != node->localVars.end())
      compile_error("Duplicated param name");
    node->localVars[param->_name] = param;

    if (defaultParamStarted && !param->_defaultVal)
      compile_error("Default param value before non-default param");
    if (param->_defaultVal)
      defaultParamStarted = true;
  }

  node->visitChildren(this);
}

void SemanticValidator::visitStructDef(StructDefNode *node) {

  node->visitChildren(this);

  int currentOffset = 0;
  for (auto member : node->_members) {
    switch (member->getNodeType()) {
    case NodeType::VAR_DEF: {
      auto varDefNode = (VarDefNode *)member;
      if (node->membersDef.find(varDefNode->_name) != node->membersDef.end())
        compile_error("Duplicated member name: " + varDefNode->_name);
      if (node->funcMembers.find(varDefNode->_name) != node->funcMembers.end())
        compile_error("Duplicated member name: " + varDefNode->_name);

      node->membersDef[varDefNode->_name] = varDefNode;
      node->membersOffset[varDefNode->_name] = currentOffset;
      currentOffset += varDefNode->_type->dataType->size;
      break;
    }
    case NodeType::FUNCTION: {
      auto funcNode = (FunctionNode *)member;
      if (node->membersDef.find(funcNode->_name) != node->membersDef.end())
        compile_error("Duplicated member name: " + funcNode->_name);
      if (node->funcMembers.find(funcNode->_name) != node->funcMembers.end())
        compile_error("Duplicated member name: " + funcNode->_name);

      node->funcMembers[funcNode->_name] = funcNode;

      auto wrongParameter = [&]() {
        compile_error(
            "Function " + funcNode->_name +
            " has no parameters, function defined inside structs should "
            "receive a pointer to the struct as first parameter");
      };

      if (funcNode->_params.empty()) {
        wrongParameter();
        break;
      }

      auto firstParamType = funcNode->_params[0]->_type->dataType;
      if (firstParamType->type != RawDataType::POINTER) {
        wrongParameter();
        break;
      }
      if (firstParamType->inner->ident != node->_name) {
        wrongParameter();
        break;
      }

      break;
    }
    default:
      unexpected_error("Invalid nodeType in StructNode children");
      break;
    }
  }
}

void SemanticValidator::visitVarDef(VarDefNode *node) {
  node->visitChildren(this);
}

void SemanticValidator::visitTypeDefNode(TypeDefNode *node) {
  node->visitChildren(this);
  node->dataType = new DataType(node);

  DataType *datatype = node->dataType;
  while (datatype->type == RawDataType::ARRAY ||
         datatype->type == RawDataType::POINTER)
    datatype = datatype->inner;

  if (datatype->type != RawDataType::STRUCT)
    return;
  if (program->structDefs.find(datatype->ident) != program->structDefs.end())
    return;

  name_error("Invalid reference for struct: " + datatype->ident);
}