#pragma once  // ← missing

#include "symbol.h"
#include "ast/astlib.h"
#include "common/diagnostics.h"  // ← missing, you call error() in the .cpp

namespace espresso_compiler {

class LocalAnalyser {

    ScopeStack scope_stack;
    
    TypeSymbolPtr validate_expression(const ExpressionPtr& expr);
    TypeSymbolPtr validate_type_annotation(const TypeExprPtr& type_expr);  // ← you'll need this
                                                                            // to resolve "Int" -> TypeSymbol

    bool validate_type_satisfies_trait(const TypeSymbol& type, const std::string& trait_name);  // ← fill in the params

    void validate_statement(const StatementPtr& stmt);   // ← you need this as the main driver
    void validate_block(const BlockStatement& block);    // ← and this

    void declare_variable(const VariableDeclNode& decl);
    void declare_function(const FunctionDeclNode& decl);
    void declare_operator_overload(const OperatorOverloadNode& decl);
    void declare_struct(const StructDeclNode& decl);
    void declare_trait(const TraitDeclNode& decl);

public:
    void analyse(const BlockStatement& root);  // ← public entry point
};

class GlobalSymbolRepository {};

}