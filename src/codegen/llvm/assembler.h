#ifndef _assembler
#define _assembler

#include "../../ast/ast.h"
#include <elf.h>
#include <functional>
#include <lld/Common/CommonLinkerContext.h>
#include <lld/Common/Driver.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/CodeGen/CommandFlags.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/Scalar.h>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <utility>

class Assembler : public BaseVisitor {
public:
  Assembler(bool withEntrypoint);

  void optimize(char optLevel = '3');
  void printAssembled(std::string filename = "");
  void validateIR();
  void generateObject(std::string out, bool useAsm = false);

  void createExternReference(FunctionNode *node, std::string rawName);

  void visitProgram(ProgramNode *node);
  void visitFunction(FunctionNode *node);
  void visitIf(IfNode *node);
  void visitWhile(WhileNode *node);
  void visitFor(ForNode *node);
  void visitVarDef(VarDefNode *node);
  void visitStructDef(StructDefNode *node);
  void visitBreakNode(BreakNode *node);
  void visitReturnNode(ReturnNode *node);

  void visitExprBinaryOp(ExprBinaryNode *node);
  void visitMemberAccess(ExprMemberAccess *node);
  void visitIndexAccess(ExprIndex *node);
  void visitExprCall(ExprCallNode *node);
  void visitExprUnaryOp(ExprUnaryNode *node);

  void visitExprVarRef(ExprVarRefNode *node);
  void visitExprConstant(ExprConstantNode *node);

  void visitBody(BodyNode *node);
  void visitTypeDefNode(TypeDefNode *node) {}

private:
  llvm::LLVMContext *TheContext;
  llvm::IRBuilder<> *Builder;
  std::unique_ptr<llvm::Module> TheModule;
  std::map<RawDataType, llvm::Type *> rawTypeMapper;

  bool compiled = false;
  bool withEntrypoint;
  bool outWithReturn = false;

  llvm::Function *main = nullptr;
  llvm::Function *function = nullptr;
  std::set<VarDefNode *> funcRegParams;

  llvm::Type *getType(DataType *type);
  llvm::Value *loadValue(ExprNode *node);
  llvm::Value *getZero(DataType *type);
  llvm::Value *getOne(DataType *type);
  llvm::Value *getCast(llvm::Value *value, DataType *original,
                       DataType *castTo);
  llvm::Value *getCondition(ExprNode *node, bool invert = false);
  llvm::Value *current;

  void error(std::string msg);
};
#endif