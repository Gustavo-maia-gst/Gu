#include "IRGenerator.h"
#include <functional>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>

using namespace llvm;

std::map<RawDataType, Type *> rawTypeMapper = {
    {RawDataType::CHAR, Type::getInt8Ty(*TheContext)},
    {RawDataType::SHORT, Type::getInt16Ty(*TheContext)},
    {RawDataType::INT, Type::getInt32Ty(*TheContext)},
    {RawDataType::LONG, Type::getInt64Ty(*TheContext)},
    {RawDataType::FLOAT, Type::getFloatTy(*TheContext)},
    {RawDataType::DOUBLE, Type::getDoubleTy(*TheContext)},
};

std::stack<BasicBlock *> breakTo;
std::map<VarDefNode *, std::pair<Type *, Value *>> varContextMap;
std::map<std::string, StructType *> structTypeMap;
std::map<FunctionNode *, Function *> functionMap;

int funcCounter = 1;

Type *IRGenerator::getType(DataType *type) {
  if (!type->inner && type->raw != RawDataType::STRUCT) {
    auto mapped = rawTypeMapper[type->raw];
    if (!mapped)
      error("Invalid type " + type->ident);
    return mapped;
  }

  if (type->raw == RawDataType::STRUCT) {
    auto mappedStruct = structTypeMap[type->ident];
    if (!mappedStruct)
      error("Invalid struct" + type->ident);

    return mappedStruct;
  }

  auto innerType = getType(type->inner);
  if (type->raw == RawDataType::POINTER) {
    auto pointerType = PointerType::get(innerType, 0);
    return pointerType;
  }
  if (type->raw == RawDataType::ARRAY) {
    auto arrayType = ArrayType::get(innerType, type->arrLength);
    return arrayType;
  }

  error("Invalid type");
  return nullptr;
}

Value *IRGenerator::getZero(DataType *type) {
  if (DataType::isFloat(type->raw))
    return ConstantFP::get(getType(type), 0);
  else
    return ConstantInt::get(getType(type), 0);
}

Value *IRGenerator::getCondition(ExprNode *node, bool invert) {
  node->visit(this);
  Value *condition;

  auto zero = getZero(node->type);
  if (DataType::isFloat((node->type->raw))) {
    condition = invert ? Builder->CreateFCmpOEQ(current, zero)
                       : Builder->CreateFCmpUNE(current, current);

  } else {
    condition = invert ? Builder->CreateICmpEQ(current, zero)
                       : Builder->CreateICmpNE(current, zero);
  }

  return condition;
}

void IRGenerator::visitProgram(ProgramNode *node) { node->visitChildren(this); }

void IRGenerator::visitFunction(FunctionNode *node) {
  std::vector<Type *> paramTypes;
  for (auto param : node->_params)
    paramTypes.push_back(getType(param->type));

  auto retType = getType(node->retType);

  auto funcType = FunctionType::get(retType, paramTypes, false);

  auto func = Function::Create(
      funcType, Function::ExternalLinkage,
      "func" + std::to_string(funcCounter++) + "_" + node->_name, *TheModule);

  functionMap[node] = func;
  function = func;

  for (auto [_, varDef] : node->localVars) {
    auto varType = getType(varDef->type);
    auto allocVar = Builder->CreateAlloca(varType, nullptr, varDef->_name);
    varContextMap[varDef] = std::make_pair(varType, allocVar);
  }

  node->_body->visit(this);
  if (node->retType->raw == RawDataType::VOID)
    Builder->CreateRetVoid();

  for (auto [_, varDef] : node->localVars)
    varContextMap.erase(varDef);

  function = nullptr;
}

void IRGenerator::visitStructDef(StructDefNode *node) {
  auto structType = StructType::create(*TheContext, node->_name);
  structTypeMap[node->_name] = structType;

  std::vector<Type *> memberTypes;
  for (auto [_, memberDef] : node->membersDef)
    memberTypes.push_back(getType(memberDef->type));
  structType->setBody(memberTypes);

  for (auto [_, funcNode] : node->funcMembers) {
    funcNode->visit(this);
  }
}

