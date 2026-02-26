#pragma once

#include <unordered_map>
#include <string>
#include "token.h"

namespace espresso_compiler {

// ========== Character Classification Helpers ==========
inline bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool is_digit(char c) {
    return '0' <= c && c <= '9';
}

inline bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

inline bool is_alpha_numeric(char c) {
    return is_alpha(c) || is_digit(c);
}

// ========== Lexer Class ==========
class Lexer {
private:
    // ===== Source & Token Storage =====
    std::string source;
    std::string filepath;
    TokenStream tokens;
    
    // ===== Position Tracking =====
    size_t start;           // Start of current token
    size_t current;         // Current character position
    int line;               // Current line number
    int column;             // Current column number
    
    // ===== Keyword & Directive Maps =====
    static const std::unordered_map<std::string, TokenType> keywords;
    static const std::unordered_map<std::string, TokenType> tags;
    
    // ===== Position & Lookahead Methods =====
    bool is_at_end() const;
    char advance();
    char peek() const;
    char peek_next() const;
    bool match(char expected);
    
    // ===== Token Creation =====
    void add_token(TokenType type, std::string lexeme = "");
    
    // ===== Main Scanning =====
    void scan_token();
    
    // ===== Specialized Tokenizers =====
    void scan_identifier();
    void scan_number();
    void scan_string();
    void scan_interpolated_string();
    void scan_raw_string();
    void scan_directive();          // Handle # tags
    void skip_line_comment();
    void skip_block_comment();
    
public:
    explicit Lexer(std::string src, std::string file) : source(std::move(src)), filepath(std::move(file)) {}
    
    // Main entry point
    TokenStream lex();
};

} // namespace espresso_compiler