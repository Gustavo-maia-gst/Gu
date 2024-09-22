#ifndef _parser_types
#include "types.cpp"
#endif
#ifndef _scanner
#include "../lexicalScanner/LexicalScanner.h"
#endif
#define _parser

class AstParser {
public:
    AstParser(LexicalScanner* sc);
    ~AstParser();
    Node::ProgramNode* parseProgram();

private:
    LexicalScanner* sc;

    Node::BodyDef* parseBody();
    Node::StatementDef* parseStatement();
    Node::IfDef* parseIf();
    Node::ForDef* parseFor();
    Node::WhileDef* parseWhile();
    Node::FunctionDef* parseFunctionDef();
    Node::AssignDef* parseAssign();
    Node::ExprNode* parseExpr();
    Node::VariableDef* parseVarDef();
};