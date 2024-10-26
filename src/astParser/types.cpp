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

    enum StatementType
    {
        IF,
        WHILE,
        FOR,
        VAR_DEF,
        ASSIGN,
        FUNCCALL,
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

    enum ExprNodeType
    {
        EXPR,
        OPERATOR,
        ATOM,
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

    enum AtomNodeType
    {
        CONSTANT,
        VARIABLE,
        FUNCCALL,
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

    struct TypeDef
    {
        Var::Type type;
    };
}

namespace Order
{
    std::unordered_map<int, std::unordered_map<TokenType, std::function<Node::ExprNode *(AstParser *, Node::ExprNode *)>>> precedenceOrderMapper = {
        {1, {
                {TokenType::OPERATOR_MINUS, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(0);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_MINUS;

                     auto leftNode = new Node::ConstantNode;
                     leftNode->nodeType = Node::ExprNodeType::ATOM;
                     leftNode->atomType = Node::AtomNodeType::CONSTANT;
                     leftNode->exprType = right->exprType;
                     leftNode->value = new Token{"0", TokenType::NUMBER};

                     node->left = leftNode;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
                {TokenType::OPERATOR_NEG, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(0);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_NEG;
                     node->left = right;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::Type::BOOLEAN;

                     return node;
                 }},
            }},
        {2, {
                {TokenType::OPERATOR_MULT, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(1);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_MULT;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
                {TokenType::OPERATOR_DIV, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(1);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_DIV;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
            }},
        {3, {
                {TokenType::OPERATOR_PLUS, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(2);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_PLUS;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
                {TokenType::OPERATOR_MINUS, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(2);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_MINUS;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
                {TokenType::OPERATOR_MOD, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(2);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_MOD;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
            }},
        {4, {
                {TokenType::OPERATOR_PLUS, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(3);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_PLUS;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
                {TokenType::OPERATOR_MINUS, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(3);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_MINUS;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
            }},
        {5, {
                {TokenType::OPERATOR_BIN_LSHIFT, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(4);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_BIN_LSHIFT;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
                {TokenType::OPERATOR_BIN_RSHIFT, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(4);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_BIN_RSHIFT;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
            }},
        {6, {
                {TokenType::OPERATOR_GT, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(5);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_GT;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::Type::BOOLEAN;

                     return node;
                 }},
                {TokenType::OPERATOR_GTE, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(5);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_GTE;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::Type::BOOLEAN;

                     return node;
                 }},
                {TokenType::OPERATOR_LT, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(5);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_LT;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::Type::BOOLEAN;

                     return node;
                 }},
                {TokenType::OPERATOR_LTE, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(5);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_LTE;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::Type::BOOLEAN;

                     return node;
                 }},
            }},
        {7, {
                {TokenType::OPERATOR_EQ, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(6);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_EQ;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::Type::BOOLEAN;

                     return node;
                 }},
                {TokenType::OPERATOR_NEQ, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(6);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_NEQ;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::Type::BOOLEAN;

                     return node;
                 }},
            }},
        {8, {
                {TokenType::OPERATOR_BIN_AND, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(7);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_BIN_AND;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
            }},
        {9, {
                {TokenType::OPERATOR_BIN_XOR, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(8);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_BIN_XOR;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
                {TokenType::OPERATOR_BIN_OR, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                 {
                     auto right = parser->parseExpr(8);
                     auto node = new Node::OperatorNode;
                     node->op = TokenType::OPERATOR_BIN_OR;
                     node->left = left;
                     node->right = right;
                     node->nodeType = Node::ExprNodeType::OPERATOR;
                     node->exprType = Var::getExprType(left->exprType, right->exprType);

                     return node;
                 }},
            }},
        {10, {
                 {TokenType::OPERATOR_LOG_AND, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                  {
                      auto right = parser->parseExpr(9);
                      auto node = new Node::OperatorNode;
                      node->op = TokenType::OPERATOR_LOG_AND;
                      node->left = left;
                      node->right = right;
                      node->nodeType = Node::ExprNodeType::OPERATOR;
                      node->exprType = Var::Type::BOOLEAN;

                      return node;
                  }},
             }},
        {11, {
                 {TokenType::OPERATOR_LOG_OR, [](AstParser *parser, Node::ExprNode *left) -> Node::ExprNode *
                  {
                      auto right = parser->parseExpr(10);
                      auto node = new Node::OperatorNode;
                      node->op = TokenType::OPERATOR_LOG_OR;
                      node->left = left;
                      node->right = right;
                      node->nodeType = Node::ExprNodeType::OPERATOR;
                      node->exprType = Var::Type::BOOLEAN;

                      return node;
                  }},
             }},
    };

    const int MAX_PRECEDENCE_LEVEL = 11;
}