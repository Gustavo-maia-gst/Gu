#include "validator.h"

const std::string MAIN_FUNC = "main";

inline bool isValidConditionType(DataType *type) {
  return DataType::isNumeric(type->raw) || DataType::isAddress(type->raw);
}

SemanticValidator::SemanticValidator(bool validateMain) {
  function = nullptr;
  program = nullptr;
  this->validateMain = validateMain;
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

  if (!node->funcs[MAIN_FUNC] && validateMain)
    compile_error("main function not defined ", node);

  node->visitChildren(this);
}

void SemanticValidator::visitFunction(FunctionNode *node) {
  if (reservedFunctions.find(node->_name) != reservedFunctions.end()) {
    name_error("Use of reserved function name " + node->_name, node);
    node->retType = DataType::build(RawDataType::ERROR);
    return;
  }

  for (auto param : node->_params) {
    if (node->localVars.find(param->_name) != node->localVars.end())
      compile_error("Duplicated param name", node);
    node->localVars[param->_name] = param;

    if (param->_defaultVal)
      compile_error("Assign in parameter declaration", node);
  }
  function = node;

  if (node->_external) {
    if (!node->retType) {
      node->visitChildren(this);
      node->retType = node->_retTypeDef->dataType;
    }
    return;
  }

  for (auto localVar : node->_innerVars) {
    if (node->localVars.find(localVar->_name) != node->localVars.end())
      compile_error("Duplicated local variable name", node);
    node->localVars[localVar->_name] = localVar;
  }

  node->visitChildren(this);
  node->retType = node->_retTypeDef->dataType;

  if (!outWithReturn && node->retType->raw != RawDataType::VOID)
    compile_error("Control reaches end of non-void function", node);
}

void SemanticValidator::visitStructDef(StructDefNode *node) {
  int currentOffset = 0;

  std::vector<FunctionNode *> structFuncs;

  if (!node->_genericArgNames.empty()) return;

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
      node->membersOffset[varDefNode->_name] = currentOffset++;
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

void SemanticValidator::visitBody(BodyNode *node) {
  outWithReturn = false;
  for (ulint i = 0; i < node->_statements.size(); i++) {
    auto statement = node->_statements[i];
    statement->visit(this);

    if (!outWithReturn)
      continue;

    if (i < node->_statements.size() - 1)
      compile_error("Unreachable code after return statement", node);
    break;
  }
}

void SemanticValidator::visitVarDef(VarDefNode *node) {
  node->visitChildren(this);

  node->type = node->_typeDef->dataType;
  if (function)
    function->localVars[node->_name] = node;
  else {
    if (node->_defaultVal &&
        node->_defaultVal->getNodeType() != NodeType::EXPR_CONSTANT)
      compile_error("Default value in global variable should be constant",
                    node);
  }

  if (node->_defaultVal) {
    if (DataType::getResultType(node->type, "=", node->_defaultVal->type) !=
        node->type)
      type_error("non-compatible type assignment", node);

    if (node->_defaultVal->getNodeType() == NodeType::EXPR_CONSTANT)
      node->_defaultVal->type = node->type;
  }
}

void SemanticValidator::visitIf(IfNode *node) {
  node->_expr->visit(this);

  auto exprType = node->_expr->type->raw;

  node->_ifBody->visit(this);
  bool ifWithReturn = outWithReturn;
  bool elseWithReturn = false;

  if (node->_elseBody) {
    node->_elseBody->visit(this);
    elseWithReturn = outWithReturn;
  }

  if (ifWithReturn && elseWithReturn)
    outWithReturn = true;

  if (DataType::isNumeric(exprType))
    return;
  if (DataType::isAddress(exprType)) {
    node->_expr->type = DataType::build(RawDataType::LONG);
    return;
  }

  type_error("The condition type should be a numeric-based type", node);
}

void SemanticValidator::visitFor(ForNode *node) {
  node->visitChildren(this);
  outWithReturn = false;
  if (!isValidConditionType(node->_cond->type) &&
      node->_cond->type->raw != RawDataType::VOID)
    compile_error("Invalid condition in for", node);
}

void SemanticValidator::visitWhile(WhileNode *node) {
  node->visitChildren(this);
  outWithReturn = false;
  if (!isValidConditionType(node->_expr->type) &&
      node->_expr->type->raw != RawDataType::VOID)
    compile_error("Invalid condition in while", node);
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
  outWithReturn = true;

  node->visitChildren(this);

  if (!this->function) {
    compile_error("Return statement outside any function", node);
    return;
  }

  auto funcRetType = function->_retTypeDef->dataType;
  auto exprType =
      node->_expr ? node->_expr->type : DataType::build(RawDataType::VOID);
  if (DataType::getResultType(funcRetType, "=", exprType) != funcRetType)
    type_error("Incompatible return type", node);
  node->retType = funcRetType;
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
  case LEX_STRING: {
    std::string str = "";
    ulint i = 0;
    while (i < node->_rawValue.size()) {
      unsigned char c = node->_rawValue[i++];
      str += (c < 0x7f ? c : '?');
    }
    node->type = DataType::fromString(str);
    node->_rawValue = str;
    break;
  }
  default:
    unexpected_error("Non-constant _rawType in ExprConstantNode", node);
    return;
  }
}