void IRGenerator::visitIf(IfNode *node) {
  auto ifBlock = BasicBlock::Create(*TheContext, "ifBlock", function);
  auto endBlock = BasicBlock::Create(*TheContext, "endBlock", function);
  auto elseBlock = node->_elseBody
                       ? BasicBlock::Create(*TheContext, "ifBlock", function)
                       : endBlock;

  auto condition = getCondition(node->_expr);
  Builder->CreateCondBr(condition, ifBlock, elseBlock);
  Builder->SetInsertPoint(ifBlock);
  node->_ifBody->visit(this);
  Builder->CreateBr(endBlock);

  if (node->_elseBody) {
    Builder->SetInsertPoint(elseBlock);
    node->_elseBody->visit(this);
    Builder->CreateBr(endBlock);
  }

  Builder->SetInsertPoint(endBlock);
}

void IRGenerator::visitWhile(WhileNode *node) {
  auto testBlock = BasicBlock::Create(*TheContext, "testBlock", function);
  auto loopBlock = BasicBlock::Create(*TheContext, "loopBlock", function);
  auto endBlock = BasicBlock::Create(*TheContext, "endBlock", function);

  breakTo.push(endBlock);

  Builder->SetInsertPoint(testBlock);

  auto condition = getCondition(node->_expr, true);
  Builder->CreateCondBr(condition, loopBlock, endBlock);

  Builder->SetInsertPoint(loopBlock);
  node->_body->visit(this);
  Builder->CreateBr(testBlock);

  Builder->SetInsertPoint(endBlock);

  breakTo.pop();
}

void IRGenerator::visitFor(ForNode *node) {
  auto testBlock = BasicBlock::Create(*TheContext, "testBlock", function);
  auto loopBlock = BasicBlock::Create(*TheContext, "loopBlock", function);
  auto endBlock = BasicBlock::Create(*TheContext, "endBlock", function);

  breakTo.push(endBlock);

  if (node->_start)
    node->_start->visit(this);

  Builder->SetInsertPoint(testBlock);

  if (node->_cond) {
    auto condition = getCondition(node->_cond, true);
    Builder->CreateCondBr(condition, loopBlock, endBlock);
  } else
    Builder->CreateBr(loopBlock);

  Builder->SetInsertPoint(loopBlock);
  node->_body->visit(this);
  if (node->_inc)
    node->_inc->visit(this);
  Builder->CreateBr(testBlock);

  Builder->SetInsertPoint(endBlock);

  breakTo.pop();
}

void IRGenerator::visitVarDef(VarDefNode *node) {
  if (function) {
    if (!node->_defaultVal)
      return;

    node->_defaultVal->visit(this);
    auto defaultVal = current;

    auto varPtr = varContextMap[node].second;
    if (!varPtr)
      error("Invalid varDef");

    auto loadedValue = Builder->CreateStore(defaultVal, varPtr);
    return;
  }

  Value *constant = nullptr;
  if (node->_defaultVal) {
    node->_defaultVal->visit(this);
    constant = current;
  }

  auto varType = getType((node->type));
  auto globalVar =
      new GlobalVariable(varType, node->_constant, GlobalValue::ExternalLinkage,
                         (Constant *)constant, node->_name);

  varContextMap[node] = std::make_pair(varType, globalVar);
}

void IRGenerator::visitBreakNode(BreakNode *node) {
  auto endBlock = breakTo.top();
  Builder->CreateBr(endBlock);
}

void IRGenerator::visitReturnNode(ReturnNode *node) {
  current = nullptr;
  if (node->_expr)
    node->_expr->visit(this);

  if (current)
    Builder->CreateRet(current);
  else
    Builder->CreateRetVoid();
}


void IRGenerator::visitMemberAccess(ExprMemberAccess *node) {
  node->_struct->visit(this);
  auto structType = structTypeMap[node->structDef->_name];
  auto offset = node->structDef->membersOffset[node->_memberName];
  current = Builder->CreateStructGEP(structType, current, offset, "struct access");
}

void IRGenerator::visitIndexAccess(ExprIndex *node) {
  node->_inner->visit(this);
  auto array = current;
  node->_index->visit(this);
  auto index = current;

  auto arrType = getType(node->_inner->type->inner);

  current = Builder->CreateGEP(arrType, array, index, "Index");
}

