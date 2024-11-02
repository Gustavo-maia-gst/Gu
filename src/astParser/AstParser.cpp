#include "AstParser.h"


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
            auto funcDef = this->parseFunctionDef();
            node->funcDefs[*(funcDef->identifier)] = funcDef;
            if (funcDef->identifier->value == "main")
            {
                if (!node->main_func)
                    compile_error("main function defined twice");
                node->main_func = funcDef;
            }
            break;
        case TokenType::VAR:
            auto varDef = this->parseVarDef();
            node->varsDefs[*(varDef->identifier)] = varDef;
            break;
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
}

Node::ExprNode *AstParser::parseExpr(int level = Order::MAX_PRECEDENCE_LEVEL)
{
    if (level < 0)
        compile_error("Error parsing expression");
    if (!level)
        return this->parseAtom();

    auto left = this->parseExpr(level - 1);
    auto op = this->sc->get();
    if (!op)
        sintax_error("Unexpected EOF when expecting operator");

    auto levelMapper = Order::precedenceOrderMapper[level];
    if (levelMapper.find(op->type) == levelMapper.end())
        sintax_error("Invalid operator: " + op->value);

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
        auto refFunc = this->context->funcDefs[*token];
        auto refVar = this->context->varsDefs[*token];
        if (refVar)
            return this->parseVarRef(refVar);
        if (refFunc)
            return this->parseFuncCall(refFunc);

        sintax_error("Unresolved identifier " + token->value);
        break;
    case TokenType::NUMBER:
        auto numberNode = new Node::ConstantNode;
        numberNode->nodeType = Node::ExprNodeType::ATOM;
        numberNode->atomType = Node::AtomNodeType::CONSTANT;
        numberNode->exprType = Var::Type::INT;
        numberNode->value = token;
        return numberNode;
        break;
    case TokenType::CHAR:
        auto charNode = new Node::ConstantNode;
        charNode->nodeType = Node::ExprNodeType::ATOM;
        charNode->atomType = Node::AtomNodeType::CONSTANT;
        charNode->exprType = Var::Type::CHAR;
        charNode->value = token;
        return charNode;
    case TokenType::STRING:
        compile_error("Not implemented");
    default:
        break;
    }

    compile_error("Expecting atom, received " + token->value);
    return nullptr;
}
