#ifndef _parser_types
#include "types.cpp"
#endif
#ifndef _scanner
#include "../lexicalScanner/LexicalScanner.h"
#endif
#define _parser

class AstParser
{
public:
    AstParser(LexicalScanner *sc);
    ~AstParser();
    Node::ProgramNode *parseProgram();
    Node::ExprNode *parseExpr(int level = Order::MAX_PRECEDENCE_LEVEL);

private:
    LexicalScanner *sc;
    Node::ProgramNode *context; 

    Node::BodyDef *parseBody();
    Node::StatementDef *parseStatement();
    Node::IfDef *parseIf();
    Node::ForDef *parseFor();
    Node::WhileDef *parseWhile();
    Node::FunctionDef *parseFunctionDef();
    Node::AssignDef *parseAssign();
    Node::VariableDef *parseVarDef();
    Node::AtomNode *parseAtom();
};