#ifndef _validator
#define _validator
#include "../parser/ast/ast.h"
#include <unordered_map>

class SemanticValidator : BaseVisitor {
public:
    void visitProgram(ProgramNode *node) override;
    void visitFunction(FunctionNode *node) override;
    void visitStructDef(StructDefNode *node) override;
    void visitVarDef(VarDefNode *node) override;
    void visitTypeDefNode(TypeDefNode *node) override;

    void visitBreakNode(BreakNode *node) override;
    void visitIf(IfNode *node) override;
    void visitWhile(WhileNode *node) override;
    void visitReturnNode(ReturnNode *node) override;

    void visitExpr(ExprNode *node) override;
    void visitExprVarRef(ExprVarRefNode *node) override;
    void visitExprAssign(ExprAssignNode *node) override;
    void visitExprCall(ExprCallNode *node) override;
    void visitExprUnaryOp(ExprUnaryNode *node) override;
    void visitExprBinaryOp(ExprBinaryNode *node) override;

    std::vector<std::string> getErrors();
private:
    void unexpected_error(std::string msg);
    void compile_error(std::string msg);
    void type_error(std::string msg);
    void name_error(std::string msg);

    ProgramNode *program;
    FunctionNode *function;
};

#endif