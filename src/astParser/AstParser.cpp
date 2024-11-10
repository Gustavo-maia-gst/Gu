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
        {"void", Var::Type::VOID},
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
        std::unordered_map<Token, FunctionDef *> funcDefs;
        std::unordered_map<Token, VariableDef *> varsDefs;

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
        std::unordered_map<Token, VariableDef *> localVars;

        BodyDef *body;
        TypeDef *retType;

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
        std::vector<StatementDef *> statements;

        ~BodyDef()
        {
            for (auto &statement : statements)
                delete statement;
        }
    };

    struct StatementDef
    {
        StatementType statementType;
    };

    struct ExprStatementDef : StatementDef
    {
        ExprNode *expr;

        ~ExprStatementDef()
        {
            delete expr;
        }
    };

    struct ReturnDef : StatementDef
    {
        ExprNode *expr;

        ~ReturnDef()
        {
            delete expr;
        }
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

namespace std
{
    string to_string(const Node::ProgramNode *node)
    {
        string program = "";

        for (auto [_, varDef] : node->varsDefs)
        {
            program += to_string(varDef);
            program += "\n";
        }

        for (auto [_, funcDef] : node->funcDefs)
        {
            program += to_string(funcDef);
        }

        program += "main()";
        return program;
    }

    string to_string(const Node::FunctionDef *node)
    {
        string function = "def ";
        function += node->identifier->value;
        function += '(';
        for (int i = 0; i < node->args.size(); i++)
        {
            function += node->args[i]->identifier->value;
            if (i < node->args.size() - 1)
                function += ", ";
        }
        function += "):\n";
        function += to_string(node->body, 1);
        return function;
    }

    string to_string(const Node::BodyDef *node, int level)
    {
        string block = "";
        for (auto statement : node->statements)
            block += to_string(statement, level) + "\n";
        return block + '\n';
    }

    string to_string(const Node::StatementDef *node, int level)
    {
        switch (node->statementType)
        {
        case Node::StatementType::ASSIGN:
            return to_string((Node::AssignDef *)node, level);
        case Node::StatementType::BREAK:
        {
            string s = "";
            while (level--)
                s += "\t";
            return s + "break";
        }
        case Node::StatementType::RETURN:
        {
            string s = "";
            while (level--)
                s += "\t";
            return s + "return " + to_string(((Node::ReturnDef *)node)->expr);
        }
        case Node::StatementType::VAR_DEF:
            return to_string((Node::VariableDef *)node, level);
        case Node::StatementType::EXPR:
        {
            string s = "";
            while (level--)
                s += "\t";
            return s + to_string((Node::ExprNode *)node);
        }
        default:
            return "";
        }
    }

    string to_string(const Node::VariableDef *node, int level)
    {
        string s = "";
        while (level--)
            s += "\t";
        s += node->identifier->value;
        if (node->value)
            s += " = " + to_string((Node::ExprNode *)node->value);
        else
            s += " = None";
        return s;
    }

    string to_string(const Node::AssignDef *node, int level)
    {
        string s = "";
        while (level--)
            s += "\t";
        s += node->var->identifier->value + " = " + to_string((Node::ExprNode *)node->val);
        return s + '\n';
    }

    string to_string(const Node::ExprNode *node)
    {
        switch (node->nodeType)
        {
        case Node::ExprNodeType::OPERATOR:
            return to_string((Node::OperatorNode *)node);
        case Node::ExprNodeType::ATOM:
            return to_string((Node::AtomNode *)node);
        default:
            return "";
        }
    }

    string to_string(const Node::OperatorNode *node)
    {
        return to_string(node->left) + " " + reverseOperators[node->op] + " " + to_string(node->right);
    }

    string to_string(const Node::AtomNode *node)
    {
        switch (node->atomType)
        {
        case Node::AtomNodeType::CONSTANT:
            return ((Node::ConstantNode *)node)->value->value;
        case Node::AtomNodeType::FUNCCALL:
            return to_string((Node::FuncCall *)node);
        case Node::AtomNodeType::VARIABLE:
            return ((Node::VariableNode *)node)->var->identifier->value;
        case Node::AtomNodeType::EXPR:
            return "(" + to_string(((Node::ExprAtomNode *)node)->expr) + ")";
        default:
            return "";
        }
    }

    string to_string(const Node::ForDef *node, int level) { return ""; }
    string to_string(const Node::WhileDef *node, int level) { return ""; }
    string to_string(const Node::VariableNode *node) { return ""; }
    string to_string(const Node::TypeDef *node) { return ""; }
    string to_string(const Node::FuncCall *node) { return ""; }
    string to_string(const Node::IfDef *node, int level) { return ""; }
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
    this->globalContext = node;

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
            auto varDef = this->parseVarDef(node->varsDefs);
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

Node::FunctionDef *AstParser::parseFunctionDef()
{
    auto node = new Node::FunctionDef;

    node->identifier = this->sc->get_expected(TokenType::IDENTIFIER, "Identifier for function");

    this->sc->expect(TokenType::OPEN_PAR, "(");

    Token *next;
    while ((next = this->sc->get())->type != TokenType::CLOSE_PAR)
    {
        this->sc->unget(next);
        if (!node->args.empty())
            this->sc->expect(TokenType::COMMA, ",");

        auto arg = new Node::VariableDef;
        arg->identifier = this->sc->get_expected(TokenType::IDENTIFIER, "identifier");
        arg->statementType = Node::StatementType::VAR_DEF;

        this->sc->expect(TokenType::TYPE_INDICATOR, ":");

        arg->type = this->parseTypeDef();
        node->args.push_back(arg);
    }

    this->sc->expect(TokenType::TYPE_INDICATOR, ":");

    node->retType = this->parseTypeDef();

    this->localContext = node;

    node->body = this->parseBody();

    bool hasAnyReturn = false;
    for (auto &statement : node->body->statements)
    {
        if (statement->statementType == Node::StatementType::RETURN)
        {
            auto returnStatement = (Node::ReturnDef *)statement;
            hasAnyReturn = true;
            if (returnStatement->expr->exprType != node->retType->type)
                compile_error("return type not compatible");
        }
    }

    if (!hasAnyReturn && node->retType->type != Var::Type::VOID)
        compile_error("Non void function reaches the end of control without returning");

    this->localContext = nullptr;

    return node;
}

Node::BodyDef *AstParser::parseBody()
{
    Token *token = this->sc->get();

    auto node = new Node::BodyDef;

    if (token->type != TokenType::OPEN_BRACES)
    {
        auto statement = this->parseStatement();
        node->statements.push_back(statement);
        return node;
    }

    while ((token = this->sc->get())->type != TokenType::CLOSE_BRACES)
    {
        this->sc->unget(token);
        auto statement = this->parseStatement();
        if (statement)
            node->statements.push_back(statement);
    }

    return node;
}

Node::StatementDef *AstParser::parseStatement()
{
    auto token = this->sc->get();
    switch (token->type)
    {
    case TokenType::FOR:
    {
        this->sc->unget(token);
        return this->parseFor();
    }
    case TokenType::WHILE:
    {
        this->sc->unget(token);
        return this->parseWhile();
    }
    case TokenType::VAR:
    {
        return this->parseVarDef(this->localContext->localVars);
    }
    case TokenType::BREAK:
    {
        this->sc->expect(TokenType::END_EXPR, "semicolon");
        auto node = new Node::StatementDef;
        node->statementType = Node::StatementType::BREAK;
        return node;
    }
    case TokenType::RETURN:
    {
        auto node = new Node::ReturnDef;
        node->expr = this->parseExpr();
        node->statementType = Node::StatementType::RETURN;
        this->sc->expect(TokenType::END_EXPR, "semicolon");
        return node;
    }
    case TokenType::LET:
    {
        return this->parseAssign();
    }
    case TokenType::END_EXPR:
    {
        return nullptr;
    }
    default:
    {
        auto node = new Node::ExprStatementDef;
        node->statementType = Node::StatementType::EXPR;
        node->expr = this->parseExpr();

        this->sc->expect(TokenType::END_EXPR, "semicolon");
        return node;
    }
    }

    return nullptr;
}

Node::VariableDef *AstParser::parseVarDef(std::unordered_map<Token, Node::VariableDef *> &context)
{
    Token *ident = this->sc->get_expected(TokenType::IDENTIFIER, "identifier");
    if (this->globalContext->varsDefs.find(*ident) != this->globalContext->varsDefs.end())
        compile_error("Variable " + ident->value + "redefined");

    this->sc->expect(TokenType::TYPE_INDICATOR, "type definition");

    auto node = new Node::VariableDef;
    node->statementType = Node::StatementType::VAR_DEF;
    node->identifier = ident;
    node->type = this->parseTypeDef();

    Token *next = this->sc->get();

    if (next->type == TokenType::OPERATOR_ASSIGN)
    {
        auto expr = this->parseExpr();
        node->value = expr;
    }
    else
        this->sc->unget(next);

    this->sc->expect(TokenType::END_EXPR, "semicolon");

    context[*ident] = node;
    return node;
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
    bool handlerFound = levelMapper.find(op->type) != levelMapper.end();

    if (!op || !handlerFound)
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

    if (token->type == TokenType::OPEN_PAR)
    {
        auto expr = this->parseExpr();
        this->sc->get_expected(TokenType::CLOSE_PAR, "closing parenteses");
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
        auto refFunc = this->getFunctionDef(token);
        auto refVar = this->getVarDef(token);

        if (refVar)
        {
            auto node = new Node::VariableNode;
            node->nodeType = Node::ExprNodeType::ATOM;
            node->atomType = Node::AtomNodeType::VARIABLE;
            node->exprType = refVar->type->type;
            node->var = refVar;
            return node;
        }
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

Node::AssignDef *AstParser::parseAssign()
{
    auto ident = this->sc->get();
    auto varDef = this->getVarDef(ident);
    if (!varDef)
        compile_error("Unknown variable " + ident->value);

    this->sc->expect(TokenType::OPERATOR_ASSIGN, "assign operator");

    auto node = new Node::AssignDef;
    node->var = varDef;
    node->val = this->parseExpr();
    node->statementType = Node::StatementType::ASSIGN;

    this->sc->expect(TokenType::END_EXPR, "semicolon");

    return node;
}

Node::FuncCall *AstParser::parseFuncCall(Node::FunctionDef *funcDef)
{
    return nullptr;
}

Node::TypeDef *AstParser::parseTypeDef()
{
    auto tokenType = this->sc->get_expected(TokenType::TYPE, "type");
    auto node = new Node::TypeDef;
    node->type = Var::tokenToType[tokenType->value];
    return node;
}

Node::VariableDef *AstParser::getVarDef(Token *ident)
{
    Node::VariableDef *varDef;

    if (this->globalContext->varsDefs.find(*ident) != this->globalContext->varsDefs.end())
        varDef = this->localContext->localVars[*ident];
    if (this->localContext->localVars.find(*ident) != this->localContext->localVars.end())
        varDef = this->localContext->localVars[*ident];

    return varDef;
}

Node::FunctionDef *AstParser::getFunctionDef(Token *ident)
{
    Node::FunctionDef *funcDef;

    if (this->globalContext->varsDefs.find(*ident) != this->globalContext->varsDefs.end())
        funcDef = this->globalContext->funcDefs[*ident];

    return funcDef;
}