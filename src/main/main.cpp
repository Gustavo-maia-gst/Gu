#include "../codegen/llvm/assembler.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../semantic/libcDefiner.h"
#include "../semantic/validator.h"
#include "argHandler.h"
#include <string>

ProgramNode *getProgramAst(std::string filename) {
  auto lexer = Lexer::fromFile(filename);
  AstParser parser(lexer);
  return parser.parseProgram();
}

void runValidator(ProgramNode *programAst, SemanticValidator *validator,
                  LibCDefiner *libCDefiner) {
  libCDefiner->addPosixSyscallsDefs(programAst);
  validator->visitProgram(programAst);
  auto errors = validator->getErrors();
  if (errors.size()) {
    std::cerr << "There are errors in the source code";
    for (auto err : errors)
      std::cerr << err << '\n';
    exit(1);
  }
}

void runAssembler(ProgramNode *programAst, Assembler *assembler,
                  LibCDefiner *libCDefiner) {
  libCDefiner->addPosixSyscallsIRReferences(programAst, assembler);
  assembler->visitProgram(programAst);
}

void compile(Assembler *assembler, std::string out, char optLevel,
             std::string asmType) {
  if (asmType != "basicIR")
    assembler->optimize(optLevel);

  if (asmType == "obj")
    assembler->generateObject(out);
  else if (asmType == "asm")
    assembler->generateObject(out, true);
  else
    assembler->printAssembled(out);
}

int main(int argc, char **argv) {
  ArgHandler argHandler{};
  argHandler.defArg("assembly", {"asm", "", "basicIR", "IR"}, "S");
  argHandler.defArg("output", {}, "o");
  argHandler.defArg("compile", {""}, "c");
  argHandler.defArg("opt", {"0", "1", "2", "3"}, "O");

  argHandler.parseArgs(argc, argv);

  auto [asmpresent, assemblyType] = argHandler.getArg("assembly");
  auto [opresent, ouputName] = argHandler.getArg("output");
  auto [cpresent, _] = argHandler.getArg("compile");
  auto [optpresent, optValue] = argHandler.getArg("opt");
  if (!optpresent)
    optValue = "3";

  auto filenames = argHandler.getPosArgs();
  if (filenames.size() != 1)
    argHandler.parseError("Expecting one file, got " +
                          std::to_string(filenames.size()));
  auto filename = filenames[0];

  SemanticValidator validator(cpresent);
  LibCDefiner libCDefiner{};
  Assembler assembler(!cpresent);

  auto programAst = getProgramAst(filename);
  runValidator(programAst, &validator, &libCDefiner);
  runAssembler(programAst, &assembler, &libCDefiner);
  compile(&assembler, opresent ? ouputName : "a.o",
          optpresent ? optValue[0] : '3', asmpresent ? assemblyType : "obj");

  return 0;
}