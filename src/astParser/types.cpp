#ifndef _parser_types
#define _parser_types
#include "AstParser.h"

namespace Var
{
    std::unordered_map<Var::Type, int> Var::typeSizeMap = {
        {Var::Type::BYTE, 8},
        {Var::Type::CHAR, 8},
        {Var::Type::SHORT, 16},
        {Var::Type::INT, 32},
        {Var::Type::LONG, 64},
    };

    std::unordered_map<std::string, Var::Type> Var::tokenToType = {
        {"byte", Var::Type::BYTE},
        {"char", Var::Type::CHAR},
        {"short", Var::Type::SHORT},
        {"int", Var::Type::INT},
        {"long", Var::Type::LONG},
    };

    // Mantenha a implementação da função
    Var::Type Var::getExprType(Var::Type left, Var::Type right)
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

#endif