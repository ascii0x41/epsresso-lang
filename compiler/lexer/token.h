#pragma once

#include <string>
#include <vector>

namespace espresso_compiler {

enum class TokenType {
    // ========== KEYWORDS ==========
    // Declaration keywords
    LET,
    FUNC,
    OPERATOR,
    STRUCT,
    IMPL,
    TYPE,
    TRAIT,
    WHERE,
    ENUM,
    SCOPE,
    UNPACK,
    USE,
    
    // Control flow
    IF,
    ELIF,
    ELSE,
    WHILE,
    DO,
    FOR,
    FOREACH,
    IN,
    
    // Jump statements
    RETURN,
    BREAK,
    CONTINUE,
    THROW,
    
    // Exception handling
    TRY,
    CATCH,
    FINALLY,
    
    // Module system
    IMPORT,
    EXPORT,
    AS,
    
    // Access modifiers
    PUBLIC,
    PRIVATE,
    
    // Type qualifiers
    CONST,
    REF,
    
    // Literals
    TRUE,
    FALSE,
    SELF,
    
    // ========== COMPILER TAGS (# prefixed) ==========
    TAG_STATICMEMBER,   // #staticmember
    TAG_CONSTMETHOD,    // #constmethod
    TAG_DEPRECATED,     // #deprecated
    TAG_DOCSTRING,      // #docstring
    TAG_CONSTEXPR,      // #constexpr
    TAG_INLINE,         // #inline
    
    // ========== IDENTIFIERS & LITERALS ==========
    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    COMPLEX_LITERAL,
    STRING_LITERAL,
    RAW_STRING_LITERAL,
    
    // Interpolated strings
    INTERP_STRING_START,      // $"
    INTERP_STRING_PART,       // text part between expressions
    INTERP_STRING_END,        // closing "
    
    // ========== OPERATORS ==========
    // Assignment
    EQ,                       // =
    PLUS_EQ,                  // +=
    MINUS_EQ,                 // -=
    STAR_EQ,                  // *=
    SLASH_EQ,                 // /=
    PERCENT_EQ,               // %=
    AMP_EQ,                   // &=
    PIPE_EQ,                  // |=
    CARET_EQ,                 // ^=
    TILDE_EQ,                 // ~=
    LT_LT_EQ,                 // <<=
    GT_GT_EQ,                 // >>=
    
    // Arithmetic
    PLUS,                     // +
    MINUS,                    // -
    STAR,                     // *
    SLASH,                    // /
    PERCENT,                  // %
    
    // Comparison
    EQ_EQ,                    // ==
    BANG_EQ,                  // !=
    LT,                       // <
    GT,                       // >
    LTE,                      // <=
    GTE,                      // >=
    
    // Logical
    BANG,                     // !
    AND,                      // &&
    OR,                       // ||
    
    // Bitwise
    AMP,                      // &
    PIPE,                     // |
    TILDE,                    // ~
    CARET,                    // ^
    LT_LT,                    // <<
    GT_GT,                    // >>
    
    // Other operators
    QUESTION,                 // ? (ternary)
    AT,                      // @ (pointers) cuz like... at... as in address (im so smart)
    
    // ========== DELIMITERS ==========
    LPAREN,                   // (
    RPAREN,                   // )
    LBRACE,                   // {
    RBRACE,                   // }
    LBRACKET,                 // [
    RBRACKET,                 // ]
    
    // ========== PUNCTUATION ==========
    COMMA,                    // ,
    SEMICOLON,                // ;
    COLON,                    // :
    COLON_COLON,              // ::
    DOT,                      // .
    DOT_DOT,                  // ..
    DOT_DOT_DOT,              // ...
    ARROW,                    // ->
    FAT_ARROW,                // =>
    HASH,                     // #
    
