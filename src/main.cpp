#include "astParser/AstParser.h"

int main(int argc, char **argv)
{
    char *filename = "src/teste";
    auto sc = new LexicalScanner(filename);
    auto parser = new AstParser(sc);
    parser->parseExpr();
}