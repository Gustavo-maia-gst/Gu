#include "validator.h"

const std::string MAIN_FUNC = "main";

SemanticValidator::SemanticValidator() {
  function = nullptr;
  program = nullptr;
}

void SemanticValidator::visitProgram(ProgramNode *node) {
  if (this->program)
    unexpected_error("Duplicated ProgramNode", node);

  this->program = node;

  for (auto child : node->_children) {
    auto childType = child->getNodeType();
    switch (childType) {
    case NodeType::FUNCTION: {
      auto funcNode = (FunctionNode *)child;
      if (node->funcs.find(funcNode->_name) != node->funcs.end())
        compile_error("Duplicated function name: " + funcNode->_name, node);
      node->funcs[funcNode->_name] = funcNode;
      break;
    }
    case NodeType::STRUCT_DEF: {
      auto structNode = (StructDefNode *)child;
      if (node->structDefs.find(structNode->_name) != node->structDefs.end())
        compile_error("Duplicated function name: " + structNode->_name, node);
      node->structDefs[structNode->_name] = structNode;
      break;
    }
    case NodeType::VAR_DEF: {
      auto varDefNode = (VarDefNode *)child;
      if (node->globalVars.find(varDefNode->_name) != node->globalVars.end())
        compile_error("Duplicated function name: " + varDefNode->_name, node);
      node->globalVars[varDefNode->_name] = varDefNode;
      break;
    }
    default:
      unexpected_error("Invalid nodeType in ProgramNode children", node);
      break;
    }
  }

  if (!node->funcs[MAIN_FUNC])
    compile_error("main function not defined ", node);

  node->visitChildren(this);
}

void SemanticValidator::visitFunction(FunctionNode *node) {
  for (auto param : node->_params) {
    if (node->localVars.find(param->_name) != node->localVars.end())
      compile_error("Duplicated param name", node);
    node->localVars[param->_name] = param;

    if (param->_defaultVal)
      compile_error("Assign in parameter declaration", node);
  }

  for (auto localVar : node->_innerVars) {
    if (node->localVars.find(localVar->_name) != node->localVars.end())
      compile_error("Duplicated local variable name", node);
    node->localVars[localVar->_name] = localVar;
  }

  hasReturn = false;
  function = node;

  node->visitChildren(this);

  node->retType = node->_retTypeDef->dataType;
  if (!hasReturn && node->retType->raw != RawDataType::VOID)
    compile_error("non-void function must have a return", node);
}

void SemanticValidator::visitStructDef(StructDefNode *node) {
  int currentOffset = 0;

  std::vector<FunctionNode *> structFuncs;

  for (auto member : node->_members) {
    switch (member->getNodeType()) {
    case NodeType::VAR_DEF: {
      member->visit(this);
      auto varDefNode = (VarDefNode *)member;
      if (node->membersDef.find(varDefNode->_name) != node->membersDef.end())
        compile_error("Duplicated name for member, there are another struct "
                      "member with the same name: " +
                          varDefNode->_name,
                      node);
      if (node->funcMembers.find(varDefNode->_name) != node->funcMembers.end())
        compile_error("Duplicated name for member, there are another struct "
                      "function with the same name" +
                          varDefNode->_name,
                      node);

      node->membersDef[varDefNode->_name] = varDefNode;
      node->membersOffset[varDefNode->_name] = currentOffset;
      currentOffset += varDefNode->_typeDef->dataType->size;
      break;
    }
    case NodeType::FUNCTION:
      structFuncs.push_back((FunctionNode *)member);
      break;
    default:
      unexpected_error("Invalid nodeType in StructNode children", node);
      break;
    }
  }

  for (auto funcNode : structFuncs) {
    funcNode->visit(this);

    if (node->membersDef.find(funcNode->_name) != node->membersDef.end())
      compile_error("Duplicated name for function, there are another struct "
                    "function with the same name" +
                        funcNode->_name,
                    node);
    if (node->funcMembers.find(funcNode->_name) != node->funcMembers.end())
      compile_error("Duplicated name for function, there are another struct "
                    "function with the same name" +
                        funcNode->_name,
                    node);

    node->funcMembers[funcNode->_name] = funcNode;

    auto wrongParameter = [&]() {
      compile_error(
          "Function " + funcNode->_name +
              " has no parameters, function defined inside structs should "
              "receive a pointer to the struct as first parameter",
          node);
    };

    if (funcNode->_params.empty()) {
      wrongParameter();
      break;
    }

    auto firstParamType = funcNode->_params[0]->_typeDef->dataType;
    if (firstParamType->raw != RawDataType::POINTER) {
      wrongParameter();
      break;
    }
    if (firstParamType->inner->ident != node->_name) {
      wrongParameter();
      break;
    }
  }
}

