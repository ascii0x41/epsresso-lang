#pragma once

#include "lexer/token.h"
#include "common/diagnostics.h"
#include "semantic/symbol.h"
#include <deque>

namespace espresso_compiler {

class StubParser {
    std::string filepath;
    const TokenStream& tokens;
    size_t idx = 0;
    TagList tag_buffer;
    std::deque<Token> pending_tokens;  // for >> splitting
    ScopeStack& scope;  // fills this directly, doesn't own it

    // ===== Navigation =====
    const Token& peek() const;
    const Token& peek_next() const;
    bool is_at_end() const;
    Token advance();
    Token expect(TokenType type, const char* msg);
    bool accept(TokenType type);

    // ===== Tag buffer =====
    TagPtr parse_tag();
    TagList flush_tags();

    // ===== Shared helpers =====
    NamePtr parse_qualified_name();
    GenericParamSymbols parse_generic_params();
    TypeExprPtr parse_type_expression();
    TypeSymbol type_expr_to_type_symbol(const TypeExprPtr& type_expr);
    void inject_closing_angle();
    SourceLocation loc() const;

    // ===== Op helpers (only for operator overload signatures) =====
    OperatorOverloadType token_to_operator_type(TokenType type, bool is_unary) const;

    // ===== Declaration parsing =====
    // Each of these parses the declaration and registers it in scope directly
    void parse_variable_decl();
    void parse_struct_decl();
    void parse_function_decl();
    void parse_trait_decl();
    void parse_type_alias_decl();
    void parse_enum_decl();
    void parse_scope_decl();

public:
    explicit StubParser(const TokenStream& ts, ScopeStack& s, std::string file)
        : tokens(ts), scope(s), filepath(std::move(file)) {}

    void parse();  // populates scope, throws on error
};

} // namespace espresso_compiler