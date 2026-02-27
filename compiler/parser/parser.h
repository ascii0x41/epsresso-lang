#pragma once

#include "lexer/token.h"
#include "common/diagnostics.h"
#include "ast/astlib.h"
#include <deque>

namespace espresso_compiler {

class Parser {
    std::string filepath;
    const TokenStream& tokens;
    size_t idx = 0;
    TagList tag_buffer;  // Accumulates #tag before a declaration
    std::deque<Token> pending_tokens;  // Queue of injected tokens (for handling >> as > >)

    Ptr<BlockStatement> result;  // Final result of parse()

    // ===== Navigation =====
    const Token& peek() const;
    const Token& peek_next() const;  // One token of extra lookahead
    bool is_at_end() const;
    Token advance();
    Token expect(TokenType type, const char* msg);
    Token expect(std::initializer_list<TokenType> types, const char* msg);
    bool accept(TokenType type);

    // ===== Tag buffer =====
    TagPtr parse_tag();  // Parses a single tag and returns it (or throws on error)
    TagList flush_tags();  // Returns and clears the buffer

    // ===== Op helpers =====
    BinaryOp token_to_binary_op(TokenType type) const;
    UnaryOp  token_to_unary_op(TokenType type) const;
    OperatorOverloadType token_to_operator_type(TokenType type, bool is_unary) const;

    // ===== Generic parameter parsing =====
    // Parses <T, U:Constraint> on a declaration
    GenericParams parse_generic_params();

    // ===== Qualified name parsing =====
    // Parses names with :: separators (e.g., math::real)
    // Returns a Name expression tree
    Ptr<Name> parse_qualified_name();

    // ===== Type parsing =====
    TypeExprPtr parse_type_expression();

    // ===== Expression parsing (precedence chain) =====
    ExpressionPtr parse_unpack_expr();
    ExpressionPtr parse_expression();
    ExpressionPtr parse_assignment();
    ExpressionPtr parse_ternary();
    ExpressionPtr parse_logical_or();
    ExpressionPtr parse_logical_and();
    ExpressionPtr parse_bitwise_or();
    ExpressionPtr parse_bitwise_xor();
    ExpressionPtr parse_bitwise_and();
    ExpressionPtr parse_equality();
    ExpressionPtr parse_comparison();
    ExpressionPtr parse_shift();
    ExpressionPtr parse_term();
    ExpressionPtr parse_factor();
    ExpressionPtr parse_unary();
    ExpressionPtr parse_postfix();
    ExpressionPtr parse_primary();

    // ===== Primary sub-parsers =====
    ExpressionPtr parse_array_literal();
    ExpressionPtr parse_map_literal();
    ExpressionPtr parse_interp_string();
    ExpressionPtr parse_lambda();         // (x:Int) => Int { ... }
    ExpressionPtr parse_paren_or_lambda();  // disambiguates ( as grouping vs lambda

    // ===== Call argument parsing =====
    // Handles both positional and named args: func(x, y=10)
    std::vector<CallExprNode::Argument> parse_call_arguments();

    // ===== Statement parsing =====
    StatementPtr parse_statement();
    Ptr<BlockStatement> parse_block();

    // ===== Declaration parsing =====
    Ptr<VariableDeclNode> parse_variable_decl();
    Ptr<DestructureDeclNode> parse_destructure_decl();
    Ptr<FunctionDeclNode> parse_function_decl();
    Ptr<StructDeclNode> parse_struct_decl();
    Ptr<OperatorOverloadNode> parse_operator_overload();
    Ptr<TraitDeclNode> parse_trait_decl();
    Ptr<TypeAliasDeclNode> parse_type_alias_decl();
    Ptr<EnumDeclNode> parse_enum_decl();
    Ptr<ScopeDeclNode> parse_scope_decl();

    // ===== Control flow parsing =====
    StatementPtr parse_if_stmt();
    StatementPtr parse_while_stmt();
    StatementPtr parse_do_while_stmt();
    StatementPtr parse_for_stmt();
    StatementPtr parse_foreach_stmt();

    // ===== Exception handling =====
    StatementPtr parse_try_catch_stmt();
    StatementPtr parse_throw_stmt();

    // ===== Jump statements =====
    StatementPtr parse_return_stmt();
    StatementPtr parse_break_stmt();
    StatementPtr parse_continue_stmt();

    // ===== Module system =====
    StatementPtr parse_import_decl();
    StatementPtr parse_export_decl();

    // ===== Lookahead helpers =====
    // True if the upcoming token sequence looks like a lambda parameter list
    // rather than a parenthesised expression.
    bool looks_like_lambda() const;

    // True if a '<' at the current position is a generic argument list
    // (e.g. Pair<Int>) rather than a less-than operator (a < b).
    // Uses one token of lookahead: only true if preceded by an identifier.
    bool looks_like_generic_args(const ExpressionPtr& preceding) const;

    // Inject a synthetic closing angle bracket token (for handling >> as > >)
    void inject_closing_angle();

    SourceLocation loc() const;  // Current token's location

public:
    explicit Parser(const TokenStream& ts, std::string file) : tokens(ts), filepath(std::move(file)) {}

    // Parse a complete file into a block of top-level statements.
    Ptr<BlockStatement> parse();
};

} // namespace espresso_compiler