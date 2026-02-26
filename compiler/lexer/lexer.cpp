#include "lexer.h"
#include <stdexcept>
#include <format>
#include "common/diagnostics.h"

namespace espresso_compiler {

// ========== Keyword Map ==========
// Maps keyword strings to their token types
const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    // Declaration keywords
    {"let", TokenType::LET},
    {"func", TokenType::FUNC},
    {"struct", TokenType::STRUCT},
    {"impl", TokenType::IMPL},
    {"type", TokenType::TYPE},
    {"trait", TokenType::TRAIT},
    {"where", TokenType::WHERE},
    {"operator", TokenType::OPERATOR},
    {"enum", TokenType::ENUM},
    {"scope", TokenType::SCOPE},
    
    // Control flow
    {"if", TokenType::IF},
    {"elif", TokenType::ELIF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"do", TokenType::DO},
    {"for", TokenType::FOR},
    {"foreach", TokenType::FOREACH},
    {"in", TokenType::IN},
    
    // Jump statements
    {"return", TokenType::RETURN},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"throw", TokenType::THROW},
    
    // Exception handling
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"finally", TokenType::FINALLY},
    
    // Module system
    {"import", TokenType::IMPORT},
    {"export", TokenType::EXPORT},
    {"as", TokenType::AS},
    
    // Access modifiers
    {"public", TokenType::PUBLIC},
    {"private", TokenType::PRIVATE},
    
    // Type qualifiers
    {"const", TokenType::CONST},
    {"ref", TokenType::REF},
    
    // Literals
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"Self", TokenType::SELF},
};

// ========== Directive Map ==========
// Maps directive names (without #) to their token types
const std::unordered_map<std::string, TokenType> Lexer::tags = {
    {"staticmember", TokenType::TAG_STATICMEMBER},
    {"constmethod", TokenType::TAG_CONSTMETHOD},
    {"deprecated", TokenType::TAG_DEPRECATED},
    {"constexpr", TokenType::TAG_CONSTEXPR},
    {"inline", TokenType::TAG_INLINE},
    {"docstring", TokenType::TAG_DOCSTRING}
};

// ========== Position & Lookahead Methods ==========

bool Lexer::is_at_end() const {
    return current >= source.size();
}

char Lexer::advance() {
    char c = source[current];
    ++current;
    ++column;
    return c;
}

char Lexer::peek() const {
    return is_at_end() ? '\0' : source[current];
}

char Lexer::peek_next() const {
    return (current + 1 >= source.size()) ? '\0' : source[current + 1];
}

bool Lexer::match(char expected) {
    if (is_at_end() || source[current] != expected) {
        return false;
    }
    ++current;
    ++column;
    return true;
}

// ========== Token Creation ==========

void Lexer::add_token(TokenType type, std::string lexeme) {
    if (lexeme.empty()) {
        lexeme = source.substr(start, current - start);
    }
    tokens.emplace_back(type, lexeme, line, column - (current - start));
}

// ========== Main Scanning Logic ==========

