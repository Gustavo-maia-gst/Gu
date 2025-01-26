#include "assembler.h"
#include <lld/Common/Driver.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Triple.h>
#include <ostream>
#include <vector>

using namespace llvm;

Assembler::Assembler(bool withEntrypoint) {
  this->withEntrypoint = withEntrypoint;

  TheContext = new LLVMContext();
  Builder = new llvm::IRBuilder<>(*TheContext);
  TheModule = std::make_unique<Module>("Program", *TheContext);

  rawTypeMapper = {
      {RawDataType::CHAR, Type::getInt8Ty(*TheContext)},
      {RawDataType::SHORT, Type::getInt16Ty(*TheContext)},
      {RawDataType::INT, Type::getInt32Ty(*TheContext)},
      {RawDataType::LONG, Type::getInt64Ty(*TheContext)},
      {RawDataType::FLOAT, Type::getFloatTy(*TheContext)},
      {RawDataType::DOUBLE, Type::getDoubleTy(*TheContext)},
      {RawDataType::VOID, Type::getVoidTy(*TheContext)},
  };

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  std::string Error;
  auto TargetTriple = LLVMGetDefaultTargetTriple();
  auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

  if (!Target) {
    errs() << "Not supported architecture: " << std::string(TargetTriple)
           << "\n";
    errs() << Error << "\n";
    exit(1);
  }

  TargetOptions opt;
  target = Target->createTargetMachine(TargetTriple, "generic", "", opt,
                                       Reloc::PIC_);
  TheModule->setDataLayout(target->createDataLayout());
  TheModule->setTargetTriple(TargetTriple);
}

std::stack<BasicBlock *> breakTo;
std::map<VarDefNode *, std::pair<Type *, Value *>> varContextMap;
std::map<std::string, StructType *> structTypeMap;
std::map<FunctionNode *, Function *> functionMap;

int funcCounter = 1;

void Assembler::printAssembled(std::string filename) {
  if (!compiled) {
    std::cerr << "Trying to print before assembling";
    return;
  }

  if (filename.empty()) {
    TheModule->print(llvm::outs(), nullptr);
  } else {
    std::error_code EC;
    llvm::raw_fd_ostream OutputStream(filename, EC, llvm::sys::fs::OF_None);

    if (EC) {
      llvm::errs() << "Error opening file: " << EC.message() << "\n";
      exit(1);
    }
    TheModule->print(OutputStream, nullptr);
  }
}

void Assembler::optimize(char optLevel) {
  if (!compiled) {
    std::cerr << "Trying to run optimizations before assembling";
    return;
  }

  PassBuilder passBuilder;
  ModulePassManager modulePassManager;

  // Configurar a pipeline de otimização
  FunctionAnalysisManager functionAnalysisManager;
  LoopAnalysisManager loopAnalysisManager;
  CGSCCAnalysisManager cgsccAnalysisManager;
  ModuleAnalysisManager moduleAnalysisManager;

  // Registrar os gerenciadores de análise
  passBuilder.registerModuleAnalyses(moduleAnalysisManager);
  passBuilder.registerCGSCCAnalyses(cgsccAnalysisManager);
  passBuilder.registerFunctionAnalyses(functionAnalysisManager);
  passBuilder.registerLoopAnalyses(loopAnalysisManager);
  passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager,
                                   cgsccAnalysisManager, moduleAnalysisManager);

  OptimizationLevel level;
  switch (optLevel) {
  case '0':
    level = OptimizationLevel::O0;
    break;
  case '1':
    level = OptimizationLevel::O1;
    break;
  case '2':
    level = OptimizationLevel::O2;
    break;
  case '3':
    level = llvm::OptimizationLevel::O3;
    break;
  default:
    std::cerr << "Invalid optLevel used\n";
    exit(1);
  }

  modulePassManager = passBuilder.buildPerModuleDefaultPipeline(level);

  // Rodar as otimizações no módulo
  modulePassManager.run(*TheModule, moduleAnalysisManager);
}

void Assembler::validateIR() {
  if (verifyModule(*TheModule, &errs())) {
    errs() << "Sorry, the code has been generated with errors, please report "
              "it including the source code at "
              "github: https://github.com/Gustavo-maia-gs/gu\n";
    exit(1);
  }
}

