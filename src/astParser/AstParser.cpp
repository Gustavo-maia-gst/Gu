#include "AstParser.h"

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
    std::vector<Node::FunctionDef *> funcDefinitions;
    std::vector<Node::VariableDef *> globalVarsDefinitions;

    Token *token;
    while ((token = this->sc->get()))
    {
        switch (token->type)
        {
        case TokenType::FUNC:
            funcDefinitions.push_back(this->parseFunctionDef());
            break;
        case TokenType::VAR:
            globalVarsDefinitions.push_back(this->parseVarDef());
        default:
            break;
        }
    }

    auto node = new Node::ProgramNode;
    for (auto func : funcDefinitions)
    {
        node->funcDefs[*(func->identifier)] = func;
        if (func->identifier->value == "main")
        {
            if (!node->main_func)
                compile_error("main function defined twice");
            node->main_func = func;
        }
    }
    for (auto var : globalVarsDefinitions)
    {
        node->varsDefs[*(var->identifier)] = var;
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