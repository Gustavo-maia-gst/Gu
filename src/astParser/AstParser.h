#ifndef _parser
#define _parser
#include "../lexicalScanner/LexicalScanner.h"

namespace Order
{
    const int MAX_PRECEDENCE_LEVEL = 11;
}

namespace Node
{
    struct ProgramNode;
    struct FunctionDef;
    struct VariableDef;
    struct BodyDef;
    struct IfDef;
    struct WhileDef;
    struct ForDef;
    struct StatementDef;
    struct AssignDef;
    struct ExprNode;
    struct OperatorNode;
    struct VariableNode;
    struct AtomNode;
    struct TypeDef;
    struct FuncCall;

    enum class StatementType
    {
        IF,
        WHILE,
        FOR,
        VAR_DEF,
        ASSIGN,
    };

    enum class ExprNodeType
    {
        EXPR,
        OPERATOR,
        ATOM,
    };

    enum class AtomNodeType
    {
        CONSTANT,
        VARIABLE,
        FUNCCALL,
        EXPR
    };
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

namespace Var
{
    enum class Type
    {
        BYTE,
        CHAR,
        SHORT,
        INT,
        LONG,
        BOOLEAN,
    };

    extern std::unordered_map<Type, int> typeSizeMap;
    extern std::unordered_map<std::string, Type> tokenToType;

    Type getExprType(Type left, Type right);
}

#endif