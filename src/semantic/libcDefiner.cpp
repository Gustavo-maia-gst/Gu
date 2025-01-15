#include "libcDefiner.h"

LibCDefiner::LibCDefiner() {
  syscalls = {
      // FILE MANAGEMENT
      {"open",
       {
           buildParam("pathNamePtr", DataType::buildPointer(
                                         DataType::build(RawDataType::CHAR))),

           buildParam("flags", DataType::build(RawDataType::INT)),
           buildParam("mode_t", DataType::build(RawDataType::INT)),
       },
       DataType::build(RawDataType::INT)},
      {"close",
       {
           buildParam("fd", DataType::build(RawDataType::INT)),
       },
       DataType::build(RawDataType::INT)},
      {"read",
       {

           buildParam("fd", DataType::build(RawDataType::INT)),
           buildParam("buff", DataType::buildPointer(
                                  DataType::build(RawDataType::CHAR))),

           buildParam("count", DataType::build(RawDataType::LONG)),
       },
       DataType::build(RawDataType::LONG)},
      {"write",
       {

           buildParam("fd", DataType::build(RawDataType::INT)),
           buildParam("buff", DataType::buildPointer(
                                  DataType::build(RawDataType::CHAR))),

           buildParam("count", DataType::build(RawDataType::LONG)),
       },
       DataType::build(RawDataType::LONG)},
      {"lseek",
       {

           buildParam("fd", DataType::build(RawDataType::INT)),
           buildParam("offset", DataType::build(RawDataType::LONG)),
           buildParam("whence", DataType::build(RawDataType::INT)),
       },
       DataType::build(RawDataType::LONG)},
      {"unlink",
       {
           buildParam("pathnamePtr", DataType::buildPointer(
                                         DataType::build(RawDataType::CHAR))),

       },
       DataType::build(RawDataType::INT)},
      {"rename",
       {
           buildParam("oldNamePtr", DataType::buildPointer(
                                        DataType::build(RawDataType::CHAR))),
           buildParam("newNamePtr", DataType::buildPointer(
                                        DataType::build(RawDataType::CHAR))),

       },
       DataType::build(RawDataType::INT)},
      {"mkdir",
       {
           buildParam("pathNamePtr", DataType::buildPointer(
                                         DataType::build(RawDataType::CHAR))),
           buildParam("newNamePtr", DataType::build(RawDataType::INT)),

       },
       DataType::build(RawDataType::INT)},
      {"rmdir",
       {
           buildParam("pathNamePtr", DataType::buildPointer(
                                         DataType::build(RawDataType::CHAR))),
       },
       DataType::build(RawDataType::INT)},

      // PROCESS MANAGEMENT
      {"fork", {}, DataType::build(RawDataType::INT)},
      {"exit",
       {
           buildParam("status", DataType::build(RawDataType::INT)),
       },
       DataType::build(RawDataType::VOID)},
      {"getpid", {}, DataType::build(RawDataType::INT)},
      {"kill",
       {
           buildParam("pid", DataType::build(RawDataType::INT)),
           buildParam("sig", DataType::build(RawDataType::INT)),
       },
       DataType::build(RawDataType::INT)},

      // MEMORY MANAGEMENT
      {"mmap",
       {
           buildParam("addr", DataType::buildPointer(
                                  DataType::build(RawDataType::CHAR))),
           buildParam("length", DataType::build(RawDataType::LONG)),
           buildParam("prot", DataType::build(RawDataType::INT)),
           buildParam("flags", DataType::build(RawDataType::INT)),
           buildParam("fd", DataType::build(RawDataType::INT)),
           buildParam("offset", DataType::build(RawDataType::LONG)),
       },
       DataType::buildPointer(DataType::build(RawDataType::CHAR))},
      {"munmap",
       {
           buildParam("addr", DataType::buildPointer(
                                  DataType::build(RawDataType::CHAR))),
           buildParam("length", DataType::build(RawDataType::LONG)),
       },
       DataType::build(RawDataType::INT)},

      // IPC
      {"pipe",
       {
           buildParam("addr", DataType::buildPointer(
                                  DataType::build(RawDataType::INT))),
       },
       DataType::build(RawDataType::INT)},
      {"socket",
       {
           buildParam("domain", DataType::build(RawDataType::INT)),
           buildParam("type", DataType::build(RawDataType::INT)),
           buildParam("protocol", DataType::build(RawDataType::INT)),
       },
       DataType::build(RawDataType::INT)},
      {"connect",
       {
           buildParam("sockfd", DataType::build(RawDataType::INT)),
           buildParam("addr", DataType::buildPointer(
                                  DataType::build(RawDataType::CHAR))),
           buildParam("addrlen", DataType::build(RawDataType::INT)),
       },
       DataType::build(RawDataType::INT)},
      {"accept",
       {
           buildParam("sockfd", DataType::build(RawDataType::INT)),
           buildParam("addr", DataType::buildPointer(
                                  DataType::build(RawDataType::CHAR))),
           buildParam("addrlen", DataType::buildPointer(
                                     DataType::build(RawDataType::LONG))),
       },
       DataType::build(RawDataType::INT)},
      {"send",
       {
           buildParam("sockfd", DataType::build(RawDataType::INT)),
           buildParam("buff", DataType::buildPointer(
                                  DataType::build(RawDataType::CHAR))),
           buildParam("length", DataType::build(RawDataType::LONG)),
           buildParam("flags", DataType::build(RawDataType::INT)),
       },
       DataType::build(RawDataType::LONG)},
      {"recv",
       {
           buildParam("sockfd", DataType::build(RawDataType::INT)),
           buildParam("buff", DataType::buildPointer(
                                  DataType::build(RawDataType::CHAR))),
           buildParam("length", DataType::build(RawDataType::LONG)),
           buildParam("flags", DataType::build(RawDataType::INT)),
       },
       DataType::build(RawDataType::LONG)},
  };
}

std::string getSyscallRealName(std::string syscall) { return "sys_" + syscall; }

void LibCDefiner::addPosixSyscallsDefs(ProgramNode *node) {
  for (auto &syscall : syscalls) {
    auto realFuncName = getSyscallRealName(syscall.name);
    auto funcDef = buildFunc(realFuncName, syscall.args, syscall.retType, node);

    node->_children.push_back(funcDef);
    funcDef->_parent = node;
    funcDef->_external = true;
  }
}

FunctionNode *LibCDefiner::buildFunc(std::string &name,
                                     std::vector<VarDefNode *> &params,
                                     DataType *retType, AstNode *parent) {
  std::string filename = "";
  auto node = new FunctionNode(filename, 0, 0, nullptr, name);
  for (auto param : params) {
    node->_params.push_back(param);
    param->_parent = node;
    node->_children.push_back(param);
  }
  node->retType = retType;
  return node;
}

VarDefNode *LibCDefiner::buildParam(std::string name, DataType *type) {
  std::string filename = "";
  auto node = new VarDefNode(filename, 0, 0, nullptr, name);
  node->type = type;
  return node;
}