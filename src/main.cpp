#include "astParser/AstParser.h"

int main(int argc, char **argv)
{
    if (argc < 2) compile_error("No file provided");

    char* filename = argv[1];
    auto sc = new LexicalScanner(filename);
    auto parser = new AstParser(sc);
    parser->parseExpr();
}