void SemanticValidator::visitVarDef(VarDefNode *node) {
  node->visitChildren(this);

  node->type = node->_typeDef->dataType;
  if (function)
    function->localVars[node->_name] = node;

  if (node->_defaultVal &&
      DataType::getResultType(node->type, "=", node->_defaultVal->type) !=
          node->type)
    type_error("non-compatible type assignment", node);
}

void SemanticValidator::visitTypeDefNode(TypeDefNode *node) {
  node->visitChildren(this);
  node->dataType = DataType::build(node);

  DataType *datatype = node->dataType;
  if (!datatype) {
    unexpected_error("Invalid datatype for TypeDefNode", node);
    return;
  }

  while (DataType::isAddress(datatype->raw))
    datatype = datatype->inner;

  if (datatype->raw != RawDataType::STRUCT)
    return;
  if (resolveStruct(datatype->ident))
    return;

  name_error("Invalid reference for struct: " + datatype->ident, node);
}

void SemanticValidator::visitBreakNode(BreakNode *node) {
  AstNode *target = node;
  while (target && !target->isLoopNode())
    target = target->_parent;
  if (!target)
    compile_error("break used outside any loop", node);
}

void SemanticValidator::visitReturnNode(ReturnNode *node) {
  hasReturn = true;

  node->visitChildren(this);

  if (!this->function) {
    compile_error("Return statement outside any function", node);
    return;
  }

  auto funcRetType = function->_retTypeDef->dataType;
  if (DataType::getResultType(funcRetType, "=", node->_expr->type) !=
      funcRetType) {
    type_error("Incompatible return type", node);
  }
}

void SemanticValidator::visitExprVarRef(ExprVarRefNode *node) {
  if (node->var) {
    node->type = node->var->type;
    return;
  }
  if (node->func) {
    node->type = DataType::build(RawDataType::FUNCTION);
    return;
  }

  auto varDef = resolveVar(node->_ident);
  auto funcDef = resolveFunction(node->_ident);
  if (!varDef && !funcDef) {
    name_error("Reference " + node->_ident + " does not exists", node);
    node->type = DataType::build(RawDataType::ERROR);
    return;
  }

  node->var = varDef;
  node->func = funcDef;
  node->type = varDef ? varDef->type : DataType::build(RawDataType::FUNCTION);

  return;
}

void SemanticValidator::visitExprConstant(ExprConstantNode *node) {
  switch (node->_rawType) {
  case LEX_NUMBER:
    node->type = DataType::fromNumber(node->_rawValue);
    break;
  case LEX_CHAR:
    node->type = DataType::fromChar(node->_rawValue);
    break;
  case LEX_STRING:
    node->type = DataType::fromString(node->_rawValue);
    break;
  default:
    unexpected_error("Non-constant _rawType in ExprConstantNode", node);
    return;
  }
}

void SemanticValidator::visitExprUnaryOp(ExprUnaryNode *node) {
  node->visitChildren(this);

  switch (node->_opNum) {
  case MULT: {
    if (!DataType::isAddress(node->_expr->type->raw)) {
      compile_error("Dereference of non-pointer datatype", node);
      node->type = DataType::build(RawDataType::ERROR);
      break;
    }

    node->type = node->_expr->type->inner;
    break;
  }
  case B_AND: {
    if (node->_expr->getNodeType() == NodeType::EXPR_CONSTANT) {
      compile_error("Dereference of non-pointer datatype", node);
      node->type = DataType::build(RawDataType::ERROR);
      break;
    }
    node->type = DataType::buildPointer(node->_expr->type);
    break;
  }
  default:
    node->type = node->_expr->type;
    break;
  }
}

void SemanticValidator::visitExprBinaryOp(ExprBinaryNode *node) {
  node->visitChildren(this);
  node->type =
      DataType::getResultType(node->_left->type, node->_op, node->_right->type);

  if (node->type->raw == RawDataType::ERROR &&
      (node->_left->type != node->_right->type)) {
    type_error("Expression with invalid types for operator " + node->_op, node);
  }
}

