#include "parser.h"
#include <format>
#include <unordered_set>
#include <unordered_map>

namespace espresso_compiler {

// ============================================================================
// NAVIGATION
// ============================================================================

const Token& Parser::peek() const {
    if (!pending_tokens.empty()) {
        return pending_tokens.front();
    }
    return tokens[idx];
}

const Token& Parser::peek_next() const {
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

bool Parser::is_at_end() const {
    return peek().type == TokenType::EOF_TOKEN;
}

Token Parser::advance() {
    Token t = peek();
    if (!pending_tokens.empty()) {
        pending_tokens.pop_front();
    } else if (!is_at_end()) {
        ++idx;
    }
    return t;
}

Token Parser::expect(TokenType type, const char* msg) {
    if (peek().type != type) {
        throw CompilerException(
            std::format("{} (got '{}')", msg, peek().lexeme),
            peek().line, peek().column, filepath
        );
    }
    return advance();
}

Token Parser::expect(std::initializer_list<TokenType> types, const char* msg) {
    if (std::find(types.begin(), types.end(), peek().type) == types.end()) {
        throw CompilerException(
            std::format("{} (got '{}')", msg, peek().lexeme),
            peek().line, peek().column, filepath
        );
    }
    return advance();
}

bool Parser::accept(TokenType type) {
    if (peek().type == type) { advance(); return true; }
    return false;
}

SourceLocation Parser::loc() const {
    return { peek().line, peek().column, filepath };
}

// ============================================================================
// SYNTHETIC TOKEN INJECTION
// ============================================================================

void Parser::inject_closing_angle() {
    // Create a synthetic GT token and inject it into the pending queue
    Token synthetic(TokenType::GT, ">", peek().line, peek().column);
    pending_tokens.push_back(synthetic);
}

// ============================================================================
// TAG BUFFER
// ============================================================================

TagList Parser::flush_tags() {
    TagList result = std::move(tag_buffer);
    tag_buffer.clear();
    return result;
}

TagPtr Parser::parse_tag() {
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
// OP HELPERS
// ============================================================================

BinaryOp Parser::token_to_binary_op(TokenType type) const {
    switch (type) {
        case TokenType::PLUS:        return BinaryOp::ADD;
        case TokenType::MINUS:       return BinaryOp::SUBTRACT;
        case TokenType::STAR:        return BinaryOp::MULTIPLY;
        case TokenType::SLASH:       return BinaryOp::DIVIDE;
        case TokenType::PERCENT:     return BinaryOp::MODULO;
        case TokenType::EQ_EQ:       return BinaryOp::EQUAL;
        case TokenType::BANG_EQ:     return BinaryOp::NOT_EQUAL;
        case TokenType::LT:          return BinaryOp::LESS;
        case TokenType::LTE:         return BinaryOp::LESS_EQUAL;
        case TokenType::GT:          return BinaryOp::GREATER;
        case TokenType::GTE:         return BinaryOp::GREATER_EQUAL;
        case TokenType::AND:         return BinaryOp::LOGICAL_AND;
        case TokenType::OR:          return BinaryOp::LOGICAL_OR;
        case TokenType::AMP:         return BinaryOp::BIT_AND;
        case TokenType::PIPE:        return BinaryOp::BIT_OR;
        case TokenType::CARET:       return BinaryOp::BIT_XOR;
        case TokenType::LT_LT:       return BinaryOp::SHIFT_LEFT;
        case TokenType::GT_GT:       return BinaryOp::SHIFT_RIGHT;
        case TokenType::EQ:          return BinaryOp::ASSIGN;
        case TokenType::PLUS_EQ:     return BinaryOp::ADD_ASSIGN;
        case TokenType::MINUS_EQ:    return BinaryOp::SUBTRACT_ASSIGN;
        case TokenType::STAR_EQ:     return BinaryOp::MULTIPLY_ASSIGN;
        case TokenType::SLASH_EQ:    return BinaryOp::DIVIDE_ASSIGN;
        case TokenType::PERCENT_EQ:  return BinaryOp::MODULO_ASSIGN;
        case TokenType::AMP_EQ:      return BinaryOp::BIT_AND_ASSIGN;
        case TokenType::PIPE_EQ:     return BinaryOp::BIT_OR_ASSIGN;
        case TokenType::CARET_EQ:    return BinaryOp::BIT_XOR_ASSIGN;
        case TokenType::TILDE_EQ:    return BinaryOp::TILDE_ASSIGN;
        case TokenType::LT_LT_EQ:    return BinaryOp::SHIFT_LEFT_ASSIGN;
        case TokenType::GT_GT_EQ:    return BinaryOp::SHIFT_RIGHT_ASSIGN;
        default:
            throw CompilerException("Token is not a binary operator",
                              peek().line, peek().column, filepath);
    }
}

UnaryOp Parser::token_to_unary_op(TokenType type) const {
    switch (type) {
        case TokenType::PLUS:  return UnaryOp::PLUS;
        case TokenType::MINUS: return UnaryOp::MINUS;
        case TokenType::BANG:  return UnaryOp::LOGICAL_NOT;
        case TokenType::TILDE: return UnaryOp::BIT_NOT;
        case TokenType::AMP:   return UnaryOp::ADDRESS_OF;
        default:
            throw CompilerException("Token is not a unary operator",
                              peek().line, peek().column, filepath);
    }
}

OperatorOverloadNode::OperatorType
Parser::token_to_operator_type(TokenType type, bool is_unary) const {
    using OT = OperatorOverloadNode::OperatorType;
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
// GENERIC PARAMETER PARSING
// Parses <T, U:Constraint> on a declaration.
// Called after consuming '<'.
// ============================================================================

GenericParams Parser::parse_generic_params() {
    GenericParams result;

    do {
        std::string name = expect(TokenType::IDENTIFIER,
                                  "Expected generic parameter name").lexeme;

        std::vector<TraitConstraint> constraints;

        if (accept(TokenType::COLON)) {
            do {
                std::string trait_name = expect(TokenType::IDENTIFIER,
                    "Expected trait name after ':'").lexeme;

                std::vector<TypeExprPtr> args;
                if (peek().type == TokenType::LT) {
                    advance();
                    do {
                        args.push_back(parse_type_expression());
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

                constraints.emplace_back(trait_name, std::move(args));

            } while (peek_next().type != TokenType::COLON);
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

TypeExprPtr Parser::parse_type_expression() {
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

// ============================================================================
// EXPRESSION PARSING — PRECEDENCE CHAIN
//
//   assignment          (right-associative =, +=, -=, ...)
//     ternary           (? :)
//       logical_or      (||)
//         logical_and   (&&)
//           bitwise_or  (|)
//             bitwise_xor (^)
//               bitwise_and (&)
//                 equality   (== !=)
//                   comparison (< <= > >=)
//                     shift    (<< >>)
//                       term   (+ -)
//                         factor (* / %)
//                           unary (- + ! ~ &)
//                             postfix (++ -- call index . ::)
//                               primary
// ============================================================================

ExpressionPtr Parser::parse_unpack_expr() {
    /*
    unpack (a, b) = tuple;
    unpack (lval, rval) = rval, lval;
    unpack (x, y, z) += 1, 2, 3;
   */

   SourceLocation l = loc();
   
   std::vector<ExpressionPtr> lvalues;
   expect(TokenType::LPAREN, "Expected '(' to start unpack expression");
    do {
        auto lval = parse_postfix();
        lvalues.push_back(lval);
    } while (peek().type == TokenType::COMMA);
    expect(TokenType::RPAREN, "Expected '(' to start unpack expression");

    BinaryOp op = token_to_binary_op(advance().type);

    std::vector<ExpressionPtr> rvalues;
    do {
        auto rval = parse_expression();
        rvalues.push_back(rval);
    } while (peek().type == TokenType::COMMA);

    return std::make_shared<UnpackExprNode>(std::move(lvalues), std::move(rvalues), op, l);
}

ExpressionPtr Parser::parse_expression() {
    return parse_assignment();
}

ExpressionPtr Parser::parse_assignment() {
    auto expr = parse_ternary();

    static const std::unordered_set<TokenType> assign_ops = {
        TokenType::EQ,       TokenType::PLUS_EQ,  TokenType::MINUS_EQ,
        TokenType::STAR_EQ,  TokenType::SLASH_EQ,  TokenType::PERCENT_EQ,
        TokenType::AMP_EQ,   TokenType::PIPE_EQ,   TokenType::CARET_EQ,
        TokenType::TILDE_EQ, TokenType::LT_LT_EQ,  TokenType::GT_GT_EQ,
    };

    if (assign_ops.count(peek().type)) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_assignment();  // right-recursive
        return std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }

    return expr;
}

ExpressionPtr Parser::parse_ternary() {
    auto expr = parse_logical_or();

    if (accept(TokenType::QUESTION)) {
        SourceLocation l = loc();
        auto then_expr = parse_expression();
        expect(TokenType::COLON, "Expected ':' in ternary expression");
        auto else_expr = parse_expression();
        return std::make_shared<TernaryExprNode>(
            std::move(expr), std::move(then_expr), std::move(else_expr), l);
    }

    return expr;
}

ExpressionPtr Parser::parse_logical_or() {
    auto expr = parse_logical_and();
    while (peek().type == TokenType::OR) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_logical_and();
        expr = std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }
    return expr;
}

ExpressionPtr Parser::parse_logical_and() {
    auto expr = parse_bitwise_or();
    while (peek().type == TokenType::AND) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_bitwise_or();
        expr = std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }
    return expr;
}

ExpressionPtr Parser::parse_bitwise_or() {
    auto expr = parse_bitwise_xor();
    while (peek().type == TokenType::PIPE) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_bitwise_xor();
        expr = std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }
    return expr;
}

ExpressionPtr Parser::parse_bitwise_xor() {
    auto expr = parse_bitwise_and();
    while (peek().type == TokenType::CARET) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_bitwise_and();
        expr = std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }
    return expr;
}

ExpressionPtr Parser::parse_bitwise_and() {
    auto expr = parse_equality();
    while (peek().type == TokenType::AMP) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_equality();
        expr = std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }
    return expr;
}

ExpressionPtr Parser::parse_equality() {
    auto expr = parse_comparison();
    while (peek().type == TokenType::EQ_EQ || peek().type == TokenType::BANG_EQ) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_comparison();
        expr = std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }
    return expr;
}

ExpressionPtr Parser::parse_comparison() {
    auto expr = parse_shift();
    while (peek().type == TokenType::LT  || peek().type == TokenType::LTE ||
           peek().type == TokenType::GT  || peek().type == TokenType::GTE) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_shift();
        expr = std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }
    return expr;
}

ExpressionPtr Parser::parse_shift() {
    auto expr = parse_term();
    while (peek().type == TokenType::LT_LT || peek().type == TokenType::GT_GT) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_term();
        expr = std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }
    return expr;
}

ExpressionPtr Parser::parse_term() {
    auto expr = parse_factor();
    while (peek().type == TokenType::PLUS || peek().type == TokenType::MINUS) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_factor();
        expr = std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }
    return expr;
}

