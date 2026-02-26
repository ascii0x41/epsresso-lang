#include "symbol.h"
#include "ast/astlib.h"

namespace espresso_compiler {

class LocalAnalyser {

    ScopeStack scope_stack;
    
    TypeSymbolPtr validate_expression(const ExpressionPtr& expr);

    bool validate_type_trait(/*Uh...*/);

    void declare_variable(const VariableDeclNode& decl);
    void declare_function(const FunctionDeclNode& decl);
    void declare_operator_overload(const OperatorOverloadNode& decl);
    void declare_struct(const StructDeclNode& decl);
    void declare_trait(const TraitDeclNode& decl);

    

    
};

}