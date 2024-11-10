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
        RETURN,
        BREAK,
        VAR_DEF,
        ASSIGN,
        EXPR,
    };

    enum class ExprNodeType
    {
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
    Node::ProgramNode *globalContext;
    Node::FunctionDef *localContext;

    Node::BodyDef *parseBody();
    Node::StatementDef *parseStatement();
    Node::IfDef *parseIf();
    Node::ForDef *parseFor();
    Node::WhileDef *parseWhile();
    Node::FunctionDef *parseFunctionDef();
    Node::AssignDef *parseAssign();
    Node::VariableDef *parseVarDef(std::unordered_map<Token, Node::VariableDef *>& context);
    Node::TypeDef *parseTypeDef();
    Node::AtomNode *parseAtom();
    Node::VariableNode *parseVarRef(Node::VariableDef *varRef);
    Node::FuncCall *parseFuncCall(Node::FunctionDef *funcDef);

    Node::VariableDef *getVarDef(Token* ident);
    Node::FunctionDef *getFunctionDef(Token* ident);
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
        VOID,
    };

    extern std::unordered_map<Type, int> typeSizeMap;
    extern std::unordered_map<std::string, Type> tokenToType;

    Type getExprType(Type left, Type right);
}

namespace std
{
    string to_string(const Node::ProgramNode* node);
    string to_string(const Node::FunctionDef* node);
    string to_string(const Node::VariableDef* node, int level = 0);
    string to_string(const Node::ForDef* node, int level = 0);
    string to_string(const Node::WhileDef* node, int level = 0);
    string to_string(const Node::AssignDef* node, int level = 0);
    string to_string(const Node::ExprNode* node);
    string to_string(const Node::OperatorNode* node);
    string to_string(const Node::VariableNode* node);
    string to_string(const Node::AtomNode* node);
    string to_string(const Node::TypeDef* node);
    string to_string(const Node::FuncCall* node);
    string to_string(const Node::BodyDef* node, int level = 0);
    string to_string(const Node::IfDef* node, int level = 0);
    string to_string(const Node::StatementDef* node, int level = 0);

}

#endif