ExpressionPtr Parser::parse_factor() {
    auto expr = parse_unary();
    while (peek().type == TokenType::STAR  ||
           peek().type == TokenType::SLASH ||
           peek().type == TokenType::PERCENT) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto right = parse_unary();
        expr = std::make_shared<BinaryExprNode>(
            token_to_binary_op(op), std::move(expr), std::move(right), l);
    }
    return expr;
}

ExpressionPtr Parser::parse_unary() {
    static const std::unordered_set<TokenType> prefix_ops = {
        TokenType::MINUS, TokenType::PLUS,
        TokenType::BANG,  TokenType::TILDE,
        TokenType::AMP,   // address-of / deref
    };

    if (prefix_ops.count(peek().type)) {
        SourceLocation l = loc();
        TokenType op = advance().type;
        auto operand = parse_unary();
        return std::make_shared<UnaryExprNode>(token_to_unary_op(op), std::move(operand), l);
    }

    return parse_postfix();
}

ExpressionPtr Parser::parse_postfix() {
    /*
    Handles (left-to-right):
      expr++  expr--          postfix increment/decrement
      expr(args)              function call
      expr[idx]               index
      expr.name               member access
      expr::name              static / namespace access
      expr<T, U>              generic instantiation (only if expr is a NameExpression)
    */
    auto expr = parse_primary();

    while (true) {
        SourceLocation l = loc();

        if (peek().type == TokenType::LPAREN) {
            advance();
            auto args = parse_call_arguments();
            expect(TokenType::RPAREN, "Expected ')' after arguments");
            expr = std::make_shared<CallExprNode>(std::move(expr), std::move(args), l);
        }
        else if (peek().type == TokenType::LBRACKET) {
            advance();
            auto index = parse_expression();
            expect(TokenType::RBRACKET, "Expected ']' after index");
            expr = std::make_shared<IndexExprNode>(std::move(expr), std::move(index), l);
        }
        else if (peek().type == TokenType::DOT) {
            advance();
            std::string member = expect(TokenType::IDENTIFIER,
                                        "Expected member name after '.'").lexeme;
            expr = std::make_shared<MemberAccessExpr>(
                std::move(expr), std::move(member), false, l);
        }
        else if (peek().type == TokenType::COLON_COLON) {
            advance();
            std::string member = expect(TokenType::IDENTIFIER,
                                        "Expected name after '::'").lexeme;
            expr = std::make_shared<MemberAccessExpr>(
                std::move(expr), std::move(member), true, l);
        }
        else if (peek().type == TokenType::LT && looks_like_generic_args(expr)) {
            advance();
            std::vector<TypeExprPtr> type_args;
            if (peek().type != TokenType::GT) {
                do {
                    type_args.push_back(parse_type_expression());
                } while (accept(TokenType::COMMA));
            }
            expect(TokenType::GT, "Expected '>' after generic type arguments");
            expr = std::make_shared<GenericInstantiationExpr>(
                std::move(expr), std::move(type_args), l);
        }
        else {
            break;
        }
    }

    return expr;
}

