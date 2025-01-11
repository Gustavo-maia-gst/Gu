#include "../../parser/ast/ast.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <stack>
#include <string>
#include <utility>

class IRGenerator : public BaseVisitor {
public:
  void print();

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

  void visitTypeDefNode(TypeDefNode *node) {}
  void visitBody(BlockNode *node) { node->visitChildren(this); }

private:
  llvm::Function *function = nullptr;

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