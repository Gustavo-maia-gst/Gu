#include "codegen/llvm/assembler.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/libcDefiner.h"
#include "semantic/validator.h"

int main() {
  std::string filename = "/home/gustavo/Projects/gu/src/teste";
  auto sc = Lexer::fromFile(filename);
  auto parser = new AstParser(sc);
  auto program = parser->parseProgram();

  auto libCDefiner = new LibCDefiner();

  libCDefiner->addPosixSyscallsDefs(program);

  auto validator = new SemanticValidator;
  program->visit(validator);

  auto semanticErros = validator->getErrors();
  if (!semanticErros.empty()) {
    std::cerr << "There are errors in the source code" << std::endl;

    for (auto err : semanticErros)
      std::cerr << err << std::endl;

    exit(1);
  }

  auto assembler = new Assembler();

  libCDefiner->addPosixSyscallsIRReferences(program, assembler);

  program->visit(assembler);
  delete program;

  assembler->printAssembled();

  assembler->validateIR();

  assembler->optimize();

  assembler->generateObject("teste.o");

  return 0;
}