void SemanticValidator::visitMemberAccess(ExprMemberAccess *node) {
  node->_struct->visit(this);
  node->type = DataType::build(RawDataType::ERROR);

  if (node->_struct->type->raw != RawDataType::STRUCT) {
    compile_error("Accessing member of non-struct type", node);
    return;
  }

  auto structName = node->_struct->type->ident;
  auto structDef = resolveStruct(structName);
  if (!structDef) {
    name_error("Struct " + structName + " does not exist", node);
    return;
  }

  auto memberDef = structDef->membersDef.find(node->_memberName) !=
                           structDef->membersDef.end()
                       ? structDef->membersDef[node->_memberName]
                       : nullptr;
  auto funcDef = structDef->funcMembers.find(node->_memberName) !=
                         structDef->funcMembers.end()
                     ? structDef->funcMembers[node->_memberName]
                     : nullptr;

  if (!memberDef && !funcDef) {
    name_error("Member " + node->_memberName + " does not exist on struct " +
                   structName,
               node);
    return;
  }

  node->var = memberDef;
  node->func = funcDef;

  node->type = memberDef ? memberDef->type : funcDef->retType;
}

void SemanticValidator::visitIndexAccess(ExprIndex *node) {
  node->visitChildren(this);

  if (!node->_inner->type || !DataType::isAddress(node->_inner->type->raw)) {
    type_error("Indexing non-address datatype", node);
    return;
  }

  if (!node->_index->type || !DataType::isInt(node->_index->type->raw)) {
    type_error("Indexing with non-int type", node);
    return;
  }

  node->type = node->_inner->type->inner;
}

void SemanticValidator::visitExprCall(ExprCallNode *node) {
  node->visitChildren(this);

  if (node->_ref->type->raw != RawDataType::FUNCTION) {
    compile_error("Calling non-function type", node);
    node->type = DataType::build(RawDataType::ERROR);
    return;
  }
  if (!node->func) {
    unexpected_error(
        "Function type expression does not contain func definition", node);
    return;
  }

  if (node->_args.size() != node->func->_params.size()) {
    compile_error("Invalid argument count for function + " + node->func->_name,
                  node);
    return;
  }

  for (int i = 0; i < node->_args.size(); i++)
    if (node->_args[i]->type != node->func->_params[i]->type) {
      type_error("Invalid type for argument " + std::to_string(i) +
                     " in function call",
                 node);
      return;
    }
}

void SemanticValidator::unexpected_error(std::string msg, AstNode *node) {
  errors.push_back(
      "Sorry, an unexpected error has occurred, please report it including "
      "the "
      "source code at our repository: https://github.com/Gustavo-maia-gst/gu"
      "\n"
      "Error: " +
      msg + " at: " + std::to_string(node->_line) + ":" +
      std::to_string(node->_startCol));
};

void SemanticValidator::compile_error(std::string msg, AstNode *node) {
  errors.push_back("compile error: " + msg +
                   " at: " + std::to_string(node->_line) + ":" +
                   std::to_string(node->_startCol));
}

void SemanticValidator::type_error(std::string msg, AstNode *node) {
  errors.push_back("type error: " + msg +
                   " at: " + std::to_string(node->_line) + ":" +
                   std::to_string(node->_startCol));
}

void SemanticValidator::name_error(std::string msg, AstNode *node) {
  errors.push_back("name error: " + msg +
                   " at: " + std::to_string(node->_line) + ":" +
                   std::to_string(node->_startCol));
}

VarDefNode *SemanticValidator::resolveVar(std::string &varName) {
  if (!program)
    return nullptr;

  VarDefNode *globalVar, *localVar;

  globalVar = program->globalVars.find(varName) != program->globalVars.end()
                  ? program->globalVars[varName]
                  : nullptr;

  if (function)
    localVar = function->localVars.find(varName) != function->localVars.end()
                   ? function->localVars[varName]
                   : nullptr;

  return localVar ? localVar : globalVar;
}

StructDefNode *SemanticValidator::resolveStruct(std::string &structName) {
  if (!program)
    return nullptr;

  return program->structDefs.find(structName) != program->structDefs.end()
             ? program->structDefs[structName]
             : nullptr;
}

FunctionNode *SemanticValidator::resolveFunction(std::string &funcName) {
  if (!program)
    return nullptr;

  return program->funcs.find(funcName) != program->funcs.end()
             ? program->funcs[funcName]
             : nullptr;
}