void IRGenerator::visitExprCall(ExprCallNode *node) {
  std::vector<Value *> args;
  for (auto arg : node->_args) {
    arg->visit(this);
    args.push_back(current);
  }

  auto func = functionMap[node->func];

  Builder->CreateCall(func, args, "call");
}

void IRGenerator::visitExprUnaryOp(ExprUnaryNode *node) {
  node->_expr->visit(this);
  auto exprType = getType(node->_expr->type);
  auto exprValue = current;

  switch (node->_op[0]) {
  case '-':
    current = DataType::isFloat(node->_expr->type->raw)
                  ? Builder->CreateFNeg(exprValue)
                  : Builder->CreateNeg(exprValue);
    break;
  case '+': {
    auto zero = getZero(node->_expr->type);

    auto isNegative = DataType::isFloat(node->_expr->type->raw)
                          ? Builder->CreateFCmpOLT(exprValue, zero)
                          : Builder->CreateICmpSLT(exprValue, zero);
    auto negated = DataType::isFloat(node->_expr->type->raw)
                       ? Builder->CreateFNeg(exprValue)
                       : Builder->CreateNeg(exprValue);
    current =
        Builder->CreateSelect(isNegative, negated, exprValue, "absolute value");
    break;
  }
  case '*':
    current = Builder->CreateLoad(exprType, exprValue, "derreferenced");
    break;
  case '&':
    current = exprValue;
    break;
  case '!': {
    auto zero = getZero(node->_expr->type);

    auto one32 = Builder->getInt32(1);
    auto zero32 = Builder->getInt32(0);

    auto isZero = DataType::isFloat(node->_expr->type->raw)
                      ? Builder->CreateFCmpOEQ(exprValue, zero)
                      : Builder->CreateICmpEQ(exprValue, zero);

    current = Builder->CreateSelect(isZero, one32, zero32);
    break;
  }
  }
}

void IRGenerator::visitExprVarRef(ExprVarRefNode *node) {
  auto varDef = node->var;
  auto var = varContextMap[varDef];

  if (!var.second)
    error("Invalid varRef");

  current = Builder->CreateLoad(var.first, var.second, node->_ident);
}

void IRGenerator::visitExprConstant(ExprConstantNode *node) {
  if (rawTypeMapper.find(node->type->raw) != rawTypeMapper.end()) {
    auto type = rawTypeMapper[node->type->raw];
    if (type->isFloatingPointTy())
      current = ConstantInt::get(type, std::stoi(node->_rawValue));
    else
      current = ConstantFP::get(type, std::stod(node->_rawValue));

    return;
  }

  if (node->type->raw == RawDataType::ARRAY) {
    current = ConstantDataArray::getString(*TheContext, node->_rawValue);
  }
}

std::map<std::string, std::function<void(ExprBinaryNode *)>> binaryOpHandlers = {
  { "+", [](ExprBinaryNode *node) {

  } },
  { "-", [](ExprBinaryNode *node) {

  } },
  { "*", [](ExprBinaryNode *node) {

  } },
  { "/", [](ExprBinaryNode *node) {

  } },
  { "%", [](ExprBinaryNode *node) {

  } },
  { ">>", [](ExprBinaryNode *node) {

  } },
  { "<<", [](ExprBinaryNode *node) {

  } },
  { ">", [](ExprBinaryNode *node) {

  } },
  { "<", [](ExprBinaryNode *node) {

  } },
  { ">=", [](ExprBinaryNode *node) {

  } },
  { "<=", [](ExprBinaryNode *node) {

  } },
  { "==", [](ExprBinaryNode *node) {

  } },
  { "&&", [](ExprBinaryNode *node) {

  } },
  { "||", [](ExprBinaryNode *node) {

  } },
  { "&", [](ExprBinaryNode *node) {

  } },
  { "|", [](ExprBinaryNode *node) {

  } },
  { "^", [](ExprBinaryNode *node) {

  } },
  { "+", [](ExprBinaryNode *node) {

  } },
};

void IRGenerator::visitExprBinaryOp(ExprBinaryNode *node) {
}