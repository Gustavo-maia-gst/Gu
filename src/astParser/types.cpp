#ifndef base
#include "../types/Base.cpp"
#endif
#ifndef _token
#include "../types/Token.cpp"
#endif
#define _parser_types

namespace Var
{
    enum Type
    {
        BYTE,
        CHAR,
        SHORT,
        INT,
        LONG,
        BOOLEAN,
    };

    std::map<std::string, Type> tokenToType = {
        {"byte", Type::BYTE},
        {"char", Type::CHAR},
        {"short", Type::SHORT},
        {"int", Type::CHAR},
        {"long", Type::CHAR},
    };

    std::map<Type, int> typeSizeMap = {
        {Type::BYTE, 8},
        {Type::CHAR, 8},
        {Type::SHORT, 16},
        {Type::INT, 32},
        {Type::LONG, 64},
    };

    Type getExprType(Type left, Type right)
    {
        int leftSize = typeSizeMap[left];
        int rightSize = typeSizeMap[right];
        if (leftSize > rightSize)
            return left;

        return right;
    }
};

namespace Node
{
    struct FunctionDef;
    struct VariableDef;
    struct BodyDef;
    struct IfDef;
    struct WhileDefj;
    struct ForDef;
    struct StatementDef;
    struct AssignDef;
    struct ExprNode;
    struct OperatorNode;
    struct VariableNode;
    struct AtomNode;
    struct TypeDef;

    enum StatementType
    {
        IF,
        WHILE,
        FOR,
        VAR_DEF,
        ASSIGN,
        FUNCCALL,
    };

    enum ExprNodeType
    {
        EXPR,
        OPERATOR,
        ATOM,
    };

    enum AtomNodeType
    {
        CONSTANT,
        VARIABLE,
        FUNCCALL,
        EXPR
    };

    struct ProgramNode
    {
        std::map<Token, FunctionDef *> funcDefs;
        std::map<Token, VariableDef *> varsDefs;

        FunctionDef *main_func;

        ~ProgramNode()
        {
            for (auto &[_, func] : funcDefs)
                delete func;
            for (auto &[_, var] : varsDefs)
                delete var;
            delete main_func;
        }
    };

    struct FunctionDef
    {
        Token *identifier;
        std::vector<VariableDef *> args;
        BodyDef *body;

        ~FunctionDef()
        {
            delete identifier;
            for (auto &var : args)
                delete var;
            delete body;
        }
    };

    struct BodyDef
    {
        std::map<Token, VariableDef *> varsDefs;
        std::vector<StatementDef *> statements;

        ~BodyDef()
        {
            for (auto &[_, var] : varsDefs)
                delete var;
            for (auto &statement : statements)
                delete statement;
        }
    };

    struct StatementDef
    {
        StatementType statementType;
    };

    struct IfDef : StatementDef
    {
        ExprNode *expr;
        BodyDef *ifBlock;
        BodyDef *elseBlock;

        ~IfDef()
        {
            delete expr;
            delete ifBlock;
            delete elseBlock;
        }
    };

    struct WhileDef : StatementDef
    {
        ExprNode *expr;
        BodyDef *block;

        ~WhileDef()
        {
            delete expr;
            delete block;
        }
    };

    struct ForDef : StatementDef
    {
        ExprNode *begin;
        ExprNode *expr;
        ExprNode *inc;
        BodyDef *block;

        ~ForDef()
        {
            delete begin;
            delete expr;
            delete inc;
            delete block;
        }
    };

    struct AssignDef : StatementDef
    {
        VariableDef *var;
        ExprNode *val;

        ~AssignDef()
        {
            delete var;
            delete val;
        }
    };

    struct VariableDef : StatementDef
    {
        Token *identifier;
        TypeDef *type;
        ExprNode *value;

        ~VariableDef()
        {
            delete identifier;
            delete type;
            delete value;
        }
    };

    struct ExprNode
    {
        Var::Type exprType;
        ExprNodeType nodeType;
    };

    struct OperatorNode : ExprNode
    {
        ExprNode *left;
        ExprNode *right;
        TokenType op;

        ~OperatorNode()
        {
            delete left;
            delete right;
        }
    };

    struct AtomNode : ExprNode
    {
        AtomNodeType atomType;
    };

    struct VariableNode : AtomNode
    {
        VariableDef *var;

        ~VariableNode()
        {
            delete var;
        }
    };

    struct FuncCall : AtomNode, StatementDef
    {
        FunctionDef *func;

        ~FuncCall()
        {
            delete func;
        }
    };

    struct ConstantNode : AtomNode
    {
        Token *value;

        ~ConstantNode()
        {
            delete value;
        }
    };

    struct ExprAtomNode : AtomNode
    {
        ExprNode *expr;

        ~ExprAtomNode()
        {
            delete expr;
        }
    };

    struct TypeDef
    {
        Var::Type type;
    };
}
