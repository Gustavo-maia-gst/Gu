#include "astParser/AstParser.h"

int main(int argc, char **argv)
{
    std::string filename = "src/teste";
    auto sc = new LexicalScanner(filename);
    auto parser = new AstParser(sc);
    auto tree = parser->parseProgram();
    std::cout << std::to_string(tree) << std::endl;
    return 0;
}