bool Parser::looks_like_generic_args(const ExpressionPtr& preceding) const {
    /*
    Only treat '<' as a generic opener if the preceding expression is a plain
    NameExpression. Uses a one-token heuristic on what follows '<':
      - IDENTIFIER, CONST, AMP  -> likely a type  -> YES
      - GT                      -> Pair<> empty generics -> YES
    */
    if (!preceding) return false;
    if (preceding->kind != NodeKind::NAME_EXPR &&
        preceding->kind != NodeKind::MEMBER_ACCESS) return false;

    TokenType next = peek_next().type;
    return next == TokenType::IDENTIFIER
        || next == TokenType::CONST
        || next == TokenType::AMP
        || next == TokenType::GT;
}

// ============================================================================
// CALL ARGUMENT PARSING
// func(a, b)  or  func(name: value, ...)
// Named arguments must all follow positional ones.
// ============================================================================

std::vector<CallExprNode::Argument> Parser::parse_call_arguments() {
    std::vector<CallExprNode::Argument> args;
    if (peek().type == TokenType::RPAREN) return args;

    bool seen_named = false;

    do {
        std::string name;
        if (peek().type == TokenType::IDENTIFIER && peek_next().type == TokenType::EQ) {
            name = advance().lexeme;  // consume identifier
            advance();                // consume '='
            seen_named = true;
        } else if (seen_named) {
            throw CompilerException(
                "Positional arguments cannot follow named arguments",
                peek().line, peek().column, filepath
            );
        }

        auto value = parse_expression();
        args.push_back({ std::move(value), std::move(name) });

    } while (accept(TokenType::COMMA));

    return args;
}

// ============================================================================
// PRIMARY EXPRESSION PARSING
// ============================================================================

ExpressionPtr Parser::parse_primary() {
    SourceLocation l = loc();

    switch (peek().type) {
        case TokenType::INT_LITERAL:
            return std::make_shared<LiteralIntNode>(advance().lexeme, l);

        case TokenType::FLOAT_LITERAL:
            return std::make_shared<LiteralFloatNode>(advance().lexeme, l);

        case TokenType::STRING_LITERAL:
            return std::make_shared<LiteralStringNode>(advance().lexeme, l);

        case TokenType::RAW_STRING_LITERAL:
            return std::make_shared<LiteralRawStringNode>(advance().lexeme, l);

        case TokenType::INTERP_STRING_START:
            return parse_interp_string();

        case TokenType::TRUE:
            advance();
            return std::make_shared<LiteralBoolNode>(true, l);

        case TokenType::FALSE:
            advance();
            return std::make_shared<LiteralBoolNode>(false, l);

        case TokenType::LBRACKET:
            return parse_array_literal();

        case TokenType::LBRACE:
            return parse_map_literal();

        case TokenType::SELF:
            advance();
            return std::make_shared<NameExpression>("Self", l);

        case TokenType::IDENTIFIER:
            return std::make_shared<NameExpression>(advance().lexeme, l);
        

        case TokenType::LPAREN:
            return parse_paren_or_lambda();

        // Generic lambda: <T>(value:T) => { ... }
        case TokenType::LT:
            if (peek_next().type == TokenType::IDENTIFIER) {
                return parse_lambda();
            }
            throw CompilerException(
                std::format("Expected expression, got '{}'", peek().lexeme),
                peek().line, peek().column, filepath
            );

        default:
            throw CompilerException(
                std::format("Expected expression, got '{}'", peek().lexeme),
                peek().line, peek().column, filepath
            );
    }
}

ExpressionPtr Parser::parse_array_literal() {
    SourceLocation l = loc();
    expect(TokenType::LBRACKET, "Expected '['");
    std::vector<ExpressionPtr> elements;

    if (peek().type != TokenType::RBRACKET) {
        do {
            elements.push_back(parse_expression());
        } while (accept(TokenType::COMMA));
    }

    expect(TokenType::RBRACKET, "Expected ']' to close array literal");
    return std::make_shared<LiteralArrayNode>(std::move(elements), l);
}

ExpressionPtr Parser::parse_map_literal() {
    SourceLocation l = loc();
    expect(TokenType::LBRACE, "Expected '{'");
    std::vector<std::pair<ExpressionPtr, ExpressionPtr>> pairs;

    if (peek().type != TokenType::RBRACE) {
        do {
            auto k = parse_expression();
            expect(TokenType::COLON, "Expected ':' after key expression");
            auto v = parse_expression();
            pairs.push_back(std::make_pair(k, v));
        } while (accept(TokenType::COMMA));
    }

    expect(TokenType::RBRACE, "Expected '}' to close map literal");
    return std::make_shared<LiteralMapNode>(std::move(pairs), l);
}

ExpressionPtr Parser::parse_interp_string() {
    /*
    $"Hello {name}, you are {age} years old!"

    Token stream from the lexer:
      INTERP_STRING_START
      INTERP_STRING_PART  ("Hello ")
      <expression tokens for `name`>
      INTERP_STRING_PART  (", you are ")
      <expression tokens for `age`>
      INTERP_STRING_PART  (" years old!")
      INTERP_STRING_END
    */
    SourceLocation l = loc();
    expect(TokenType::INTERP_STRING_START, "Expected interpolated string start");

    std::vector<LiteralInterpStringNode::StringPart> parts;

    while (peek().type != TokenType::INTERP_STRING_END && !is_at_end()) {
        if (peek().type == TokenType::INTERP_STRING_PART) {
            std::string text = advance().lexeme;
            parts.push_back({ false, std::move(text), nullptr });
        } else {
            auto expr = parse_expression();
            parts.push_back({ true, {}, std::move(expr) });
        }
    }

    expect(TokenType::INTERP_STRING_END, "Unterminated interpolated string");
    return std::make_shared<LiteralInterpStringNode>(std::move(parts), l);
}

bool Parser::looks_like_lambda() const {
    /*
    A '(' is a lambda if followed by:
      ')'  '=>'           -- zero-parameter:  () => ...
      IDENTIFIER  ':'     -- named parameter: (x:Int) => ...
    */
    if (peek().type != TokenType::LPAREN) return false;

    size_t after_lparen = idx + 1;

    // () => ...
    if (after_lparen < tokens.size() &&
        tokens[after_lparen].type == TokenType::RPAREN) {
        size_t after_rparen = after_lparen + 1;
        return after_rparen < tokens.size() &&
               tokens[after_rparen].type == TokenType::FAT_ARROW;
    }

    // (identifier : ...
    if (after_lparen < tokens.size() &&
        tokens[after_lparen].type == TokenType::IDENTIFIER) {
        size_t after_id = after_lparen + 1;
        return after_id < tokens.size() &&
               tokens[after_id].type == TokenType::COLON;
    }

    return false;
}

