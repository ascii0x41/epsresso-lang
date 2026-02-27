#include "ast_dump.h"
#include <sstream>

namespace espresso_compiler {

// ============================================================================
// HELPERS
// ============================================================================

static void indent(std::ostream& out, int level) {
    for (int i = 0; i < level; ++i) out << "  ";
}

static void dump_tag_list(std::ostream& out, const TagList& tags, int level) {
    if (tags.empty()) return;

    indent(out, level);
    out << "tags: [\n";
    for (const auto& tag : tags) {
        indent(out, level + 1);
        switch (tag->kind) {
            case NodeKind::TAG_STATICMEMBER: out << "#staticmember\n";  break;
            case NodeKind::TAG_CONSTMETHOD:  out << "#constmethod\n";   break;
            case NodeKind::TAG_CONSTEXPR:    out << "#constexpr\n";     break;
            case NodeKind::TAG_INLINE:       out << "#inline\n";        break;
            case NodeKind::TAG_DEPRECATED: {
                auto t = std::static_pointer_cast<TagDeprecated>(tag);
                out << "#deprecated";
                if (!t->message.empty()) out << "(\"" << t->message << "\")";
                out << "\n";
                break;
            }
            case NodeKind::TAG_DOCSTRING: {
                auto t = std::static_pointer_cast<TagDocstring>(tag);
                out << "#docstring(\"" << t->doc << "\")\n";
                break;
            }
            default: out << "#(unknown tag)\n"; break;
        }
    }
    indent(out, level);
    out << "]\n";
}

// Forward declaration
static void dump_node(std::ostream& out, const ASTNodePtr& node, int level);

static void dump_type(std::ostream& out, const TypeExprPtr& type, int level) {
    if (!type) {
        indent(out, level);
        out << "(inferred)\n";
        return;
    }

    indent(out, level);

    switch (type->kind) {
        case NodeKind::TYPE_SIMPLE: {
            auto t = std::static_pointer_cast<SimpleType>(type);
            out << (type->is_const ? "const " : "") << t->name << "\n";
            break;
        }
        case NodeKind::TYPE_GENERIC: {
            auto t = std::static_pointer_cast<GenericType>(type);
            out << (type->is_const ? "const " : "") << t->base_name << "<\n";
            for (const auto& arg : t->type_arguments) dump_type(out, arg, level + 1);
            indent(out, level);
            out << ">\n";
            break;
        }
        case NodeKind::TYPE_REFERENCE: {
            auto t = std::static_pointer_cast<ReferenceType>(type);
            out << (type->is_const ? "const " : "") << "&\n";
            dump_type(out, t->pointee_type, level + 1);
            break;
        }
        default:
            out << node_kind_to_string(type->kind) << "\n";
            break;
    }
}

static void dump_generic_params(std::ostream& out, const GenericParams& params, int level) {
    if (params.empty()) return;
    indent(out, level);
    out << "generics: <";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) out << ", ";
        out << params[i].name;
        if (!params[i].traits.empty()) {
            out << ":";
            for (size_t j = 0; j < params[i].traits.size(); ++j) {
                if (j > 0) out << ",";
                out << params[i].traits[j].trait_name->to_string();
                if (!params[i].traits[j].args.empty()) {
                    out << "<...>";
                }
            }
        }
    }
    out << ">\n";
}

// ============================================================================
// MAIN DUMP
// ============================================================================

