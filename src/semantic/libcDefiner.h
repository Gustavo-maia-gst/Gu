#ifndef _libcDefiner
#define _libcDefiner

#include "../codegen/llvm/assembler.h"

struct SyscallDef {
  std::string name;
  std::vector<VarDefNode *> args;
  DataType *retType;

  SyscallDef(const std::string &n, const std::vector<VarDefNode *> &a,
             DataType *ret)
      : name(n), args(a), retType(ret) {}
};

class LibCDefiner {
public:
  LibCDefiner();
  void addPosixSyscallsDefs(ProgramNode *node);
  void addPosixSyscallsIRReferences(ProgramNode *node, Assembler *assembler);

private:
  std::vector<SyscallDef> syscalls;
  VarDefNode *buildParam(std::string name, DataType *type);
  FunctionNode *buildFunc(std::string &name, std::vector<VarDefNode *> &params,
                          DataType *retType, AstNode *parent);
};
#endif