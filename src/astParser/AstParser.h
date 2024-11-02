#ifndef _parser
#define _parser
#include "types.cpp"
#include "../lexicalScanner/LexicalScanner.h"

namespace Order
{
    extern int MAX_PRECEDENCE_LEVEL;
}

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
    Node::VariableNode *parseVarRef(Node::VariableDef *varDef);
    Node::FuncCall *parseFuncCall(Node::FunctionDef *funcDef);
};

#endif