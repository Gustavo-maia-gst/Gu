#ifndef base
#include "../types/Base.cpp"
#endif
#ifndef _token
#include "../types/Token.cpp"
#endif
#define _parser_types

namespace Var {
    enum Type {
        BYTE,
        CHAR,
        SHORT,
        INT,
        LONG,
    };

    std::map<std::string, Type> tokenToType = {
        { "byte", Type::BYTE },
        { "char", Type::CHAR },
        { "short", Type::SHORT },
        { "int", Type::CHAR },
        { "long", Type::CHAR },
    };
};

namespace Node {
    struct ProgramNode {
        std::map<Token, FunctionDef*> funcDefs;
        std::map<Token, VariableDef*> varsDefs;

        FunctionDef* main_func;
    };

    struct FunctionDef {
        Token* identifier;
        std::vector<VariableDef*> args;
        BodyDef* body;
    };

    struct BodyDef {
        std::map<Token, VariableDef*> varsDefs;
        std::vector<StatementDef*> statements;
    };

    enum StatementType {
        IF,
        WHILE,
        FOR,
        VAR_DEF,
        ASSIGN,
        FUNCCALL,
    };

    struct StatementDef {
        StatementType type;
    };

    struct IfDef : StatementDef {
        ExprNode* expr;
        BodyDef* ifBlock;
        BodyDef* elseBlock;
    };

    struct WhileDef : StatementDef {
        ExprNode* expr;
        BodyDef* block;
    };

    struct ForDef : StatementDef{
        ExprNode* begin;
        ExprNode* expr;
        ExprNode* inc;
        BodyDef* block;
    };

    struct AssignDef : StatementDef {
        VariableDef* var;
        ExprNode* val;
    };

    struct VariableDef : StatementDef {
        Token* identifier;
        TypeDef* type;
    };

    enum ExprNodeType {
        EXPR,
        OPERATOR,
        CONSTANT,
        VARIABLE,
        FUNCCALL,
    };

    struct ExprNode {
        Var::Type exprType;
        ExprNodeType nodeType;
        std::vector<ExprNode*> children;
    };

    struct OperatorNode : ExprNode {
        ExprNode* left;
        ExprNode* right;
        Token* op;
    };

    struct ConstantNode : ExprNode {
        Token* value;
    };

    struct FuncCall : ExprNode, StatementDef {
        FunctionDef* func;
    };

    struct TypeDef {
        Var::Type type;
    };
}