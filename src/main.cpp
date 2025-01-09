#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/validator.h"
#include "codegen/llvm/IRGenerator.h"

int main() {
  std::string filename = "/home/gustavo/Projects/gu/src/teste";
  auto sc = Lexer::fromFile(filename);
  auto parser = new AstParser(sc);
  auto program = parser->parseProgram();

  auto validator = new SemanticValidator;
  program->visit(validator);
  auto errors = validator->getErrors();
  if (!errors.empty()) {
    std::cerr << "There are errors in the source code" << std::endl;

    for (auto err : errors)
      std::cerr << err << std::endl;

    exit(1);
  }

  auto irGen = new IRGenerator();
  program->visit(irGen);

  return 0;
}