void Lexer::scan_token() {
    char c = advance();
    
    switch (c) {
        // ===== Single-Character Tokens =====
        case '(': add_token(TokenType::LPAREN); break;
        case ')': add_token(TokenType::RPAREN); break;
        case '{': add_token(TokenType::LBRACE); break;
        case '}': add_token(TokenType::RBRACE); break;
        case '[': add_token(TokenType::LBRACKET); break;
        case ']': add_token(TokenType::RBRACKET); break;
        case ',': add_token(TokenType::COMMA); break;
        case ';': add_token(TokenType::SEMICOLON); break;
        case '?': add_token(TokenType::QUESTION); break;
        
        // ===== Dot (., .., ...) =====
        case '.':
            if (match('.')) {
                if (match('.')) {
                    add_token(TokenType::DOT_DOT_DOT);
                } else {
                    add_token(TokenType::DOT_DOT);
                }
            } else {
                add_token(TokenType::DOT);
            }
            break;
        
        // ===== Colon (:, ::) =====
        case ':':
            add_token(match(':') ? TokenType::COLON_COLON : TokenType::COLON);
            break;
        
        // ===== Plus (+, +=) =====
        case '+':
            add_token(match('=') ? TokenType::PLUS_EQ : TokenType::PLUS);
            break;
        
        // ===== Minus (-, -=, ->) =====
        case '-':
            if (match('>')) {
                add_token(TokenType::ARROW);
            } else if (match('=')) {
                add_token(TokenType::MINUS_EQ);
            } else {
                add_token(TokenType::MINUS);
            }
            break;
        
        // ===== Star (*, *=) =====
        case '*':
            add_token(match('=') ? TokenType::STAR_EQ : TokenType::STAR);
            break;
        
        // ===== Percent (%, %=) =====
        case '%':
            add_token(match('=') ? TokenType::PERCENT_EQ : TokenType::PERCENT);
            break;
        
        // ===== Slash (/, /=, //, /*) =====
        case '/':
            if (match('/')) {
                skip_line_comment();
            } else if (match('*')) {
                skip_block_comment();
            } else {
                add_token(match('=') ? TokenType::SLASH_EQ : TokenType::SLASH);
            }
            break;
        
        // ===== Equals (=, ==, =>) =====
        case '=':
            if (match('=')) {
                add_token(TokenType::EQ_EQ);
            } else if (match('>')) {
                add_token(TokenType::FAT_ARROW);
            } else {
                add_token(TokenType::EQ);
            }
            break;
        
        // ===== Bang (!, !=) =====
        case '!':
            add_token(match('=') ? TokenType::BANG_EQ : TokenType::BANG);
            break;
        
        // ===== Less Than (<, <=, <<, <<=) =====
        case '<':
            if (match('<')) {
                add_token(match('=') ? TokenType::LT_LT_EQ : TokenType::LT_LT);
            } else {
                add_token(match('=') ? TokenType::LTE : TokenType::LT);
            }
            break;
        
        // ===== Greater Than (>, >=, >>, >>=) =====
        case '>':
            if (match('>')) {
                add_token(match('=') ? TokenType::GT_GT_EQ : TokenType::GT_GT);
            } else {
                add_token(match('=') ? TokenType::GTE : TokenType::GT);
            }
            break;
        
        // ===== Ampersand (&, &&, &=) =====
        case '&':
            if (match('&')) {
                add_token(TokenType::AND);
            } else if (match('=')) {
                add_token(TokenType::AMP_EQ);
            } else {
                add_token(TokenType::AMP);
            }
            break;
        
        // ===== Pipe (|, ||, |=) =====
        case '|':
            if (match('|')) {
                add_token(TokenType::OR);
            } else if (match('=')) {
                add_token(TokenType::PIPE_EQ);
            } else {
                add_token(TokenType::PIPE);
            }
            break;
        
        // ===== Caret (^, ^=) =====
        case '^':
            add_token(match('=') ? TokenType::CARET_EQ : TokenType::CARET);
            break;
        
        // ===== Tilde (~, ~=) =====
        case '~':
            add_token(match('=') ? TokenType::TILDE_EQ : TokenType::TILDE);
            break;
        
        // ===== Hash (# - tags) =====
        case '#':
            add_token(TokenType::HASH); // Emit the # token first; parser will handle the rest
            scan_directive();   // Then scan the directive name and emit the corresponding tag token
            break;
        
        // ===== String Literals =====
        case '"':
            scan_string();
            break;
        
        // ===== Interpolated Strings ($") =====
        case '$':
            if (peek() == '"') {
                advance(); // consume "
                scan_interpolated_string();
            } else {
                throw CompilerException(
                    "Unexpected character '$'", line,column, filepath
                );
            }
            break;
        
        // ===== Raw Strings (R") =====
        case 'R':
            if (peek() == '"') {
                advance(); // consume "
                scan_raw_string();
            } else {
                // It's just a regular identifier starting with 'R'
                scan_identifier();
            }
            break;
        
        // ===== Whitespace =====
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace
            break;
        
        case '\n':
            line++;
            column = 0; // Will be incremented to 1 by advance()
            break;
        
        // ===== Default: Identifier or Number =====
        default:
            if (is_alpha(c)) {
                scan_identifier();
            } else if (is_digit(c)) {
                scan_number();
            } else {
                throw CompilerException(
                    std::format("Unexpected character '{}'", c), line, column, filepath
                );
            }
            break;
    }
}

// ========== Specialized Tokenizers ==========

void Lexer::scan_identifier() {
    while (is_alpha_numeric(peek())) {
        advance();
    }
    
    std::string text = source.substr(start, current - start);
    
    // Check if it's a keyword
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        add_token(it->second, text);
    } else {
        add_token(TokenType::IDENTIFIER, text);
    }
}

void Lexer::scan_number() {
    // Scan integer part
    while (is_digit(peek())) {
        advance();
    }
    
    bool is_float = false;
    
    // Check for decimal point
    if (peek() == '.' && is_digit(peek_next())) {
        is_float = true;
        advance(); // consume '.'
        
        // Scan fractional part
        while (is_digit(peek())) {
            advance();
        }
    }
    
    // Check for scientific notation (e or E)
    if (peek() == 'e' || peek() == 'E') {
        is_float = true;
        advance(); // consume 'e' or 'E'
        
        // Optional sign
        if (peek() == '+' || peek() == '-') {
            advance();
        }
        
        // Must have at least one digit after exponent
        if (!is_digit(peek())) {
            throw CompilerException(
                "Expected digits after exponent", line, column, filepath
            );
        }
        
        while (is_digit(peek())) {
            advance();
        }
    }
    
    // Check for complex number suffix (i or j)
    // Only treat as complex if NOT followed by an alphanumeric character
    bool is_complex = false;
    if ((peek() == 'i' || peek() == 'j') && !is_alpha_numeric(peek_next())) {
        is_complex = true;
        is_float = true; // Complex numbers are treated as float-like
        advance(); // consume 'i' or 'j'
    }
    
    std::string text = source.substr(start, current - start);
    if (is_complex) {
        add_token(TokenType::COMPLEX_LITERAL, text);
    } else {
        add_token(is_float ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL, text);
    }
}