void Assembler::generateObject(std::string out, bool useAsm) {
  std::error_code err;
  raw_fd_ostream dest(out, err, sys::fs::OF_None);

  if (err) {
    errs() << "Could not open file: " << err.message() << '\n';
    exit(1);
  }
  legacy::PassManager pass;
  auto FileType =
      useAsm ? CodeGenFileType::AssemblyFile : CodeGenFileType::ObjectFile;

  if (target->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    errs() << "TargetMachine can't emit a file of this type";
    exit(1);
  }

  pass.run(*TheModule);
  dest.flush();
}

Value *Assembler::loadValue(ExprNode *node) {
  node->visit(this);
  if (node->type->raw == RawDataType::ARRAY)
    return current;

  std::set<NodeType> memoryAccessTypes = {
      NodeType::VAR_REF, NodeType::INDEX_ACCESS, NodeType::MEMBER_ACCESS};

  if (function && node->getNodeType() == NodeType::VAR_REF) {
    auto argDef = ((ExprVarRefNode *)node)->var;
    if (funcRegParams.find(argDef) != funcRegParams.end())
      return current; // Parameters are saved in registers instead of memory
  }

  if (memoryAccessTypes.find(node->getNodeType()) != memoryAccessTypes.end())
    return Builder->CreateLoad(getType(node->type), current);
  else
    return current;
}

