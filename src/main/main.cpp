#include "../codegen/llvm/assembler.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../parser/processors/importManager.h"
#include "../parser/processors/templates.h"
#include "../semantic/validator.h"
#include "argHandler.h"
#include <cstdlib>
#include <string>

ProgramNode *getProgramAst(std::string filename, std::vector<std::string> &filenames) {
  ImportManager importManager;
  AstCloner astCloner;
  TemplatesVisitor genericVisitor(&astCloner);

  auto lexer = Lexer::fromFile(filename);
  AstParser parser(lexer);

  auto program = parser.parseProgram();
  importManager.processImports(program);
  genericVisitor.visitProgram(program);

  for (auto file: importManager.getImportedFiles())
    filenames.push_back(file);

  return program;
}

std::string getLoaderSuffix() {
#if defined(__APPLE__)
  return "-lSystem";
#elif defined(__x86_64__)
  return "-lc -dynamic-linker /lib64/ld-linux-x86-64.so.2";
#elif defined(__i386__)
  return "-lc -dynamic-linker /lib/ld-linux.so.2";
#elif defined(__arm__)
#elif defined(__aarch64__)
  return "-lc -dynamic-linker /lib/ld-linux-aarch64.so.1"; // Para ARM 64-bit
#else
  std::cerr << "Not supported architecture\n";
  exit(1);
#endif
}

void runValidator(ProgramNode *programAst, SemanticValidator *validator) {

  validator->visitProgram(programAst);
  auto errors = validator->getErrors();
  if (errors.size()) {
    std::cerr << "There are errors in the source code:\n\n";
    for (auto err : errors)
      std::cerr << err << '\n';
    exit(1);
  }
}

void runAssembler(ProgramNode *programAst, Assembler *assembler) {
  assembler->visitProgram(programAst);
}

void compile(Assembler *assembler, std::string out, char optLevel,
             std::string asmType) {
  if (asmType != "basicIR")
    assembler->validateIR();

  if (asmType != "basicIR")
    assembler->optimize(optLevel);

  if (asmType == "obj" || asmType == "exec")
    assembler->generateObject(out);
  else if (asmType == "asm")
    assembler->generateObject(out, true);
  else
    assembler->printAssembled(out);
}

int main(int argc, char **argv) {
  ArgHandler argHandler{};
  argHandler.defArg("assembly", {"asm", "obj", "basicIR", "IR"}, "S");
  argHandler.defArg("output", {}, "o");
  argHandler.defArg("compile", {""}, "c", true);
  argHandler.defArg("opt", {"0", "1", "2", "3"}, "O");

  argHandler.parseArgs(argc, argv);

  auto [asmpresent, asmType] = argHandler.getArg("assembly");
  auto [opresent, outputName] = argHandler.getArg("output");
  auto [cpresent, _] = argHandler.getArg("compile");
  auto [optpresent, optValue] = argHandler.getArg("opt");

  if (!asmpresent)
    asmType = "obj";
  if (asmType == "obj" && !cpresent)
    asmType = "exec";

  auto filenames = argHandler.getPosArgs();
  if (filenames.empty())
    argHandler.parseError(
        "Expecting at least one file and possibly some object files");
  auto filename = filenames[0];

  SemanticValidator validator(!cpresent);
  Assembler assembler(!cpresent);

  auto programAst = getProgramAst(filename, filenames);
  runValidator(programAst, &validator);
  runAssembler(programAst, &assembler);

  bool toBinary = asmType == "exec";
  std::string outName = opresent ? outputName : "a.o";

  compile(&assembler, toBinary ? "a.o" : outName,
          optpresent ? optValue[0] : '2', asmType);

  if (toBinary) {
    std::string ldCommand = "ld a.o -o " + outName + " ";
    for (ulint i = 1; i < filenames.size(); i++)
      ldCommand += " " + filenames[i] + " ";
    ldCommand += getLoaderSuffix();
    int result = system(ldCommand.c_str());
    system("rm a.o");
    if (result) {
      std::cerr << "There are errors during the linking phase\n";
      exit(1);
    }
  }

  return 0;
}