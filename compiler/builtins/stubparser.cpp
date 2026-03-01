#include "stubparser.h"
#include <format>
#include <unordered_set>
#include <unordered_map>

namespace espresso_compiler {

// ============================================================================
// NAVIGATION
// ============================================================================


const Token& StubParser::peek() const {
    if (!pending_tokens.empty()) {
        return pending_tokens.front();
    }
    return tokens[idx];
}

const Token& StubParser::peek_next() const {
    if (!pending_tokens.empty()) {
        // If we have pending tokens, peek_next is the next pending or the current stream token
        if (pending_tokens.size() > 1) {
            return pending_tokens[1];
        }
        return tokens[idx];
    }
    size_t next = idx + 1;
    return (next < tokens.size()) ? tokens[next] : tokens.back();
}

bool StubParser::is_at_end() const {
    return peek().type == TokenType::EOF_TOKEN;
}

Token StubParser::advance() {
    Token t = peek();
    if (!pending_tokens.empty()) {
        pending_tokens.pop_front();
    } else if (!is_at_end()) {
        ++idx;
    }
    return t;
}

Token StubParser::expect(TokenType type, const char* msg) {
    if (peek().type != type) {
        throw CompilerException(
            std::format("{} (got '{}')", msg, peek().lexeme),
            peek().line, peek().column, filepath
        );
    }
    return advance();
}

bool StubParser::accept(TokenType type) {
    if (peek().type == type) { advance(); return true; }
    return false;
}

SourceLocation StubParser::loc() const {
    return { peek().line, peek().column, filepath };
}

// ============================================================================
// SYNTHETIC TOKEN INJECTION
// ============================================================================

void StubParser::inject_closing_angle() {
    // Create a synthetic GT token and inject it into the pending queue
    Token synthetic(TokenType::GT, ">", peek().line, peek().column);
    pending_tokens.push_back(synthetic);
}

// ============================================================================
// TAG BUFFER
// ============================================================================

TagList StubParser::flush_tags() {
    TagList result = std::move(tag_buffer);
    tag_buffer.clear();
    return result;
}

TagPtr StubParser::parse_tag() {
    /*
    The lexer emits two tokens for every tag:
        HASH  TAG_<KIND>  [optional: LPAREN STRING_LITERAL RPAREN]

    This is called when peek() == HASH.
    We consume HASH, then dispatch on the following TAG_* token.

    Supported tags:
        #staticmember
        #constmethod
        #constexpr
        #inline
        #deprecated                      -- no message
        #deprecated("Use foo() instead") -- with message
        #docstring("...")
    */
    SourceLocation l = loc();
    expect(TokenType::HASH, "Expected '#' for tag");

    switch (peek().type) {
        case TokenType::TAG_STATICMEMBER:
            advance();
            return std::make_shared<TagStaticmember>(l);

        case TokenType::TAG_CONSTMETHOD:
            advance();
            return std::make_shared<TagConstmethod>(l);

        case TokenType::TAG_CONSTEXPR:
            advance();
            return std::make_shared<TagConstexpr>(l);

        case TokenType::TAG_INLINE:
            advance();
            return std::make_shared<TagInline>(l);

        case TokenType::TAG_DEPRECATED: {
            advance();
            std::string msg;
            if (accept(TokenType::LPAREN)) {
                msg = expect(TokenType::STRING_LITERAL,
                             "Expected deprecation message string").lexeme;
                expect(TokenType::RPAREN, "Expected ')' after deprecation message");
            }
            return std::make_shared<TagDeprecated>(std::move(msg), l);
        }

        case TokenType::TAG_DOCSTRING: {
            // NOTE: #docstring is not in the lexer's tag map yet.
            // You need to add {"docstring", TokenType::TAG_DOCSTRING} to
            // Lexer::tags in lexer.cpp for this to work.
            advance();
            std::string doc;
            if (accept(TokenType::LPAREN)) {
                doc = expect(TokenType::STRING_LITERAL,
                             "Expected docstring content").lexeme;
                expect(TokenType::RPAREN, "Expected ')' after docstring");
            }
            return std::make_shared<TagDocstring>(std::move(doc), l);
        }

        default:
            throw CompilerException(
                std::format("Unknown tag '#{}'", peek().lexeme),
                peek().line, peek().column, filepath
            );
    }
}

// ============================================================================
// QUALIFIED NAME PARSING
// Parses names with :: separators: math::real, std::vector, etc.
// ============================================================================

NamePtr StubParser::parse_qualified_name() {
    SourceLocation l = loc();
    std::string first_name = expect(TokenType::IDENTIFIER,
                                    "Expected identifier").lexeme;
    
    NamePtr result = std::make_shared<NameExpression>(first_name, l);
    
    while (peek().type == TokenType::COLON_COLON || peek().type == TokenType::DOT) {
        bool is_static = peek().type == TokenType::COLON_COLON;
        advance();  // consume '::' or '.'
        std::string next_name = expect(TokenType::IDENTIFIER,
                                      "Expected identifier after '::' or '.'").lexeme;
        result = std::make_shared<MemberAccessExpr>(
            result, std::move(next_name), is_static, l);
    }
    
    return result;
}

// ============================================================================
// GENERIC PARAMETER PARSING
// Parses <T, U:Constraint> on a declaration.
// Called after consuming '<'.
// ============================================================================

GenericParamSymbols StubParser::parse_generic_params() {
    GenericParamSymbols result;

    do {
        std::string name = expect(TokenType::IDENTIFIER,
                                  "Expected generic parameter name").lexeme;

        std::vector<GenericParamSymbol::TraitConstraint> constraints;

        if (accept(TokenType::COLON)) {
            // Multiple constraints separated by '+'
            while (true) {
                auto trait_name_ptr = parse_qualified_name();
                auto trait_name_node = std::dynamic_pointer_cast<Name>(trait_name_ptr);
                auto trait_name_string = trait_name_node->to_string();
        
                std::vector<TypeSymbol> args;
                if (peek().type == TokenType::LT) {
                    advance();
                    do {
                        args.push_back(type_expr_to_type_symbol(parse_type_expression()));
                    } while (accept(TokenType::COMMA));
                    // Handle closing '>': can be either GT or the first '>' of >>
                    if (peek().type == TokenType::GT) {
                        advance();
                    } else if (peek().type == TokenType::GT_GT) {
                        advance();
                        inject_closing_angle();
                    } else {
                        throw CompilerException(
                            "Expected '>' after trait arguments",
                            peek().line, peek().column, filepath
                        );
                    }
                }

                constraints.emplace_back(trait_name_node, std::move(args));

                // Constraints are separated by '+'; anything else ends constraint list
                if (!accept(TokenType::PLUS)) {
                    break;
                }
            }
        }

        result.emplace_back(name, std::move(constraints));

    } while (accept(TokenType::COMMA));

    // Handle closing '>': can be either GT or the first '>' of >>
    if (peek().type == TokenType::GT) {
        advance();
    } else if (peek().type == TokenType::GT_GT) {
        advance();
        inject_closing_angle();
    } else {
        throw CompilerException(
            "Expected '>' after generic parameters",
            peek().line, peek().column, filepath
        );
    }
    return result;
}

// ============================================================================
// TYPE EXPRESSION PARSING
//
//   type_expr := ['const'] '&'? base_type ['<' type_list '>']
//   base_type  := IDENTIFIER
// ============================================================================

TypeExprPtr StubParser::parse_type_expression() {
    SourceLocation l = loc();
    bool is_const = accept(TokenType::CONST);

    // Reference type: &Type  or  &const Type
    if (accept(TokenType::REF)) {
        auto pointee = parse_type_expression();
        return std::make_shared<ReferenceType>(std::move(pointee), is_const, l);
    }

    std::string base_name = expect(TokenType::IDENTIFIER,
                                   "Expected type name").lexeme;

    // Generic arguments: Array<Int>, Tuple<Int, String>
    if (peek().type == TokenType::LT) {
        advance();  // consume '<'
        std::vector<TypeExprPtr> args;
        if (peek().type != TokenType::GT && peek().type != TokenType::GT_GT) {
            do {
                args.push_back(parse_type_expression());
            } while (accept(TokenType::COMMA));
        }
        // Handle closing '>': can be either GT or the first '>' of a GT_GT (>>)
        if (peek().type == TokenType::GT) {
            advance();
        } else if (peek().type == TokenType::GT_GT) {
            // Consume >>, but inject one > back for the parent level
            advance();
            inject_closing_angle();
        } else {
            throw CompilerException(
                "Expected '>' after generic type arguments",
                peek().line, peek().column, filepath
            );
        }
        return std::make_shared<GenericType>(base_name, std::move(args), is_const, l);
    }

    return std::make_shared<SimpleType>(base_name, is_const, l);
}

TypeSymbol StubParser::type_expr_to_type_symbol(const TypeExprPtr& type_expr) {
    if (auto simple = std::dynamic_pointer_cast<SimpleType>(type_expr)) {
        return TypeSymbol{simple->name, {}};
    }
    if (auto generic = std::dynamic_pointer_cast<GenericType>(type_expr)) {
        std::vector<TypeSymbol> args;
        for (const auto& arg : generic->type_arguments) {
            args.push_back(type_expr_to_type_symbol(arg));
        }
        return TypeSymbol{generic->base_name, std::move(args)};
    }
    if (auto ref = std::dynamic_pointer_cast<ReferenceType>(type_expr)) {
        // Represent &Int as a TypeSymbol with name "ref" and the inner type as arg
        return TypeSymbol{"ref", {type_expr_to_type_symbol(ref->pointee_type)}};
    }
    return TypeSymbol::make_error();
}

OperatorOverloadType StubParser::token_to_operator_type(TokenType type, bool is_unary) const {
    using OT = OperatorOverloadType;
    if (is_unary) {
        switch (type) {
            case TokenType::PLUS:  return OT::UNARY_PLUS;
            case TokenType::MINUS: return OT::UNARY_MINUS;
            default: break;
        }
    } else {
        switch (type) {
            case TokenType::PLUS:       return OT::BINARY_PLUS;
            case TokenType::MINUS:      return OT::BINARY_MINUS;
            case TokenType::STAR:       return OT::BINARY_MULTIPLY;
            case TokenType::SLASH:      return OT::BINARY_DIVIDE;
            case TokenType::EQ_EQ:      return OT::EQUAL;
            case TokenType::BANG_EQ:    return OT::NOT_EQUAL;
            case TokenType::LT:         return OT::LESS;
            case TokenType::GT:         return OT::GREATER;
            case TokenType::LTE:        return OT::LESS_EQUAL;
            case TokenType::GTE:        return OT::GREATER_EQUAL;
            case TokenType::PLUS_EQ:    return OT::COMPOUND_ADD;
            case TokenType::MINUS_EQ:   return OT::COMPOUND_SUB;
            case TokenType::STAR_EQ:    return OT::COMPOUND_MUL;
            case TokenType::SLASH_EQ:   return OT::COMPOUND_DIV;
            case TokenType::LBRACKET:   return OT::INDEX;
            default: break;
        }
    }
    throw CompilerException(
        std::format("'{}' is not a valid operator overload symbol",
                    token_type_to_string(type)),
        peek().line, peek().column, filepath
    );
}

// ============================================================================
// DECLARATION PARSING
// ============================================================================

void StubParser::parse_variable_decl() {
    auto tags = flush_tags();
    SourceLocation l = loc();

    std::string name = expect(TokenType::IDENTIFIER,
                              "Expected variable name").lexeme;
    expect(TokenType::COLON, "Expected ':' after variable name in stub");
    auto type = parse_type_expression();
    expect(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    // Build the symbol and register it
    auto sym = std::make_shared<VariableSymbol>(
        name,
        type_expr_to_type_symbol(type)
    );
    scope.declare(sym);
}

void StubParser::parse_function_decl() {
    auto tags = flush_tags();
    SourceLocation l = loc();

    std::string name = expect(TokenType::IDENTIFIER,
                              "Expected function name").lexeme;

    GenericParamSymbols generic_params;
    if (peek().type == TokenType::LT) {
        advance();
        generic_params = parse_generic_params();
    }
    
    
}

}