ExpressionPtr Parser::parse_paren_or_lambda() {
    if (looks_like_lambda()) {
        return parse_lambda();
    }

    SourceLocation l = loc();
    expect(TokenType::LPAREN, "Expected '('");
    auto expr = parse_expression();
    expect(TokenType::RPAREN, "Expected ')' after grouped expression");
    return expr;
}

ExpressionPtr Parser::parse_lambda() {
    /*
    (x:Int, y:Int) => Int { return x + y; }
    <T:Printable>(item:T) => String { return item.toString(); }
    () => Int { return 42; }
    <T:Numeric>(x:T) => { return x * x; }   -- inferred return type
    */
    SourceLocation l = loc();

    // Optional generic params: <T> or <T:Printable>
    GenericParams generic_params;
    if (peek().type == TokenType::LT) {
        advance();
        generic_params = parse_generic_params();
    }

    expect(TokenType::LPAREN, "Expected '(' for lambda parameters");
    std::vector<LambdaExprNode::Parameter> params;
    bool seen_default = false;

    if (peek().type != TokenType::RPAREN) {
        do {
            std::string pname = expect(TokenType::IDENTIFIER,
                                       "Expected parameter name").lexeme;
            expect(TokenType::COLON, "Expected ':' after parameter name");
            auto ptype = parse_type_expression();

            ExpressionPtr default_val;
            if (accept(TokenType::EQ)) {
                default_val = parse_expression();
                seen_default = true;
            } else if (seen_default) {
                throw CompilerException(
                    "Parameters without default values cannot follow parameters with defaults",
                    peek().line, peek().column, filepath
                );
            }

            params.push_back({ std::move(pname), std::move(ptype),
                               std::move(default_val) });
        } while (accept(TokenType::COMMA));
    }

    expect(TokenType::RPAREN, "Expected ')' after lambda parameters");
    expect(TokenType::FAT_ARROW, "Expected '=>' after lambda parameters");

    // Return type is optional — brace directly after '=>' means inferred
    TypeExprPtr return_type;
    if (peek().type != TokenType::LBRACE) {
        return_type = parse_type_expression();
    }

    auto body = parse_block();

    return std::make_shared<LambdaExprNode>(
        std::move(generic_params), std::move(params),
        std::move(return_type), std::move(body), l
    );
}

// ============================================================================
// STATEMENT PARSING
// ============================================================================

Ptr<BlockStatement> Parser::parse_block() {
    SourceLocation l = loc();
    expect(TokenType::LBRACE, "Expected '{' to open block");
    std::vector<StatementPtr> stmts;

    while (peek().type != TokenType::RBRACE && !is_at_end()) {
        stmts.push_back(parse_statement());
    }

    expect(TokenType::RBRACE, "Expected '}' to close block");
    return std::make_shared<BlockStatement>(std::move(stmts), l);
}

StatementPtr Parser::parse_statement() {
    // Consume any leading tags into the buffer
    while (peek().type == TokenType::HASH) {
        tag_buffer.push_back(parse_tag());
    }

    // Declarations consume the tag buffer
    if (accept(TokenType::LET)) {
        if (peek().type == TokenType::LPAREN) {
            return parse_destructure_decl();
        } else if (peek().type == TokenType::IDENTIFIER) {
            return parse_variable_decl();
        } else {
            SourceLocation l = loc();
            throw CompilerException(
                std::format("Expected either identifier or '(' for destrucuting, got {}", peek().lexeme), 
                l.line, l.column, l.file_name);    
        }
    }
    if (accept(TokenType::FUNC))   return parse_function_decl();
    if (accept(TokenType::STRUCT)) return parse_struct_decl();
    if (accept(TokenType::TRAIT))  return parse_trait_decl();
    if (accept(TokenType::TYPE))   return parse_type_alias_decl();
    if (accept(TokenType::ENUM))   return parse_enum_decl();

    // Tags can also appear before operator overloads inside structs
    if (peek().type == TokenType::OPERATOR) {
        advance();
        return parse_operator_overload();
    }

    // If tags are still buffered here, nothing valid can follow them
    if (!tag_buffer.empty()) {
        throw CompilerException(
            "Compiler tags can only precede declarations (let, func, struct, enum)",
            peek().line, peek().column, filepath
        );
    }

    // Control flow
    if (accept(TokenType::IF))      return parse_if_stmt();
    if (accept(TokenType::WHILE))   return parse_while_stmt();
    if (accept(TokenType::DO))      return parse_do_while_stmt();
    if (accept(TokenType::FOR))     return parse_for_stmt();
    if (accept(TokenType::FOREACH)) return parse_foreach_stmt();

    // Exception handling
    if (accept(TokenType::TRY))   return parse_try_catch_stmt();
    if (accept(TokenType::THROW)) return parse_throw_stmt();

    // Jump statements
    if (accept(TokenType::RETURN))   return parse_return_stmt();
    if (accept(TokenType::BREAK))    return parse_break_stmt();
    if (accept(TokenType::CONTINUE)) return parse_continue_stmt();

    // Module system
    if (accept(TokenType::IMPORT)) return parse_import_decl();
    if (accept(TokenType::EXPORT)) return parse_export_decl();

    // Expression statement
    SourceLocation l = loc();
    ExpressionPtr expr = accept(TokenType::UNPACK) ? parse_unpack_expr() : parse_expression();
    expect(TokenType::SEMICOLON, "Expected ';' after expression statement");
    return std::make_shared<ExpressionStmt>(std::move(expr), l);
}

// ============================================================================
// DECLARATION PARSING
// ============================================================================

