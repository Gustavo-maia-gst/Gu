#include "lexer/lexer.h"
#include "parser/parser.h"

int main() {
    std::string filename = "/home/gustavo/Projects/gu/src/teste";
    auto sc = Lexer::fromFile(filename);
    auto parser = new AstParser(sc);
    auto program = parser->parseProgram();
    return 0;
}