void SemanticValidator::visitExprUnaryOp(ExprUnaryNode *node) {
  node->visitChildren(this);

  switch (node->_op[0]) {
  case '*': {
    if (!DataType::isAddress(node->_expr->type->raw)) {
      compile_error("Dereference of non-pointer datatype", node);
      node->type = DataType::build(RawDataType::ERROR);
      break;
    }

    node->type = node->_expr->type->inner;
    break;
  }
  case '&': {
    if (node->_expr->getNodeType() == NodeType::EXPR_CONSTANT) {
      compile_error("Reference of a constant value", node);
      node->type = DataType::build(RawDataType::ERROR);
      break;
    }
    node->type = DataType::buildPointer(node->_expr->type);
    break;
  }
  case '+': {
    if (!DataType::isNumeric(node->type->raw)) {
      type_error("Invalid operation + for type", node);
      node->type = DataType::build(RawDataType::ERROR);
    }
    node->type = node->_expr->type;
    break;
  }
  case '-': {
    if (!DataType::isNumeric(node->type->raw)) {
      type_error("Invalid operation - for type", node);
      node->type = DataType::build(RawDataType::ERROR);
    }
    node->type = node->_expr->type;
    break;
  }
  case '!':
    node->type = DataType::build(RawDataType::INT);
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

  if (node->_right->getNodeType() == NodeType::EXPR_CONSTANT &&
      node->type->raw != RawDataType::ERROR) {
    node->type = node->_left->type;
    node->_right->type = node->type;
  }
  if (node->_left->getNodeType() == NodeType::EXPR_CONSTANT &&
      node->type->raw != RawDataType::ERROR) {
    node->type = node->_right->type;
    node->_left->type = node->type;
  }

  if (node->_op == "=") {
    if (!node->var) {
      node->type = DataType::build(RawDataType::ERROR);
      return;
    }

    if (node->var->_constant)
      compile_error("Assignment to constant variable", node);
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
  node->structDef = resolveStruct(structName);
  if (!node->structDef) {
    name_error("Struct " + structName + " does not exist", node);
    return;
  }

  auto memberDef = node->structDef->membersDef.find(node->_memberName) !=
                           node->structDef->membersDef.end()
                       ? node->structDef->membersDef[node->_memberName]
                       : nullptr;
  auto funcDef = node->structDef->funcMembers.find(node->_memberName) !=
                         node->structDef->funcMembers.end()
                     ? node->structDef->funcMembers[node->_memberName]
                     : nullptr;

  if (!memberDef && !funcDef) {
    name_error("Member " + node->_memberName + " does not exist on struct " +
                   structName,
               node);
    return;
  }

  node->var = memberDef;
  node->func = funcDef;

  node->type =
      memberDef ? memberDef->type : DataType::build(RawDataType::FUNCTION);
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
  if (node->_index->type->size < 32)
    node->_index->type = DataType::build(RawDataType::INT);

  node->type = node->_inner->type->inner;
}

void SemanticValidator::visitExprCall(ExprCallNode *node) {
  if (node->_ref->getNodeType() == NodeType::VAR_REF) {
    auto funcName = ((ExprVarRefNode *)node->_ref)->_ident;
    if (funcName == "sizeof")
      return visitSizeOfCall(node);

    auto funcDef = resolveFunction(funcName);
    if (!funcDef) {
      name_error("Function " + funcName + " does not exists", node);
      return;
    }

    node->func = funcDef;
    node->type = funcDef->retType;
  } else if (node->_ref->getNodeType() == NodeType::MEMBER_ACCESS) {
    node->_ref->visit(this);
    if (node->_ref->type->raw == RawDataType::ERROR) {
      node->type = node->_ref->type;
      return;
    }

    auto memberNode = (ExprMemberAccess *)node->_ref;
    auto _struct = memberNode->_struct;

    if (!node->_ref->func) {
      name_error("Function " + memberNode->_memberName +
                     " does not exists on struct " + _struct->type->ident,
                 node);
      return;
    }

    auto refToStruct = new ExprUnaryNode(
        node->_filename, node->_line, node->_startCol, node, "&", 0, _struct);

    node->_args.insert(node->_args.begin(), refToStruct);
    node->func = node->_ref->func;
    node->type = node->func->retType;
  } else {
    compile_error("Calling non-function type", node);
  }

  if (node->_args.size() != node->func->_params.size()) {
    compile_error("Invalid argument count for function + " + node->func->_name,
                  node);
    return;
  }

  for (ulint i = 0; i < node->_args.size(); i++) {
    node->_args[i]->visit(this);
    auto castedType = DataType::getResultType(node->func->_params[i]->type, "=",
                                              node->_args[i]->type);
    bool validType = castedType->equals(node->func->_params[i]->type);
    if (!validType) {
      type_error("Invalid type for argument " + std::to_string(i) +
                     " in function call",
                 node);
      return;
    }
  }
}

void SemanticValidator::visitSizeOfCall(ExprCallNode *node) {
  if (node->_args.size() != 1) {
    node->type = DataType::build(RawDataType::ERROR);
    compile_error("sizeof must receive exactly one parameter", node);
  }
  if (node->_args[0]->getNodeType() != NodeType::VAR_REF) {
    node->type = DataType::build(RawDataType::ERROR);
    compile_error("sizeof parameter must be a type or a reference", node);
  }
  auto varRef = (ExprVarRefNode *)node->_args[0];
  auto var = resolveVar(varRef->_ident);
  auto type= var ? var->type
                      : DataType::build(TypeDefNode::build(
                            varRef->_ident, varRef->_ident, 0, 0));
  node->_ref->type = type;
  node->_ref->var = var;
  node->type = DataType::build(RawDataType::LONG);
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
  errors.push_back(node->_filename + ":" + std::to_string(node->_line) + ":" +
                   std::to_string(node->_startCol) + " compile error: " + msg);
}

void SemanticValidator::type_error(std::string msg, AstNode *node) {
  errors.push_back(node->_filename + ":" + std::to_string(node->_line) + ":" +
                   std::to_string(node->_startCol) + " type error: " + msg);
}

void SemanticValidator::name_error(std::string msg, AstNode *node) {
  errors.push_back(node->_filename + ":" + std::to_string(node->_line) + ":" +
                   std::to_string(node->_startCol) + " name error: " + msg);
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