Type *Assembler::getType(DataType *type) {
  if (!type->inner && type->raw != RawDataType::STRUCT) {
    auto mapped = rawTypeMapper[type->raw];
    if (!mapped)
      error("Invalid type " + type->ident);
    return mapped;
  }

  if (type->raw == RawDataType::STRUCT) {
    auto mappedStruct = structTypeMap[type->ident];
    if (!mappedStruct)
      error("Invalid struct " + type->ident);

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

Value *Assembler::getZero(DataType *type) {
  if (DataType::isFloat(type->raw))
    return ConstantFP::get(getType(type), 0);
  else
    return ConstantInt::get(getType(type), 0);
}

Value *Assembler::getOne(DataType *type) {
  if (DataType::isFloat(type->raw))
    return ConstantFP::get(getType(type), 1);
  else
    return ConstantInt::get(getType(type), 1);
}

Value *Assembler::getCast(Value *value, DataType *original, DataType *castTo) {
  if (original->equals(castTo))
    return value;
  if (DataType::isAddress(original->raw) && DataType::isAddress((castTo->raw)))
    return value;

  if (!DataType::isNumeric(original->raw) || !DataType::isNumeric(castTo->raw))
    error("Invalid cast");

  if (DataType::isInt(original->raw) && DataType::isInt((castTo->raw))) {
    if (original->size < castTo->size)
      return Builder->CreateSExt(value, getType(castTo));
    return Builder->CreateTrunc(value, getType(castTo));
  }

  if (DataType::isFloat(original->raw) && DataType::isFloat(castTo->raw)) {
    if (original->size < castTo->size)
      return Builder->CreateFPExt(value, getType(castTo));
    return Builder->CreateFPTrunc(value, getType(castTo));
  }

  error("Invalid cast");
  return nullptr;
}

Value *Assembler::getCondition(ExprNode *node, bool invert) {
  current = loadValue(node);
  Value *condition;

  auto zero = getZero(node->type);
  if (DataType::isFloat((node->type->raw))) {
    condition = invert ? Builder->CreateFCmpOEQ(current, zero)
                       : Builder->CreateFCmpUNE(current, zero);

  } else {
    condition = invert ? Builder->CreateICmpEQ(current, zero)
                       : Builder->CreateICmpNE(current, zero);
  }

  return condition;
}

void Assembler::visitProgram(ProgramNode *node) {
  program = node;

  node->visitChildren(this);

  if (withEntrypoint) {
    if (!main)
      error("Non-existing main function");
    auto exitFuncDef = node->funcs["sys_exit"];
    if (!exitFuncDef)
      error("libCDefiner has not been runned");
    auto exitFunc = functionMap[exitFuncDef];

    auto startType = FunctionType::get(Type::getVoidTy(*TheContext), {}, false);
    auto start = Function::Create(startType, Function::ExternalLinkage,
                                  "_start", *TheModule);
    auto startBlock = BasicBlock::Create(*TheContext, "startBlock", start);
    Builder->SetInsertPoint(startBlock);
    current = Builder->CreateCall(main, {}, "code");
    auto statusCode = getCast(current, node->funcs[MAIN_FUNC]->retType,
                              DataType::build(RawDataType::INT));
    Builder->CreateCall(exitFunc, {statusCode});
    Builder->CreateRetVoid();
  }

  compiled = true;
}

void Assembler::visitFunction(FunctionNode *node) {
  funcRegParams.clear();

  std::vector<Type *> paramTypes;
  for (auto param : node->_params) {
    paramTypes.push_back(getType(param->type));
    funcRegParams.insert(param);
  }

  auto retType = getType(node->retType);

  auto funcType = FunctionType::get(retType, paramTypes, false);

  auto funcName =
      node->_externName.empty()
          ? node->_name == MAIN_FUNC
                ? MAIN_FUNC
                : "func" + std::to_string(funcCounter++) + "_" + node->_name
          : node->_externName;
  auto func = Function::Create(funcType, Function::ExternalLinkage, funcName,
                               *TheModule);

  functionMap[node] = func;
  if (node->_external)
    return;

  function = func;

  if (funcName == MAIN_FUNC)
    main = func;

  for (ulint i = 0; i < node->_params.size(); i++) {
    auto arg = func->getArg(i);
    auto paramDef = node->_params[i];
    auto paramType = paramTypes[i];

    arg->setName(paramDef->_name);
    varContextMap[paramDef] = std::make_pair(paramType, arg);
  }

  auto block = BasicBlock::Create(*TheContext, node->_name, func);
  Builder->SetInsertPoint(block);

  for (auto &[varName, varDef] : node->localVars) {
    if (funcRegParams.find(varDef) != funcRegParams.end())
      continue;

    auto varType = getType(varDef->type);
    auto allocVar = Builder->CreateAlloca(varType, nullptr, varName);
    varContextMap[varDef] = std::make_pair(varType, allocVar);
  }

  node->_body->visit(this);
  if (node->retType->raw == RawDataType::VOID) {
    if (!outWithReturn)
      Builder->CreateRetVoid();
  }

  for (auto &[_, varDef] : node->localVars)
    varContextMap.erase(varDef);

  function = nullptr;
  funcRegParams.clear();
}

void Assembler::visitStructDef(StructDefNode *node) {
  if (!node->_genericArgNames.empty())
    return;

  auto structType = StructType::create(*TheContext, node->_name);
  structTypeMap[node->_name] = structType;

  std::vector<Type *> memberTypes;
  for (auto &[_, memberDef] : node->membersDef)
    memberTypes.push_back(getType(memberDef->type));
  structType->setBody(memberTypes);

  for (auto &[_, funcNode] : node->funcMembers) {
    funcNode->visit(this);
  }
}

void Assembler::visitBody(BodyNode *node) {
  outWithReturn = false;

  for (auto &statement : node->_statements) {
    statement->visit(this);
    if (statement->getNodeType() == NodeType::RETURN) {
      outWithReturn = true;
      break;
    };
  }
}

void Assembler::visitIf(IfNode *node) {
  auto ifBlock = BasicBlock::Create(*TheContext, "ifBlock", function);
  BasicBlock *endBlock = nullptr;
  auto elseBlock = BasicBlock::Create(
      *TheContext, node->_elseBody ? "elseBlock" : "endBlock", function);

  auto condition = getCondition(node->_expr);
  Builder->CreateCondBr(condition, ifBlock, elseBlock);

  Builder->SetInsertPoint(ifBlock);
  node->_ifBody->visit(this);
  if (!outWithReturn) {
    if (node->_elseBody) {
      endBlock = BasicBlock::Create(*TheContext, "endBlock", function);
      Builder->CreateBr(endBlock);
    } else
      Builder->CreateBr(elseBlock);
  }

  if (node->_elseBody) {
    Builder->SetInsertPoint(elseBlock);
    node->_elseBody->visit(this);
    if (!outWithReturn) {
      if (!endBlock)
        endBlock = BasicBlock::Create(*TheContext, "endBlock", function);

      Builder->CreateBr(endBlock);
    }
  }

  if (endBlock)
    Builder->SetInsertPoint(endBlock);
  else
    Builder->SetInsertPoint(elseBlock);
}

void Assembler::visitWhile(WhileNode *node) {
  auto testBlock = BasicBlock::Create(*TheContext, "testBlock", function);
  auto loopBlock = BasicBlock::Create(*TheContext, "loopBlock", function);
  auto endBlock = BasicBlock::Create(*TheContext, "endBlock", function);

  breakTo.push(endBlock);

  Builder->CreateBr(testBlock);

  Builder->SetInsertPoint(testBlock);

  auto condition = getCondition(node->_expr);
  Builder->CreateCondBr(condition, loopBlock, endBlock);

  Builder->SetInsertPoint(loopBlock);
  node->_body->visit(this);
  Builder->CreateBr(testBlock);

  Builder->SetInsertPoint(endBlock);

  breakTo.pop();
}

void Assembler::visitFor(ForNode *node) {
  auto testBlock = BasicBlock::Create(*TheContext, "testBlock", function);
  auto loopBlock = BasicBlock::Create(*TheContext, "loopBlock", function);
  auto endBlock = BasicBlock::Create(*TheContext, "endBlock", function);

  breakTo.push(endBlock);

  if (node->_start)
    node->_start->visit(this);

  Builder->CreateBr(testBlock);

  Builder->SetInsertPoint(testBlock);

  if (node->_cond) {
    auto condition = getCondition(node->_cond);
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

void Assembler::visitVarDef(VarDefNode *node) {
  if (!function) {
    Value *constant = nullptr;
    if (node->_defaultVal) {
      node->_defaultVal->visit(this);
      constant = current;
    }

    auto varType = getType((node->type));
    auto globalVar = new GlobalVariable(*TheModule, varType, node->_constant,
                                        GlobalValue::ExternalLinkage,
                                        (Constant *)constant, node->_name);

    varContextMap[node] = std::make_pair(varType, globalVar);
    return;
  }

  if (!node->_defaultVal) {
    if (!node->_initArgs.empty()) {
      auto structDef = program->structDefs[node->type->ident];
      auto initFunc = functionMap[structDef->funcMembers[INIT_FUNC]];
      std::vector<Value *> args;
      for (auto arg : node->_initArgs) {
        arg->visit(this);
        args.push_back(current);
      }

      Builder->CreateCall(initFunc, args);
    }

    return;
  }

  auto defaultVal = loadValue(node->_defaultVal);
  auto castedVal = getCast(defaultVal, node->_defaultVal->type, node->type);

  auto varPtr = varContextMap[node].second;
  if (!varPtr)
    error("Invalid varDef");

  Builder->CreateStore(castedVal, varPtr);
  return;
}

void Assembler::visitBreakNode(BreakNode *node) {
  auto endBlock = breakTo.top();
  Builder->CreateBr(endBlock);
}

void Assembler::visitReturnNode(ReturnNode *node) {
  if (node->_expr) {
    auto retVal = loadValue(node->_expr);
    current = getCast(retVal, node->_expr->type, node->retType);
    Builder->CreateRet(current);
    return;
  }

  Builder->CreateRetVoid();
}

void Assembler::visitMemberAccess(ExprMemberAccess *node) {
  node->_struct->visit(this);
  auto structType = structTypeMap[node->structDef->_name];
  auto offset = node->structDef->membersOffset[node->_memberName];

  current =
      Builder->CreateStructGEP(structType, current, offset, node->_memberName);
}

void Assembler::visitIndexAccess(ExprIndex *node) {
  auto array = loadValue(node->_inner);
  auto index = loadValue(node->_index);

  auto arrType = getType(node->_inner->type);

  // current = Builder->CreateGEP(arrType, array, {Builder->getInt32(0), index},
  //                              "index");
  current = Builder->CreateGEP(arrType, array, index, "index");
}

void Assembler::visitExprCall(ExprCallNode *node) {
  if (node->_ref->getNodeType() == NodeType::VAR_REF) {
    auto varRef = (ExprVarRefNode *)node->_ref;
    if (varRef->_ident == "sizeof") {
      return visitSizeof(node);
    }
  }

  std::vector<Value *> args;
  for (ulint i = 0; i < node->_args.size(); i++) {
    auto loadedValue = loadValue(node->_args[i]);
    auto casted = getCast(loadedValue, node->_args[i]->type,
                          node->func->_params[i]->type);
    args.push_back(casted);
  }

  auto func = functionMap[node->func];

  current = Builder->CreateCall(func, args, "call");
}

void Assembler::visitSizeof(ExprCallNode *node) {
  auto varRef = (ExprVarRefNode *)node->_ref;
  Type *type;
  if (varRef->var) {
    type = varContextMap[varRef->var].first;
  } else {
    type = structTypeMap[varRef->type->ident];
  }
  if (!type)
    error("Invalid sizeof");

  auto dataLayout = TheModule->getDataLayout();
  ulint size = dataLayout.getTypeAllocSize(type);
  current = ConstantInt::get(Type::getInt64Ty(*TheContext), size);
}

void Assembler::visitExprUnaryOp(ExprUnaryNode *node) {
  auto exprType = getType(node->_expr->type);
  Value *exprValue;

  switch (node->_op[0]) {
  case '-':
    exprValue = loadValue(node->_expr);
    current = DataType::isFloat(node->_expr->type->raw)
                  ? Builder->CreateFNeg(exprValue)
                  : Builder->CreateNeg(exprValue);
    break;
  case '+': {
    exprValue = loadValue(node->_expr);
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
    exprValue = loadValue(node->_expr);
    current = Builder->CreateLoad(exprType, exprValue, "derreferenced");
    break;
  case '&': {
    auto localVar =
        Builder->CreateAlloca(getType(node->type), nullptr, "address");
    node->_expr->visit(this);
    Builder->CreateStore(current, localVar);
    current = localVar;
    break;
  }
  case '!': {
    exprValue = loadValue(node->_expr);
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

void Assembler::visitExprVarRef(ExprVarRefNode *node) {
  auto varDef = node->var;
  auto var = varContextMap[varDef];
  if (!var.first || !var.second)
    error("Invalid varRef");

  current = var.second;
}

void Assembler::visitExprConstant(ExprConstantNode *node) {
  if (rawTypeMapper.find(node->type->raw) != rawTypeMapper.end()) {
    auto type = rawTypeMapper[node->type->raw];
    if (type->isFloatingPointTy()) {
      double value = std::stod(node->_rawValue);
      current = ConstantFP::get(type, value);
    } else {
      std::string strVal = "";
      for (char c : node->_rawValue) {
        if (c == '.')
          break;
        strVal += c;
      }
      long value = std::stoi(strVal);
      current = ConstantInt::get(type, value);
    }

    return;
  }

  if (node->type->raw == RawDataType::POINTER) {
    auto stringPtr = ConstantDataArray::getString(*TheContext, node->_rawValue);
    current =
        new GlobalVariable(*TheModule, stringPtr->getType(), true,
                           GlobalValue::ExternalLinkage, stringPtr, "string");
  } else if (node->type->raw == RawDataType::ARRAY) {
    current = ConstantDataArray::getString(*TheContext, node->_rawValue);
  }
}

void Assembler::visitExprBinaryOp(ExprBinaryNode *node) {
  static std::map<std::string,
                  std::function<void(ExprBinaryNode *, Value *, Value *)>>
      binaryOpHandlers = {
          {"+",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             if (DataType::isFloat(node->type->raw))
               current = Builder->CreateFAdd(left, right, "add");
             else
               current = Builder->CreateAdd(left, right);
           }},
          {"-",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             if (DataType::isFloat(node->type->raw))
               current = Builder->CreateFSub(left, right, "add");
             else
               current = Builder->CreateSub(left, right);
           }},
          {"*",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             if (DataType::isFloat(node->type->raw))
               current = Builder->CreateFMul(left, right, "add");
             else
               current = Builder->CreateMul(left, right);
           }},
          {"/",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             if (DataType::isFloat(node->type->raw))
               current = Builder->CreateFDiv(left, right, "add");
             else
               current = Builder->CreateSDiv(left, right);
           }},
          {"%",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             if (DataType::isFloat(node->type->raw))
               current = Builder->CreateFRem(left, right, "add");
             else
               current = Builder->CreateSRem(left, right);
           }},
          {">>",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             current = Builder->CreateAShr(left, right);
           }},
          {"<<",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             current = Builder->CreateShl(left, right);
           }},
          {">",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             auto zero = getZero(node->type);
             auto one = getOne(node->type);
             auto isGreater = DataType::isFloat(node->type->raw)
                                  ? Builder->CreateFCmpOGT(left, right)
                                  : Builder->CreateICmpSGT(left, right);
             current = Builder->CreateSelect(isGreater, one, zero);
           }},
          {"<",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             auto zero = getZero(node->type);
             auto one = getOne(node->type);
             auto isGreater = DataType::isFloat(node->type->raw)
                                  ? Builder->CreateFCmpOLT(left, right)
                                  : Builder->CreateICmpSLT(left, right);
             current = Builder->CreateSelect(isGreater, one, zero);
           }},
          {">=",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             auto zero = getZero(node->type);
             auto one = getOne(node->type);
             auto isGreater = DataType::isFloat(node->type->raw)
                                  ? Builder->CreateFCmpOGE(left, right)
                                  : Builder->CreateICmpSGE(left, right);
             current = Builder->CreateSelect(isGreater, one, zero);
           }},
          {"<=",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             auto zero = getZero(node->type);
             auto one = getOne(node->type);
             auto isGreater = DataType::isFloat(node->type->raw)
                                  ? Builder->CreateFCmpOLE(left, right)
                                  : Builder->CreateICmpSLE(left, right);
             current = Builder->CreateSelect(isGreater, one, zero);
           }},
          {"==",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             auto zero = getZero(node->type);
             auto one = getOne(node->type);
             auto isGreater = DataType::isFloat(node->type->raw)
                                  ? Builder->CreateFCmpOEQ(left, right)
                                  : Builder->CreateICmpEQ(left, right);
             current = Builder->CreateSelect(isGreater, one, zero);
           }},
          {"!=",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             auto zero = getZero(node->type);
             auto one = getOne(node->type);
             auto isGreater = DataType::isFloat(node->type->raw)
                                  ? Builder->CreateFCmpONE(left, right)
                                  : Builder->CreateICmpNE(left, right);
             current = Builder->CreateSelect(isGreater, one, zero);
           }},
          {"&&",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             auto zero = getZero(node->type);

             auto leftNonZero = DataType::isFloat(node->type->raw)
                                    ? Builder->CreateFCmpONE(left, zero)
                                    : Builder->CreateICmpNE(left, zero);
             auto rightNonZero = DataType::isFloat(node->type->raw)
                                     ? Builder->CreateFCmpONE(right, zero)
                                     : Builder->CreateICmpNE(right, zero);
             current = Builder->CreateAnd(leftNonZero, rightNonZero);
           }},
          {"||",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             auto zero = getZero(node->type);

             auto leftNonZero = DataType::isFloat(node->type->raw)
                                    ? Builder->CreateFCmpONE(left, zero)
                                    : Builder->CreateICmpNE(left, zero);
             auto rightNonZero = DataType::isFloat(node->type->raw)
                                     ? Builder->CreateFCmpONE(right, zero)
                                     : Builder->CreateICmpNE(right, zero);
             current = Builder->CreateOr(leftNonZero, rightNonZero);
           }},
          {"&",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             current = Builder->CreateAnd(left, right);
           }},
          {"|",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             current = Builder->CreateOr(left, right);
           }},
          {"^",
           [this](ExprBinaryNode *node, Value *left, Value *right) {
             current = Builder->CreateXor(left, right);
           }},
      };

  if (node->_op == "=") {
    auto rightVal = loadValue(node->_right);
    node->_left->visit(this);
    auto leftPtr = current;
    Builder->CreateStore(rightVal, leftPtr);
    current = rightVal;
    return;
  }

  auto leftVal = loadValue(node->_left);
  auto rightVal = loadValue(node->_right);

  auto leftCasted = getCast(leftVal, node->_left->type, node->type);
  auto rightCasted = getCast(rightVal, node->_right->type, node->type);

  auto handler = binaryOpHandlers[node->_op];
  handler(node, leftCasted, rightCasted);
}

void Assembler::error(std::string err) {
  std::cerr << "Assembler error: " << err
            << ". This is a compiler error, please report it at github: "
               "https://github.com/Gustavo-maia-gst"
            << std::endl;
  exit(1);
}