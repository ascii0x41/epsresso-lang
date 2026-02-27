#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast_dump.h"
#include <iostream>

#include <fstream>
#include <string>
#include <format>

std::string read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

using namespace espresso_compiler;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage:\n" ;
        std::cerr << argv[0] << " <source.esp>\n";
        std::cerr << argv[0] << " <source.esp> --no-astdump\n";
        return 1;
    }

    std::string path = argv[1];
    bool no_astdump = false;
    
    // Check for optional flags
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--no-astdump") {
            no_astdump = true;
        }
    }

    std::cout << "Skipping astdump: " << no_astdump << std::endl;
    
    std::ofstream out("output.txt");

    try {
        Lexer my_lexer(read_file(path), path);

        auto tokens = my_lexer.lex();

        for (auto& token : tokens) {
            out << std::format("Token(TokenType.{},\"{}\",{},{}),", 
                token_type_to_string(token.type), 
                token.lexeme, token.line, token.column) << "\n";
        }

        // Dump AST for debugging
        if (!no_astdump) { 
            Parser my_parser(tokens, path);
            auto ast = my_parser.parse();
        
            out << "AST Dump:\n" << std::endl;
            dump_ast(out, ast);
        }


    } catch (const CompilerException& e) {
        fatal(e.what(), e.filepath, e.line, e.column);
        g_diagnostics.print_all();
        g_diagnostics.print_summary();
        return 1;
    }

    return 0;
}