void Lexer::scan_string() {
    std::string value;
    
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') {
            line++;
            column = 0;
        }
        
        // Handle escape sequences
        if (peek() == '\\') {
            advance(); // consume '\'
            if (!is_at_end()) {
                char escaped = advance();
                // Store the escaped character as-is for now
                // Parser can interpret \n, \t, etc. later
                value += '\\';
                value += escaped;
            }
        } else {
            value += advance();
        }
    }
    
    if (is_at_end()) {
        throw CompilerException(
            "Unterminated string", line, column, filepath
        );
    }
    
    advance(); // consume closing "
    add_token(TokenType::STRING_LITERAL, value);
}

void Lexer::scan_interpolated_string() {
    add_token(TokenType::INTERP_STRING_START);
    
    std::string current_text;
    
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') {
            line++;
            column = 0;
        }
        
        // Handle escape sequences
        if (peek() == '\\') {
            current_text += advance();
            if (!is_at_end()) {
                current_text += advance();
            }
        }
        // Handle interpolation
        else if (peek() == '{') {
            // Emit accumulated text as a string part
            if (!current_text.empty()) {
                add_token(TokenType::INTERP_STRING_PART, current_text);
                current_text.clear();
            }
            
            advance(); // consume '{'
            
            // Tokenize the expression inside {}
            while (peek() != '}' && !is_at_end()) {
                start = current;
                scan_token();
            }
            
            if (is_at_end()) {
                throw CompilerException(
                    "Unterminated interpolation", line, column, filepath
                );
            }
            
            advance(); // consume '}'
        }
        else {
            current_text += advance();
        }
    }
    
    // Emit final text part if any
    if (!current_text.empty()) {
        add_token(TokenType::INTERP_STRING_PART, current_text);
    }
    
    if (is_at_end()) {
        throw CompilerException(
            "Unterminated interpolated string", line, column, filepath
        );
    }
    
    advance(); // consume closing "
    add_token(TokenType::INTERP_STRING_END);
}

void Lexer::scan_raw_string() {
    std::string value;
    
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') {
            line++;
            column = 0;
        }
        value += advance();
    }
    
    if (is_at_end()) {
        throw CompilerException(
            "Unterminated raw string", line, column, filepath
        );
    }
    
    advance(); // consume closing "
    add_token(TokenType::RAW_STRING_LITERAL, value);
}

void Lexer::scan_directive() {
    // A directive starts with # followed by an identifier
    
    // Scan the directive name
    while (is_alpha_numeric(peek())) {
        advance();
    }
    
    std::string directive_name = source.substr(start + 1, current - start - 1); // Skip the #
    
    // Look up the directive
    auto it = tags.find(directive_name);
    if (it != tags.end()) {
        add_token(it->second, directive_name);
    } else {
        throw CompilerException(
            std::format("Unknown directive '{}'", directive_name),
                line, column - directive_name.length(), filepath
        );
    }
}

void Lexer::skip_line_comment() {
    // Skip everything until end of line
    while (peek() != '\n' && !is_at_end()) {
        advance();
    }
}

void Lexer::skip_block_comment() {
    int nesting_level = 1; // Support nested block comments
    
    while (nesting_level > 0 && !is_at_end()) {
        if (peek() == '\n') {
            line++;
            column = 0;
        }
        
        // Check for nested /*
        if (peek() == '/' && peek_next() == '*') {
            advance(); // consume '/'
            advance(); // consume '*'
            nesting_level++;
        }
        // Check for closing */
        else if (peek() == '*' && peek_next() == '/') {
            advance(); // consume '*'
            advance(); // consume '/'
            nesting_level--;
        }
        else {
            advance();
        }
    }
    
    if (nesting_level > 0) {
        throw CompilerException(
            "Unterminated block comment", line, column, filepath
        );
    }
}

// ========== Main Entry Point ==========

TokenStream Lexer::lex() {
    // Initialize position tracking
    start = 0;
    current = 0;
    line = 1;
    column = 1;

    if (source.empty()) {
        // If the source is empty, just return an EOF token
        tokens.emplace_back(TokenType::EOF_TOKEN, "", line, column);
        return tokens;
    }
    if (!tokens.empty()) {
        return tokens; // Return cached tokens if already lexed
    }
    
    // Scan all tokens
    while (!is_at_end()) {
        start = current;
        scan_token();
    }
    
    // Add EOF token
    tokens.emplace_back(TokenType::EOF_TOKEN, "", line, column);
    
    return tokens;
}

} // namespace espresso_compiler