Ptr<VariableDeclNode> Parser::parse_variable_decl() {
    /*
    let x: Int = 42;
    let name: String;           -- type only, no initialiser
    let y = someExpression;    -- initialiser only, type inferred
    */
    SourceLocation l = loc();
    auto tags = flush_tags();

    std::string name = expect(TokenType::IDENTIFIER, "Expected variable name").lexeme;

    TypeExprPtr type;
    if (accept(TokenType::COLON)) {
        type = parse_type_expression();
    }

    ExpressionPtr init;
    if (accept(TokenType::EQ)) {
        init = parse_expression();
    }

    if (!type && !init) {
        throw CompilerException(
            "Variable declaration must have a type annotation, an initialiser, or both",
            l.line, l.column
        );
    }

    expect(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_shared<VariableDeclNode>(
        std::move(name), std::move(type), std::move(init), std::move(tags), l);
}

Ptr<DestructureDeclNode> Parser::parse_destructure_decl() {
    /*
    let (first, second) = expr;
    */
    SourceLocation l = loc();
    auto tags = flush_tags();
    

    advance(); // consume '('

    std::vector<DestructureDeclNode::Binding> bindings;
    do {
        if (peek().type == TokenType::IDENTIFIER) {
            std::string name = advance().lexeme;
            TypeExprPtr type;
            if (accept(TokenType::COLON)) {
                type = parse_type_expression();
            }
            bindings.emplace_back(std::move(name), std::move(type));
        } else {
            throw CompilerException(
                std::format("Expected binding name in destructure, got '{}'",
                            peek().lexeme),
                peek().line, peek().column, filepath);
        }
    } while (accept(TokenType::COMMA));

    expect(TokenType::RPAREN, "Expected ')' to close tuple destructure");
    
    expect(TokenType::EQ, "Expected initialiser for tuple unpacking");
    ExpressionPtr initialiser = parse_expression();

    expect(TokenType::SEMICOLON, "Expected ';' after destructure declaration");

    return std::make_shared<DestructureDeclNode>(
        std::move(bindings), std::move(initialiser), std::move(tags), l);
}

Ptr<FunctionDeclNode> Parser::parse_function_decl() {
    /*
    func add(a:Int, b:Int) -> Int { return a + b; }
    func generic<T:Printable>(value:T) -> String { ... }
    */
    SourceLocation l = loc();
    auto tags = flush_tags();

    std::string name = expect(TokenType::IDENTIFIER, "Expected function name").lexeme;

    GenericParams generic_params;
    if (peek().type == TokenType::LT) {
        advance();
        generic_params = parse_generic_params();
    }

    expect(TokenType::LPAREN, "Expected '(' after function name");
    std::vector<FunctionDeclNode::Parameter> params;

    if (peek().type != TokenType::RPAREN) {
        bool seen_default = false;
        do {
            std::string pname = expect(TokenType::IDENTIFIER,
                                       "Expected parameter name").lexeme;
            expect(TokenType::COLON, "Expected ':' after parameter name");
            auto ptype = parse_type_expression();

            ExpressionPtr default_val;
            if (accept(TokenType::EQ)) {
                default_val = parse_expression();
                seen_default = true;
            } else if (seen_default) {
                throw CompilerException(
                    "Non-defaulted parameter cannot follow a defaulted one",
                    peek().line, peek().column, filepath
                );
            }

            params.push_back({ std::move(pname), std::move(ptype),
                               std::move(default_val) });
        } while (accept(TokenType::COMMA));
    }

    expect(TokenType::RPAREN, "Expected ')' after parameters");

    TypeExprPtr return_type;
    if (accept(TokenType::ARROW)) {
        return_type = parse_type_expression();
    }

    Ptr<BlockStatement> body;
    if (peek().type == TokenType::LBRACE) {
        body = parse_block();
    } else {
        expect(TokenType::SEMICOLON,
               "Expected '{' for function body or ';' for prototype");
    }

    return std::make_shared<FunctionDeclNode>(
        std::move(name), std::move(generic_params),
        std::move(params), std::move(return_type), std::move(body),
        std::move(tags), l
    );
}

Ptr<OperatorOverloadNode> Parser::parse_operator_overload() {
    /*
    operator+(other:Complex) -> Complex { ... }
    operator-() -> Complex { ... }     -- unary
    operator[](idx:Int) -> T { ... }

    ENTRY: 'operator' already consumed
    */
    SourceLocation l = loc();
    auto tags = flush_tags();

    TokenType op_token = peek().type;
    advance();

    if (op_token == TokenType::LBRACKET) {
        expect(TokenType::RBRACKET, "Expected ']' for operator[]");
    }

    expect(TokenType::LPAREN, "Expected '(' after operator");
    std::vector<FunctionDeclNode::Parameter> params;

    if (peek().type != TokenType::RPAREN) {
        do {
            std::string pname = expect(TokenType::IDENTIFIER,
                                       "Expected parameter name").lexeme;
            expect(TokenType::COLON, "Expected ':' after parameter name");
            auto ptype = parse_type_expression();
            params.push_back({ std::move(pname), std::move(ptype), nullptr });
        } while (accept(TokenType::COMMA));
    }

    expect(TokenType::RPAREN, "Expected ')' after operator parameters");

    TypeExprPtr return_type;
    if (accept(TokenType::ARROW)) {
        return_type = parse_type_expression();
    }

    bool is_unary = params.empty();
    auto op_type  = token_to_operator_type(op_token, is_unary);

    Ptr<BlockStatement> body;
    if (peek().type == TokenType::LBRACE) {
        body = parse_block();
    } else {
        expect(TokenType::SEMICOLON,
               "Expected '{' for operator body or ';' for prototype");
    }

    return std::make_shared<OperatorOverloadNode>(
        op_type, std::move(params), std::move(return_type),
        std::move(body), std::move(tags), l
    );
}

Ptr<StructDeclNode> Parser::parse_struct_decl() {
    /*
    struct Point { let x:Float; let y:Float; }
    struct Pair<T, U> { ... }

    Members can be: let, func, operator, type  (and private: / public: toggles)
    */
    SourceLocation l = loc();
    auto tags = flush_tags();

    std::string name = expect(TokenType::IDENTIFIER, "Expected struct name").lexeme;

    GenericParams generic_params;
    if (peek().type == TokenType::LT) {
        advance();
        generic_params = parse_generic_params();
    }

    std::vector<TraitConstraint> constraints;
    if (accept(TokenType::IMPL)) {
        do {
            std::string trait_name = expect(TokenType::IDENTIFIER, "Expected identifier after 'impl'").lexeme;
            std::vector<TypeExprPtr> args;

            if (peek().type == TokenType::LT) {
                    advance();
                    do {
                        args.push_back(parse_type_expression());
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

                constraints.emplace_back(trait_name, std::move(args));            
        } while (accept(TokenType::COMMA));
    }

    expect(TokenType::LBRACE, "Expected '{' after struct name");

    std::vector<StructDeclNode::Member> members;
    bool is_private = false;

    while (peek().type != TokenType::RBRACE && !is_at_end()) {

        // Consume member-level tags into the shared buffer
        while (peek().type == TokenType::HASH) {
            tag_buffer.push_back(parse_tag());
        }

        // Access control toggle
        if (peek().type == TokenType::PRIVATE || peek().type == TokenType::PUBLIC) {
            is_private = (advance().type == TokenType::PRIVATE);
            expect(TokenType::COLON, "Expected ':' after access modifier");
            continue;
        }

        Ptr<Declaration> decl;
        if (accept(TokenType::LET)) {
            decl = parse_variable_decl();
        } else if (accept(TokenType::FUNC)) {
            decl = parse_function_decl();
        } else if (peek().type == TokenType::OPERATOR) {
            advance();
            decl = parse_operator_overload();
        } else if (accept(TokenType::TYPE)) {
            decl = parse_type_alias_decl();
        } else {
            throw CompilerException(
                std::format("Expected struct member declaration, got '{}'",
                            peek().lexeme),
                peek().line, peek().column, filepath
            );
        }

        members.push_back({ std::move(decl), is_private });
    }

    expect(TokenType::RBRACE, "Expected '}' to close struct");

    return std::make_shared<StructDeclNode>(
        std::move(name), std::move(generic_params), std::move(constraints),
        std::move(members), std::move(tags), l
    );
}

Ptr<TraitDeclNode> Parser::parse_trait_decl() {
    /*
    trait Printable {
        func toString() -> String;
    }
    trait MyRequirements {
        func doThis() -> String;
        #staticmember
        func say(String) -> Void;     -- static method requirement (old syntax kept for compat)
        Self:Printable;                -- type constraint
    }
    */
    SourceLocation l = loc();

    if (!tag_buffer.empty()) {
        throw CompilerException("Compiler tags cannot be applied to traits",
                          peek().line, peek().column, filepath);
    }

    std::string name = expect(TokenType::IDENTIFIER, "Expected trait name").lexeme;

    GenericParams generic_params;
    if (peek().type == TokenType::LT) {
        advance();
        generic_params = parse_generic_params();
    }

    expect(TokenType::LBRACE, "Expected '{' after trait name");

    std::vector<Ptr<TraitDeclNode::Requirement>> requirements;
    TagList method_tags;

    while (peek().type != TokenType::RBRACE && !is_at_end()) {

        method_tags.clear();
        while (peek().type == TokenType::HASH) {
            method_tags.push_back(parse_tag());
        }

        auto only_staticmember_allowed = [&]() -> bool {
            if (method_tags.size() > 1) {
                throw CompilerException(
                    "Only #staticmember can be applied to a trait requirement",
                    peek().line, peek().column, filepath);
            }
            if (method_tags.size() == 1) {
                if (method_tags[0]->kind != NodeKind::TAG_STATICMEMBER) {
                    throw CompilerException(
                        std::format("Tag '{}' is not valid on a trait requirement",
                                    tag_kind_to_string(method_tags[0]->kind)),
                        peek().line, peek().column, filepath);
                }
                return true;
            }
            return false;
        };

        if (accept(TokenType::FUNC)) {
            bool is_static = only_staticmember_allowed();
            std::string method_name = expect(TokenType::IDENTIFIER,
                                             "Expected method name").lexeme;

            // Generic params on a trait method requirement (rare but allowed)
            GenericParams method_generics;
            if (peek().type == TokenType::LT) {
                advance();
                method_generics = parse_generic_params();
            }

            expect(TokenType::LPAREN, "Expected '(' after method name");
            std::vector<TypeExprPtr> param_types;
            if (peek().type != TokenType::RPAREN) {
                do {
                    // Accept optional "name:" prefix; we only care about the type
                    if (peek().type == TokenType::IDENTIFIER &&
                        peek_next().type == TokenType::COLON) {
                        advance(); advance();  // skip name:
                    }
                    param_types.push_back(parse_type_expression());
                } while (accept(TokenType::COMMA));
            }
            expect(TokenType::RPAREN, "Expected ')' after method parameters");

            TypeExprPtr return_type;
            if (accept(TokenType::ARROW)) {
                return_type = parse_type_expression();
            }
            expect(TokenType::SEMICOLON, "Expected ';' after method requirement");

            requirements.push_back(std::make_shared<TraitDeclNode::MethodRequirement>(
                std::move(method_name), std::move(param_types),
                std::move(return_type), is_static));

        } else if (accept(TokenType::OPERATOR)) {
            if (!method_tags.empty()) {
                throw CompilerException(
                    "Tags cannot be applied to operator requirements in traits",
                    peek().line, peek().column, filepath);
            }

            TokenType op_token = peek().type;
            advance();

            if (op_token == TokenType::LBRACKET) {
                expect(TokenType::RBRACKET, "Expected ']' for operator[]");
            }

            expect(TokenType::LPAREN, "Expected '(' after operator in trait");
            std::vector<TypeExprPtr> param_types;
            if (peek().type != TokenType::RPAREN) {
                do {
                    if (peek().type == TokenType::IDENTIFIER &&
                        peek_next().type == TokenType::COLON) {
                        advance(); advance();
                    }
                    param_types.push_back(parse_type_expression());
                } while (accept(TokenType::COMMA));
            }
            expect(TokenType::RPAREN, "Expected ')' after operator parameters");

            TypeExprPtr return_type;
            if (accept(TokenType::ARROW)) {
                return_type = parse_type_expression();
            }

            auto op_type = token_to_operator_type(op_token, param_types.empty());
            expect(TokenType::SEMICOLON, "Expected ';' after operator requirement");

            requirements.push_back(std::make_shared<TraitDeclNode::OperatorRequirement>(
                op_type, std::move(param_types), std::move(return_type)));

        } else if (peek().type == TokenType::IDENTIFIER ||
                   peek().type == TokenType::SELF) {
            // Type constraint: Self:Printable;  or  T:Comparable<U>;
            if (!method_tags.empty()) {
                throw CompilerException(
                    "Tags cannot be applied to type constraints in traits",
                    peek().line, peek().column, filepath);
            }

            std::string type_param = advance().lexeme;
            expect(TokenType::COLON, "Expected ':' in type constraint");

            // Parse the constraint type expression
            auto constraint_type = parse_type_expression();
            expect(TokenType::SEMICOLON, "Expected ';' after type constraint");

            // Extract trait name and args from the parsed type
            std::string trait_name;
            std::vector<TypeExprPtr> trait_args;
            if (auto simple = std::dynamic_pointer_cast<SimpleType>(constraint_type)) {
                trait_name = simple->name;
            } else if (auto generic = std::dynamic_pointer_cast<GenericType>(constraint_type)) {
                trait_name = generic->base_name;
                trait_args = std::move(generic->type_arguments);
            } else {
                throw CompilerException("Invalid trait constraint", peek().line, peek().column, filepath);
            }

            requirements.push_back(std::make_shared<TraitDeclNode::TypeRequirement>(
                std::move(type_param), TraitConstraint(std::move(trait_name), std::move(trait_args))));

        } else {
            throw CompilerException(
                std::format("Expected trait requirement, got '{}'", peek().lexeme),
                peek().line, peek().column, filepath
            );
        }
    }

    expect(TokenType::RBRACE, "Expected '}' to close trait");

    return std::make_shared<TraitDeclNode>(
        std::move(name), std::move(generic_params), std::move(requirements), l
    );
}

Ptr<TypeAliasDeclNode> Parser::parse_type_alias_decl() {
    /*
    type Point2D = Pair<Float, Float>;
    type GenericArray<T> = Array<T>;

    ENTRY: 'type' already consumed
    */
    SourceLocation l = loc();

    if (!tag_buffer.empty()) {
        throw CompilerException("Compiler tags cannot be applied to type aliases",
                          peek().line, peek().column, filepath);
    }

    std::string alias = expect(TokenType::IDENTIFIER,
                               "Expected type alias name").lexeme;

    GenericParams generic_params;
    if (peek().type == TokenType::LT) {
        advance();
        generic_params = parse_generic_params();
    }

    expect(TokenType::EQ, "Expected '=' after type alias name");
    auto target = parse_type_expression();
    expect(TokenType::SEMICOLON, "Expected ';' after type alias");

    std::vector<std::string> param_names;
    for (const auto& p : generic_params) param_names.push_back(p.name);

    return std::make_shared<TypeAliasDeclNode>(
        std::move(alias), std::move(param_names), std::move(target), l
    );
}

Ptr<EnumDeclNode> Parser::parse_enum_decl() {
    /*
    enum Colours { RED, ORANGE, YELLOW, GREEN, BLUE }

    ENTRY: 'enum' already consumed
    */
    SourceLocation l = loc();

    if (!tag_buffer.empty()) {
        throw CompilerException("Compiler tags cannot be applied to enums",
                          peek().line, peek().column, filepath);
    }

    std::string name = expect(TokenType::IDENTIFIER, "Expected enum name").lexeme;

    GenericParams generic_params;
    if (peek().type == TokenType::LT) {
        advance();
        generic_params = parse_generic_params();
    }

    expect(TokenType::LBRACE, "Expected '{' after enum name");
    std::vector<std::string> variants;

    // BUG FIX: old code called expect(IDENTIFIER) then immediately called it
    // again inside the loop, eating the first variant and skipping to the second.
    if (peek().type != TokenType::RBRACE) {
        do {
            variants.push_back(
                expect(TokenType::IDENTIFIER, "Expected enum variant name").lexeme);
        } while (accept(TokenType::COMMA) && peek().type != TokenType::RBRACE);
    }

    expect(TokenType::RBRACE, "Expected '}' to close enum");

    return std::make_shared<EnumDeclNode>(std::move(name), std::move(variants), l);
}

Ptr<ScopeDeclNode> Parser::parse_scope_decl() {
    /*
    scope MyScope {
        func myFunc() -> String { return "Hello, World!"; }
    }
    */

    SourceLocation l = loc();

    if (!tag_buffer.empty()) {
        throw CompilerException("Compiler tags cannot be applied to enums",
                          peek().line, peek().column, filepath);
    }

    std::string name = expect(TokenType::IDENTIFIER, "Expected scope name").lexeme;

    expect(TokenType::LBRACE, "Expected '{' to start scope block");

    std::vector<DeclarationPtr> decls;
    while (peek().type != TokenType::RBRACE && !is_at_end()) {
        Ptr<Declaration> decl;
        if (accept(TokenType::LET)) {
            if (peek().type == TokenType::LPAREN || peek().type == TokenType::LBRACKET) {
                decl = parse_destructure_decl();
            } else {
                decl = parse_variable_decl();
        }
        } else if (accept(TokenType::FUNC)) {
            decl = parse_function_decl();
        } else if (accept(TokenType::STRUCT)) {
            decl = parse_struct_decl();
        } else if (accept(TokenType::TYPE)) {
            decl = parse_type_alias_decl();
        } else if (accept(TokenType::ENUM)) {
            decl = parse_enum_decl();
        } else if (accept(TokenType::TRAIT)) {
            decl = parse_trait_decl();
        } else if (accept(TokenType::SCOPE)) {
            decl = parse_scope_decl();
        } else {
            throw CompilerException(
                std::format("Expected declaration, got '{}'",
                            peek().lexeme),
                peek().line, peek().column, filepath
            );
        }
        decls.push_back(std::move(decl));
    }

    return std::make_shared<ScopeDeclNode>(std::move(name), std::move(decls), loc);
}

// ============================================================================
// CONTROL FLOW
// ============================================================================

StatementPtr Parser::parse_if_stmt() {
    /*
    if (cond) { ... }
    if (cond) { ... } elif (cond2) { ... } else { ... }
    Note: conditions do NOT require parens in the grammar — the parens
    are just a parenthesised expression and handled naturally.

    ENTRY: 'if' already consumed
    */
    SourceLocation l = loc();
    auto condition = parse_expression();
    auto then_block = parse_block();

    std::vector<std::pair<ExpressionPtr, Ptr<BlockStatement>>> elif_branches;
    while (accept(TokenType::ELIF)) {
        auto elif_cond  = parse_expression();
        auto elif_block = parse_block();
        elif_branches.push_back({ std::move(elif_cond), std::move(elif_block) });
    }

    Ptr<BlockStatement> else_block;
    if (accept(TokenType::ELSE)) {
        else_block = parse_block();
    }

    return std::make_shared<IfStatementNode>(
        std::move(condition), std::move(then_block),
        std::move(elif_branches), std::move(else_block), l
    );
}

StatementPtr Parser::parse_while_stmt() {
    // while (cond) { ... }
    // ENTRY: 'while' already consumed
    SourceLocation l = loc();
    expect(TokenType::LPAREN, "Expected '(' after 'while'");
    auto condition = parse_expression();
    expect(TokenType::RPAREN, "Expected ')' after while condition");
    auto body = parse_block();
    return std::make_shared<WhileLoopNode>(std::move(condition), std::move(body), l);
}

StatementPtr Parser::parse_do_while_stmt() {
    /*
    do { ... } while (cond);

    BUG FIX: old code expected '(' then WHILE then cond,
    i.e. do { } (while cond) which is wrong.
    The actual syntax from test.txt is: do { } while (condition);

    ENTRY: 'do' already consumed
    */
    SourceLocation l = loc();
    auto body = parse_block();
    expect(TokenType::WHILE, "Expected 'while' after do block");
    expect(TokenType::LPAREN, "Expected '(' after 'while' in do-while");
    auto condition = parse_expression();
    expect(TokenType::RPAREN, "Expected ')' after do-while condition");
    accept(TokenType::SEMICOLON);  // optional trailing ;

    return std::make_shared<DoWhileLoopNode>(std::move(body), std::move(condition), l);
}

StatementPtr Parser::parse_for_stmt() {
    // for (let i:Int = 0; i < limit; i++) { ... }
    // ENTRY: 'for' already consumed
    SourceLocation l = loc();
    expect(TokenType::LPAREN, "Expected '(' after 'for'");

    StatementPtr init;
    if (accept(TokenType::LET)) {
        init = parse_variable_decl();  // already consumes ';'
    } else {
        auto expr = parse_expression();
        expect(TokenType::SEMICOLON, "Expected ';' after for initialiser");
        init = std::make_shared<ExpressionStmt>(std::move(expr), l);
    }

    auto condition = parse_expression();
    expect(TokenType::SEMICOLON, "Expected ';' after for condition");

    auto increment = parse_expression();
    expect(TokenType::RPAREN, "Expected ')' after for increment");

    auto body = parse_block();

    return std::make_shared<ForLoopNode>(
        std::move(init), std::move(condition), std::move(increment),
        std::move(body), l
    );
}

StatementPtr Parser::parse_foreach_stmt() {
    // foreach (item in collection) { ... }
    // foreach (item:Int in collection) { ... }
    // ENTRY: 'foreach' already consumed
    SourceLocation l = loc();
    expect(TokenType::LPAREN, "Expected '(' after 'foreach'");

    std::string iter_name = expect(TokenType::IDENTIFIER,
                                   "Expected iterator variable name").lexeme;

    TypeExprPtr iter_type;
    if (accept(TokenType::COLON)) {
        iter_type = parse_type_expression();
    }

    expect(TokenType::IN, "Expected 'in' after foreach variable");
    auto iterable = parse_expression();
    expect(TokenType::RPAREN, "Expected ')' after foreach iterable");

    auto body = parse_block();

    return std::make_shared<ForEachLoopNode>(
        std::move(iter_name), std::move(iter_type),
        std::move(iterable), std::move(body), l
    );
}

// ============================================================================
// EXCEPTION HANDLING
// ============================================================================

StatementPtr Parser::parse_try_catch_stmt() {
    /*
    try { ... }
    catch (e:IOError) { ... }
    catch (e:MathError) { ... }
    finally { ... }   -- optional

    ENTRY: 'try' already consumed
    */
    SourceLocation l = loc();
    auto try_block = parse_block();

    std::vector<TryCatchStatementNode::CatchClause> clauses;
    while (accept(TokenType::CATCH)) {
        expect(TokenType::LPAREN, "Expected '(' after 'catch'");
        std::string ename = expect(TokenType::IDENTIFIER,
                                   "Expected exception variable name").lexeme;
        expect(TokenType::COLON, "Expected ':' after exception variable");
        auto etype = parse_type_expression();
        expect(TokenType::RPAREN, "Expected ')' after catch type");
        auto handler = parse_block();
        clauses.push_back({ std::move(ename), std::move(etype), std::move(handler) });
    }

    if (clauses.empty()) {
        throw CompilerException("try block must have at least one catch clause",
                          l.line, l.column);
    }

    Ptr<BlockStatement> finally_block;
    if (accept(TokenType::FINALLY)) {
        finally_block = parse_block();
    }

    return std::make_shared<TryCatchStatementNode>(
        std::move(try_block), std::move(clauses), std::move(finally_block), l
    );
}

StatementPtr Parser::parse_throw_stmt() {
    // throw IOError("Something went wrong");
    // ENTRY: 'throw' already consumed
    SourceLocation l = loc();
    auto exception = parse_expression();
    expect(TokenType::SEMICOLON, "Expected ';' after throw");
    return std::make_shared<ThrowStatementNode>(std::move(exception), l);
}

// ============================================================================
// JUMP STATEMENTS
// ============================================================================

StatementPtr Parser::parse_return_stmt() {
    SourceLocation l = loc();
    ExpressionPtr value;
    if (peek().type != TokenType::SEMICOLON) {
        value = parse_expression();
    }
    expect(TokenType::SEMICOLON, "Expected ';' after return");
    return std::make_shared<ReturnStatementNode>(std::move(value), l);
}

StatementPtr Parser::parse_break_stmt() {
    SourceLocation l = loc();
    expect(TokenType::SEMICOLON, "Expected ';' after break");
    return std::make_shared<BreakStatementNode>(l);
}

StatementPtr Parser::parse_continue_stmt() {
    SourceLocation l = loc();
    expect(TokenType::SEMICOLON, "Expected ';' after continue");
    return std::make_shared<ContinueStatementNode>(l);
}

// ============================================================================
// MODULE SYSTEM
// ============================================================================

StatementPtr Parser::parse_import_decl() {
    /*
    import "std/math" { sin, cos };
    import "path/to/file" as m;
    import "path/to/file" { sym1 as alias1, sym2 };

    ENTRY: 'import' already consumed
    */
    SourceLocation l = loc();

    std::string path = expect(TokenType::STRING_LITERAL,
                              "Expected quoted module path after 'import'").lexeme;

    std::optional<std::string> module_alias;
    std::vector<ImportDeclNode::ImportedSymbol> symbols;

    if (accept(TokenType::AS)) {
        module_alias = expect(TokenType::IDENTIFIER,
                              "Expected alias name after 'as'").lexeme;
    } else if (accept(TokenType::LBRACE)) {
        if (peek().type != TokenType::RBRACE) {
            do {
                std::string orig = expect(TokenType::IDENTIFIER,
                                          "Expected symbol name").lexeme;
                std::string alias;
                if (accept(TokenType::AS)) {
                    alias = expect(TokenType::IDENTIFIER,
                                   "Expected alias after 'as'").lexeme;
                }
                symbols.push_back({ std::move(orig), std::move(alias) });
            } while (accept(TokenType::COMMA));
        }
        expect(TokenType::RBRACE, "Expected '}' after import list");
    }

    expect(TokenType::SEMICOLON, "Expected ';' after import");
    return std::make_shared<ImportDeclNode>(
        std::move(path), std::move(module_alias), std::move(symbols), l
    );
}

StatementPtr Parser::parse_export_decl() {
    // export { Point3D, add, Colours };
    // ENTRY: 'export' already consumed
    SourceLocation l = loc();
    std::vector<std::string> symbols;

    expect(TokenType::LBRACE, "Expected '{' after 'export'");
    if (peek().type != TokenType::RBRACE) {
        do {
            symbols.push_back(
                expect(TokenType::IDENTIFIER, "Expected symbol name").lexeme
            );
        } while (accept(TokenType::COMMA));
    }
    expect(TokenType::RBRACE, "Expected '}' after export list");
    accept(TokenType::SEMICOLON);

    return std::make_shared<ExportDeclNode>(std::move(symbols), l);
}

// ============================================================================
// TOP-LEVEL ENTRY POINT
// ============================================================================

Ptr<BlockStatement> Parser::parse() {
    if (result) return result;

    if (tokens.empty() || peek().type == TokenType::EOF_TOKEN) {
        return std::make_shared<BlockStatement>(
            std::vector<StatementPtr>{}, SourceLocation{1, 1});
    }

    SourceLocation l = loc();
    std::vector<StatementPtr> stmts;

    while (!is_at_end()) {
        stmts.push_back(parse_statement());
    }

    stmts.push_back(std::make_shared<EOFStatement>(loc()));
    result = std::make_shared<BlockStatement>(std::move(stmts), l);
    return result;
}

} // namespace espresso_compiler