static void dump_node(std::ostream& out, const ASTNodePtr& node, int level) {
    if (!node) {
        indent(out, level);
        out << "(null)\n";
        return;
    }

    indent(out, level);

    switch (node->kind) {

        // ===== LITERALS =====
        case NodeKind::LITERAL_INT: {
            auto n = std::static_pointer_cast<LiteralIntNode>(node);
            out << "(int " << n->value << ")\n";
            break;
        }
        case NodeKind::LITERAL_FLOAT: {
            auto n = std::static_pointer_cast<LiteralFloatNode>(node);
            out << "(float " << n->value << ")\n";
            break;
        }
        case NodeKind::LITERAL_STRING: {
            auto n = std::static_pointer_cast<LiteralStringNode>(node);
            out << "(string \"" << n->value << "\")\n";
            break;
        }
        case NodeKind::LITERAL_RAW_STRING: {
            auto n = std::static_pointer_cast<LiteralRawStringNode>(node);
            out << "(raw_string R\"" << n->value << "\")\n";
            break;
        }
        case NodeKind::LITERAL_BOOL: {
            auto n = std::static_pointer_cast<LiteralBoolNode>(node);
            out << "(bool " << (n->value ? "true" : "false") << ")\n";
            break;
        }
        case NodeKind::LITERAL_ARRAY: {
            auto n = std::static_pointer_cast<LiteralArrayNode>(node);
            out << "(array\n";
            for (const auto& e : n->elements) dump_node(out, e, level + 1);
            indent(out, level);
            out << ")\n";
            break;
        }
        case NodeKind::LITERAL_INTERP_STRING: {
            auto n = std::static_pointer_cast<LiteralInterpStringNode>(node);
            out << "(interp_string\n";
            for (const auto& part : n->parts) {
                if (part.is_expression) {
                    indent(out, level + 1);
                    out << "{expr:\n";
                    dump_node(out, part.expression, level + 2);
                    indent(out, level + 1);
                    out << "}\n";
                } else {
                    indent(out, level + 1);
                    out << "\"" << part.text << "\"\n";
                }
            }
            indent(out, level);
            out << ")\n";
            break;
        }

        // ===== NAMES =====
        case NodeKind::NAME_EXPR: {
            auto n = std::static_pointer_cast<NameExpression>(node);
            out << "(name " << n->name << ")\n";
            break;
        }
        case NodeKind::MEMBER_ACCESS: {
            auto n = std::static_pointer_cast<MemberAccessExpr>(node);
            out << "(member_access " << (n->is_static ? "::" : ".") << n->member_name << "\n";
            dump_node(out, n->object, level + 1);
            indent(out, level);
            out << ")\n";
            break;
        }

        // ===== OPERATORS =====
        case NodeKind::BINARY_EXPR: {
            auto n = std::static_pointer_cast<BinaryExprNode>(node);
            out << "(binary " << binary_op_to_string(n->op) << "\n";
            dump_node(out, n->left, level + 1);
            dump_node(out, n->right, level + 1);
            indent(out, level);
            out << ")\n";
            break;
        }
        case NodeKind::UNARY_EXPR: {
            auto n = std::static_pointer_cast<UnaryExprNode>(node);
            out << "(unary " << unary_op_to_string(n->op) << "\n";
            dump_node(out, n->operand, level + 1);
            indent(out, level);
            out << ")\n";
            break;
        }
        case NodeKind::TERNARY_EXPR: {
            auto n = std::static_pointer_cast<TernaryExprNode>(node);
            out << "(ternary\n";
            indent(out, level + 1); out << "cond:\n";
            dump_node(out, n->condition, level + 2);
            indent(out, level + 1); out << "then:\n";
            dump_node(out, n->true_expr, level + 2);
            indent(out, level + 1); out << "else:\n";
            dump_node(out, n->false_expr, level + 2);
            indent(out, level);
            out << ")\n";
            break;
        }

        // ===== CALLS & INDEXING =====
        case NodeKind::CALL_EXPR: {
            auto n = std::static_pointer_cast<CallExprNode>(node);
            out << "(call\n";
            indent(out, level + 1); out << "callee:\n";
            dump_node(out, n->callee, level + 2);
            if (!n->arguments.empty()) {
                indent(out, level + 1); out << "args:\n";
                for (const auto& arg : n->arguments) {
                    if (!arg.name.empty()) {
                        indent(out, level + 2);
                        out << arg.name << "=\n";
                        dump_node(out, arg.value, level + 3);
                    } else {
                        dump_node(out, arg.value, level + 2);
                    }
                }
            }
            indent(out, level);
            out << ")\n";
            break;
        }
        case NodeKind::INDEX_EXPR: {
            auto n = std::static_pointer_cast<IndexExprNode>(node);
            out << "(index\n";
            dump_node(out, n->object, level + 1);
            indent(out, level + 1); out << "[\n";
            dump_node(out, n->index, level + 2);
            indent(out, level + 1); out << "]\n";
            indent(out, level);
            out << ")\n";
            break;
        }
        case NodeKind::GENERIC_INSTANTIATION: {
            auto n = std::static_pointer_cast<GenericInstantiationExpr>(node);
            out << "(generic_inst\n";
            dump_node(out, n->base, level + 1);
            indent(out, level + 1); out << "type_args:\n";
            for (const auto& ta : n->type_arguments) dump_type(out, ta, level + 2);
            indent(out, level);
            out << ")\n";
            break;
        }

        // ===== LAMBDA =====
        case NodeKind::LAMBDA_EXPR: {
            auto n = std::static_pointer_cast<LambdaExprNode>(node);
            out << "(lambda\n";
            dump_generic_params(out, n->generic_params, level + 1);
            if (!n->parameters.empty()) {
                indent(out, level + 1); out << "params:\n";
                for (const auto& p : n->parameters) {
                    indent(out, level + 2); out << p.name << ":\n";
                    dump_type(out, p.type, level + 3);
                    if (p.default_value) {
                        indent(out, level + 3); out << "default:\n";
                        dump_node(out, p.default_value, level + 4);
                    }
                }
            }
            indent(out, level + 1); out << "return:\n";
            dump_type(out, n->return_type, level + 2);
            indent(out, level + 1); out << "body:\n";
            dump_node(out, n->body, level + 2);
            indent(out, level);
            out << ")\n";
            break;
        }

        // ===== STATEMENTS =====
        case NodeKind::BLOCK_STMT: {
            auto n = std::static_pointer_cast<BlockStatement>(node);
            out << "(block\n";
            for (const auto& s : n->statements) dump_node(out, s, level + 1);
            indent(out, level);
            out << ")\n";
            break;
        }
        case NodeKind::EXPRESSION_STMT: {
            auto n = std::static_pointer_cast<ExpressionStmt>(node);
            out << "(expr_stmt\n";
            dump_node(out, n->expression, level + 1);
            indent(out, level);
            out << ")\n";
            break;
        }

        // ===== CONTROL FLOW =====
        case NodeKind::IF_STMT: {
            auto n = std::static_pointer_cast<IfStatementNode>(node);
            out << "(if\n";
            indent(out, level + 1); out << "cond:\n";
            dump_node(out, n->condition, level + 2);
            indent(out, level + 1); out << "then:\n";
            dump_node(out, n->then_block, level + 2);
            for (const auto& [c, b] : n->elif_branches) {
                indent(out, level + 1); out << "elif cond:\n";
                dump_node(out, c, level + 2);
                indent(out, level + 1); out << "elif body:\n";
                dump_node(out, b, level + 2);
            }
            if (n->else_block) {
                indent(out, level + 1); out << "else:\n";
                dump_node(out, n->else_block, level + 2);
            }
            indent(out, level);
            out << ")\n";
            break;
        }
        case NodeKind::WHILE_STMT: {
            auto n = std::static_pointer_cast<WhileLoopNode>(node);
            out << "(while\n";
            indent(out, level + 1); out << "cond:\n";
            dump_node(out, n->condition, level + 2);
            indent(out, level + 1); out << "body:\n";
            dump_node(out, n->body, level + 2);
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::DO_WHILE_STMT: {
            auto n = std::static_pointer_cast<DoWhileLoopNode>(node);
            out << "(do_while\n";
            indent(out, level + 1); out << "body:\n";
            dump_node(out, n->body, level + 2);
            indent(out, level + 1); out << "cond:\n";
            dump_node(out, n->condition, level + 2);
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::FOR_STMT: {
            auto n = std::static_pointer_cast<ForLoopNode>(node);
            out << "(for\n";
            indent(out, level + 1); out << "init:\n";
            dump_node(out, n->initialiser, level + 2);
            indent(out, level + 1); out << "cond:\n";
            dump_node(out, n->condition, level + 2);
            indent(out, level + 1); out << "inc:\n";
            dump_node(out, n->increment, level + 2);
            indent(out, level + 1); out << "body:\n";
            dump_node(out, n->body, level + 2);
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::FOREACH_STMT: {
            auto n = std::static_pointer_cast<ForEachLoopNode>(node);
            out << "(foreach " << n->iterator_name;
            if (n->iterator_type) {
                out << ":";
                // inline short type names
                if (n->iterator_type->kind == NodeKind::TYPE_SIMPLE) {
                    out << std::static_pointer_cast<SimpleType>(n->iterator_type)->name;
                } else {
                    out << "..";
                }
            }
            out << " in\n";
            dump_node(out, n->iterable, level + 1);
            indent(out, level + 1); out << "body:\n";
            dump_node(out, n->body, level + 2);
            indent(out, level); out << ")\n";
            break;
        }

        // ===== JUMP STATEMENTS =====
        case NodeKind::RETURN_STMT: {
            auto n = std::static_pointer_cast<ReturnStatementNode>(node);
            if (n->value) {
                out << "(return\n";
                dump_node(out, n->value, level + 1);
                indent(out, level); out << ")\n";
            } else {
                out << "(return)\n";
            }
            break;
        }
        case NodeKind::BREAK_STMT:    out << "(break)\n";    break;
        case NodeKind::CONTINUE_STMT: out << "(continue)\n"; break;

        // ===== EXCEPTION HANDLING =====
        case NodeKind::THROW_STMT: {
            auto n = std::static_pointer_cast<ThrowStatementNode>(node);
            out << "(throw\n";
            dump_node(out, n->exception, level + 1);
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::TRY_CATCH_STMT: {
            auto n = std::static_pointer_cast<TryCatchStatementNode>(node);
            out << "(try_catch\n";
            indent(out, level + 1); out << "try:\n";
            dump_node(out, n->try_block, level + 2);
            for (const auto& c : n->catch_clauses) {
                indent(out, level + 1);
                out << "catch " << c.exception_name << ":\n";
                dump_type(out, c.exception_type, level + 2);
                dump_node(out, c.handler, level + 2);
            }
            if (n->finally_block) {
                indent(out, level + 1); out << "finally:\n";
                dump_node(out, n->finally_block, level + 2);
            }
            indent(out, level); out << ")\n";
            break;
        }

        // ===== DECLARATIONS =====
        case NodeKind::VARIABLE_DECL: {
            auto n = std::static_pointer_cast<VariableDeclNode>(node);
            out << "(let " << n->name << "\n";
            dump_tag_list(out, n->directives, level + 1);
            indent(out, level + 1); out << "type:\n";
            dump_type(out, n->type, level + 2);
            if (n->initialiser) {
                indent(out, level + 1); out << "init:\n";
                dump_node(out, n->initialiser, level + 2);
            }
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::DESTRUCTURE_DECL: {
            auto n = std::static_pointer_cast<DestructureDeclNode>(node);
            out << "(destructure " << "\n";
            dump_tag_list(out, n->directives, level + 1);
            indent(out, level + 1); out << "bindings:\n";
            for (const auto& b : n->bindings) {
                indent(out, level + 2); out << b.name << ":\n";
                dump_type(out, b.type, level + 3);
            }
            if (n->value) {
                indent(out, level + 1); out << "init:\n";
                dump_node(out, n->value, level + 2);
            }
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::FUNCTION_DECL: {
            auto n = std::static_pointer_cast<FunctionDeclNode>(node);
            out << "(func " << n->name << "\n";
            dump_tag_list(out, n->directives, level + 1);
            dump_generic_params(out, n->generic_params, level + 1);
            if (!n->parameters.empty()) {
                indent(out, level + 1); out << "params:\n";
                for (const auto& p : n->parameters) {
                    indent(out, level + 2); out << p.name << ":\n";
                    dump_type(out, p.type, level + 3);
                    if (p.default_value) {
                        indent(out, level + 3); out << "default:\n";
                        dump_node(out, p.default_value, level + 4);
                    }
                }
            }
            indent(out, level + 1); out << "return:\n";
            dump_type(out, n->return_type, level + 2);
            if (n->body) {
                indent(out, level + 1); out << "body:\n";
                dump_node(out, n->body, level + 2);
            } else {
                indent(out, level + 1); out << "(prototype)\n";
            }
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::STRUCT_DECL: {
            auto n = std::static_pointer_cast<StructDeclNode>(node);
            out << "(struct " << n->name << "\n";
            if (!n->constraints.empty()) {
                indent(out, level + 1); out << "(constraints " << "\n";
                for (const auto& c : n->constraints) {
                    indent(out, level + 2); out << c.trait_name->to_string();
                    if (!c.args.empty()) {
                        out << ":\n";
                        for (const auto& a : c.args) {
                            dump_type(out, a, level + 3);
                        }
                    } else { out << "\n"; }
                }
                indent(out, level + 1); out << ")\n";
            }
            dump_tag_list(out, n->directives, level + 1);
            dump_generic_params(out, n->generic_params, level + 1);
            for (const auto& m : n->members) {
                indent(out, level + 1);
                out << (m.is_private ? "[private]\n" : "[public]\n");
                dump_node(out, m.declaration, level + 2);
            }
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::OPERATOR_OVERLOAD_DECL: {
            auto n = std::static_pointer_cast<OperatorOverloadNode>(node);
            out << "(operator " << operator_type_to_string(n->op_type) << "\n";
            dump_tag_list(out, n->directives, level + 1);
            if (!n->parameters.empty()) {
                indent(out, level + 1); out << "params:\n";
                for (const auto& p : n->parameters) {
                    indent(out, level + 2); out << p.name << ":\n";
                    dump_type(out, p.type, level + 3);
                }
            }
            indent(out, level + 1); out << "return:\n";
            dump_type(out, n->return_type, level + 2);
            if (n->body) {
                indent(out, level + 1); out << "body:\n";
                dump_node(out, n->body, level + 2);
            }
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::TRAIT_DECL: {
            auto n = std::static_pointer_cast<TraitDeclNode>(node);
            out << "(trait " << n->name << "\n";
            dump_generic_params(out, n->generic_params, level + 1);
            for (const auto& req : n->requirements) {
                indent(out, level + 1);
                if (auto r = std::dynamic_pointer_cast<TraitDeclNode::TypeRequirement>(req)) {
                    out << "[type_req] " << r->type_param << ":" << r->constraint.trait_name->to_string() << "\n";
                } else if (auto r = std::dynamic_pointer_cast<TraitDeclNode::MethodRequirement>(req)) {
                    out << "[method_req] " << (r->is_static ? "static " : "") << r->name << "(";
                    for (size_t i = 0; i < r->parameters.size(); ++i) {
                        if (i > 0) out << ", ";
                        // inline simple type names
                        if (r->parameters[i] && r->parameters[i]->kind == NodeKind::TYPE_SIMPLE) {
                            out << std::static_pointer_cast<SimpleType>(r->parameters[i])->name;
                        } else {
                            out << "..";
                        }
                    }
                    out << ") -> ";
                    if (r->return_type && r->return_type->kind == NodeKind::TYPE_SIMPLE) {
                        out << std::static_pointer_cast<SimpleType>(r->return_type)->name;
                    } else {
                        out << "..";
                    }
                    out << "\n";
                } else if (auto r = std::dynamic_pointer_cast<TraitDeclNode::OperatorRequirement>(req)) {
                    out << "[op_req] operator" << operator_type_to_string(r->operator_type) << "\n";
                } else {
                    out << "[unknown_req]\n";
                }
            }
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::TYPE_ALIAS_DECL: {
            auto n = std::static_pointer_cast<TypeAliasDeclNode>(node);
            out << "(type_alias " << n->alias_name;
            dump_generic_params(out, n->generic_params, level + 1);

            out << " =\n";
            dump_type(out, n->target_type, level + 1);
            indent(out, level); out << ")\n";
            break;
        }
        case NodeKind::ENUM_DECL: {
            auto n = std::static_pointer_cast<EnumDeclNode>(node);
            out << "(enum " << n->name << " {";
            for (size_t i = 0; i < n->variants.size(); ++i) {
                if (i > 0) out << ", ";
                out << n->variants[i];
            }
            out << "})\n";
            break;
        }
        case NodeKind::SCOPE_DECL: {
            auto n = std::static_pointer_cast<ScopeDeclNode>(node);
            out << "(scope " << n->name << "\n";
            for (const auto& d : n->declares) {
                indent(out, level + 1);
                dump_node(out, d, level + 2);
            }
            indent(out, level); out << ")\n";
            break;
        }
        // ===== MODULE =====
        case NodeKind::IMPORT_DECL: {
            auto n = std::static_pointer_cast<ImportDeclNode>(node);
            out << "(import \"" << n->module_path << "\"";
            if (n->module_alias) out << " as " << *n->module_alias;
            if (!n->symbols.empty()) {
                out << " {";
                for (size_t i = 0; i < n->symbols.size(); ++i) {
                    if (i > 0) out << ", ";
                    out << n->symbols[i].original_name;
                    if (!n->symbols[i].alias.empty()) out << " as " << n->symbols[i].alias;
                }
                out << "}";
            }
            out << ")\n";
            break;
        }
        case NodeKind::EXPORT_DECL: {
            auto n = std::static_pointer_cast<ExportDeclNode>(node);
            out << "(export {";
            for (size_t i = 0; i < n->symbols.size(); ++i) {
                if (i > 0) out << ", ";
                out << n->symbols[i];
            }
            out << "})\n";
            break;
        }

        case NodeKind::EOF_STATEMENT: {
            out << "(EOF)\n";
            break;
        }

        default:
            out << "(" << node_kind_to_string(node->kind) << " [unhandled])\n";
            break;
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

void dump_ast(std::ostream& out, const ASTNodePtr& node, int indent_level) {
    dump_node(out, node, indent_level);
}

std::string ast_to_string(const ASTNodePtr& node) {
    std::ostringstream oss;
    dump_node(oss, node, 0);
    return oss.str();
}

} // namespace espresso_compiler