    // ========== SPECIAL ==========
    EOF_TOKEN
};

// Convert TokenType to string for debugging
inline std::string token_type_to_string(TokenType t) {
    switch(t) {
        // Keywords
        case TokenType::LET: return "LET";
        case TokenType::FUNC: return "FUNC";
        case TokenType::OPERATOR: return "OPERATOR";
        case TokenType::STRUCT: return "STRUCT";
        case TokenType::IMPL: return "IMPL";
        case TokenType::ENUM: return "ENUM";
        case TokenType::TYPE: return "TYPE";
        case TokenType::TRAIT: return "TRAIT";
        case TokenType::WHERE: return "WHERE";
        case TokenType::SCOPE: return "SCOPE";
        case TokenType::IF: return "IF";
        case TokenType::ELIF: return "ELIF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::DO: return "DO";
        case TokenType::FOR: return "FOR";
        case TokenType::FOREACH: return "FOREACH";
        case TokenType::IN: return "IN";
        case TokenType::RETURN: return "RETURN";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::THROW: return "THROW";
        case TokenType::TRY: return "TRY";
        case TokenType::CATCH: return "CATCH";
        case TokenType::FINALLY: return "FINALLY";
        case TokenType::IMPORT: return "IMPORT";
        case TokenType::EXPORT: return "EXPORT";
        case TokenType::AS: return "AS";
        case TokenType::UNPACK: return "UNPACK";
        case TokenType::USE: return "USE";
        case TokenType::PUBLIC: return "PUBLIC";
        case TokenType::PRIVATE: return "PRIVATE";
        case TokenType::CONST: return "CONST";
        case TokenType::REF: return "REF";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::SELF: return "SELF";
        
        // Compiler directives
        case TokenType::TAG_STATICMEMBER: return "TAG_STATICMEMBER";
        case TokenType::TAG_CONSTMETHOD: return "TAG_CONSTMETHOD";
        case TokenType::TAG_DEPRECATED: return "TAG_DEPRECATED";
        case TokenType::TAG_CONSTEXPR: return "TAG_CONSTEXPR";
        case TokenType::TAG_INLINE: return "TAG_INLINE";
        case TokenType::TAG_DOCSTRING: return "TAG_DOCSTRING";
        
        // Identifiers & literals
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::INT_LITERAL: return "INT_LITERAL";
        case TokenType::FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TokenType::STRING_LITERAL: return "STRING_LITERAL";
        case TokenType::RAW_STRING_LITERAL: return "RAW_STRING_LITERAL";
        case TokenType::INTERP_STRING_START: return "INTERP_STRING_START";
        case TokenType::INTERP_STRING_PART: return "INTERP_STRING_PART";
        case TokenType::INTERP_STRING_END: return "INTERP_STRING_END";
        
        // Assignment operators
        case TokenType::EQ: return "EQ";
        case TokenType::PLUS_EQ: return "PLUS_EQ";
        case TokenType::MINUS_EQ: return "MINUS_EQ";
        case TokenType::STAR_EQ: return "STAR_EQ";
        case TokenType::SLASH_EQ: return "SLASH_EQ";
        case TokenType::PERCENT_EQ: return "PERCENT_EQ";
        case TokenType::AMP_EQ: return "AMP_EQ";
        case TokenType::PIPE_EQ: return "PIPE_EQ";
        case TokenType::CARET_EQ: return "CARET_EQ";
        case TokenType::TILDE_EQ: return "TILDE_EQ";
        case TokenType::LT_LT_EQ: return "LT_LT_EQ";
        case TokenType::GT_GT_EQ: return "GT_GT_EQ";
        
        // Arithmetic operators
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";
        
        // Comparison operators
        case TokenType::EQ_EQ: return "EQ_EQ";
        case TokenType::BANG_EQ: return "BANG_EQ";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::LTE: return "LTE";
        case TokenType::GTE: return "GTE";
        
        // Logical operators
        case TokenType::BANG: return "BANG";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        
        // Bitwise operators
        case TokenType::AMP: return "AMP";
        case TokenType::PIPE: return "PIPE";
        case TokenType::TILDE: return "TILDE";
        case TokenType::CARET: return "CARET";
        case TokenType::LT_LT: return "LT_LT";
        case TokenType::GT_GT: return "GT_GT";
        
        // Other operators
        case TokenType::QUESTION: return "QUESTION";
        case TokenType::AT: return "AT";
        
        // Delimiters
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        
        // Punctuation
        case TokenType::COMMA: return "COMMA";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COLON: return "COLON";
        case TokenType::COLON_COLON: return "COLON_COLON";
        case TokenType::DOT: return "DOT";
        case TokenType::DOT_DOT: return "DOT_DOT";
        case TokenType::DOT_DOT_DOT: return "DOT_DOT_DOT";
        case TokenType::ARROW: return "ARROW";
        case TokenType::FAT_ARROW: return "FAT_ARROW";
        case TokenType::HASH: return "HASH";
        
        // Special
        case TokenType::EOF_TOKEN: return "EOF_TOKEN";
        
        default: return "UNKNOWN";
    }
}

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    
    Token(TokenType t, const std::string& l, int ln = 0, int col = 0)
        : type(t), lexeme(l), line(ln), column(col) {}
    
    // Helper for debugging
    std::string to_string() const {
        return token_type_to_string(type) + " '" + lexeme + "' at " 
               + std::to_string(line) + ":" + std::to_string(column);
    }
};

using TokenStream = std::vector<Token>;

} // namespace espresso_compiler