#pragma once

#include "astlib.h"
#include <iostream>
#include <string>

namespace espresso_compiler {

/**
 * Dump AST to stdout with indentation for debugging
 */
void dump_ast(std::ostream& out, const ASTNodePtr& node, int indent = 0);

/**
 * Dump AST to a string (useful for testing)
 */
std::string ast_to_string(const ASTNodePtr& node);

} // namespace espresso_compiler