#include "AstParser.h"

namespace Var
{
    std::unordered_map<Var::Type, int> typeSizeMap = {
        {Var::Type::BYTE, 8},
        {Var::Type::CHAR, 8},
        {Var::Type::SHORT, 16},
        {Var::Type::INT, 32},
        {Var::Type::LONG, 64},
    };

    std::unordered_map<std::string, Var::Type> tokenToType = {
        {"byte", Var::Type::BYTE},
        {"char", Var::Type::CHAR},
        {"short", Var::Type::SHORT},
        {"int", Var::Type::INT},
        {"long", Var::Type::LONG},
    };

    // Mantenha a implementação da função
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
}

AstParser::AstParser(LexicalScanner *sc)
{
    this->sc = sc;
}

AstParser::~AstParser()
{
    delete this->sc;
}

Node::ProgramNode *AstParser::parseProgram()
{
    auto node = new Node::ProgramNode;
    this->context = node;

    Token *token;
    while ((token = this->sc->get()))
    {
        switch (token->type)
        {
        case TokenType::FUNC:
        {
            auto funcDef = this->parseFunctionDef();
            node->funcDefs[*(funcDef->identifier)] = funcDef;
            if (funcDef->identifier->value == "main")
            {
                if (node->main_func)
                    compile_error("main function defined twice");
                node->main_func = funcDef;
            }
            break;
        }
        case TokenType::VAR:
        {
            auto varDef = this->parseVarDef();
            node->varsDefs[*(varDef->identifier)] = varDef;
            break;
        }
        default:
            break;
        }
    }

    if (!node->main_func)
        compile_error("main function not defined");

    return node;
}

Node::VariableDef *AstParser::parseVarDef()
{
    Token *ident = this->sc->get_expected(TokenType::IDENTIFIER, "identifier");

    this->sc->expect(TokenType::OPERATOR_TYPE, "type definition");

    Token *type = this->sc->get_expected(TokenType::TYPE, "type");

    Token *next = this->sc->get();

    auto node = new Node::VariableDef;
    node->statementType = Node::StatementType::VAR_DEF;
    node->identifier = ident;
    node->type = new Node::TypeDef{Var::tokenToType[type->value]};

    if (next->type == TokenType::END_EXPR)
        return node;
    if (next->type == TokenType::OPERATOR_ASSIGN)
    {
        auto expr = this->parseExpr();
        this->sc->expect(TokenType::END_EXPR, "semicolon");
        node->value = expr;
        return node;
    }

    sintax_error("expecting assignment or semicolon, got " + next->value);
    return nullptr;
}

Node::ExprNode *AstParser::parseExpr(int level)
{
    if (level < 0)
        compile_error("Error parsing expression");
    if (!level)
        return this->parseAtom();

    auto left = this->parseExpr(level - 1);
    auto op = this->sc->get();

    auto levelMapper = Order::precedenceOrderMapper[level];
    if (!op || levelMapper.find(op->type) == levelMapper.end())
    {
        this->sc->unget(op);
        return left;
    }

    auto handler = levelMapper[op->type];
    return handler(this, left);
}

Node::AtomNode *AstParser::parseAtom()
{
    Token *token = this->sc->get();
    if (this->sc->lookChar() == '(')
    {
        this->sc->match("(");
        auto expr = this->parseExpr();
        this->sc->match(")");
        auto node = new Node::ExprAtomNode;
        node->nodeType = Node::ExprNodeType::ATOM;
        node->atomType = Node::AtomNodeType::EXPR;
        node->expr = expr;
        node->exprType = expr->exprType;
        return node;
    }

    switch (token->type)
    {
    case TokenType::IDENTIFIER:
    {
        auto refFunc = this->context->funcDefs[*token];
        auto refVar = this->context->varsDefs[*token];
        if (refVar)
            return this->parseVarRef(refVar);
        if (refFunc)
            return this->parseFuncCall(refFunc);

        sintax_error("Unresolved identifier " + token->value);
        break;
    }
    case TokenType::NUMBER:
    {
        auto numberNode = new Node::ConstantNode;
        numberNode->nodeType = Node::ExprNodeType::ATOM;
        numberNode->atomType = Node::AtomNodeType::CONSTANT;
        numberNode->exprType = Var::Type::INT;
        numberNode->value = token;
        return numberNode;
        break;
    }
    case TokenType::CHAR:
    {
        auto charNode = new Node::ConstantNode;
        charNode->nodeType = Node::ExprNodeType::ATOM;
        charNode->atomType = Node::AtomNodeType::CONSTANT;
        charNode->exprType = Var::Type::CHAR;
        charNode->value = token;
        return charNode;
    }
    case TokenType::STRING:
    {
        compile_error("Not implemented");
    }
    default:
        break;
    }

    compile_error("Expecting atom, received " + token->value);
    return nullptr;
}

Node::BodyDef *AstParser::parseBody()
{
    return nullptr;
}

Node::StatementDef *AstParser::parseStatement()
{
    return nullptr;
}

Node::ForDef *AstParser::parseFor()
{
    return nullptr;
}

Node::WhileDef *AstParser::parseWhile()
{
    return nullptr;
}

Node::IfDef *AstParser::parseIf()
{
    return nullptr;
}

Node::FunctionDef *AstParser::parseFunctionDef()
{
    return nullptr;
}

Node::AssignDef *AstParser::parseAssign()
{
    return nullptr;
}

Node::VariableNode *AstParser::parseVarRef(Node::VariableDef *varDef)
{
    return nullptr;
}

Node::FuncCall *AstParser::parseFuncCall(Node::FunctionDef *funcDef)
{
    return nullptr;
}
