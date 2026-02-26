#include "analyser.h"
#include "common/diagnostics.h"
#include <memory>
#include <algorithm>

namespace espresso_compiler {

TypeSymbol make_error_type() {
    return TypeSymbol{"<error>", {}};
}

TypeSymbolPtr LocalAnalyser::validate_expression(const ExpressionPtr& expr) {
    // We can do recursion here, no? All expressions from Binary to Unary to Literals are ExpressionPtr

    switch (expr->kind) {
        case NodeKind::LITERAL_INT: return std::make_shared<TypeSymbol>(TypeSymbol{"Int", {}});
        case NodeKind::LITERAL_FLOAT: return std::make_shared<TypeSymbol>(TypeSymbol{"Float", {}});
        case NodeKind::LITERAL_COMPLEX: return std::make_shared<TypeSymbol>(TypeSymbol{"Complex", {}});
        case NodeKind::LITERAL_STRING: 
        case NodeKind::LITERAL_RAW_STRING:
        case NodeKind::LITERAL_INTERP_STRING:
        return std::make_shared<TypeSymbol>(TypeSymbol{"String", {}});
        case NodeKind::LITERAL_BOOL: return std::make_shared<TypeSymbol>(TypeSymbol{"Bool", {}});
        case NodeKind::LITERAL_ARRAY: {
            auto array = std::static_pointer_cast<LiteralArrayNode>(expr);
            const auto& elements = array->elements;

            if (elements.empty()) {
                return std::make_shared<TypeSymbol>(
                    TypeSymbol{"Array", {TypeSymbol{"auto", {}}}}
                );
            }

            auto first_ptr = validate_expression(elements.front());
            if (!first_ptr) { return std::make_shared<TypeSymbol>(
                TypeSymbol{"Array", {make_error_type()}}
            ); }

            TypeSymbol first = *first_ptr;

            for (size_t i = 1; i < elements.size(); ++i) {
                auto current = validate_expression(elements[i]);
                if (!current || *current != first) {
                    error("Array contains heterogeneous elements",
                        array->location.file_name,
                        array->location.line,
                        array->location.column);
                    return std::make_shared<TypeSymbol>(
                        TypeSymbol{"Array", {make_error_type()}}
                    );
                }
            }

            return std::make_shared<TypeSymbol>(
                TypeSymbol{"Array", {first}}
            );
        }
        case NodeKind::LITERAL_MAP: {
            auto map = static_pointer_cast<LiteralMapNode>(expr);
            const auto& pairs = map->pairs;

            if (pairs.empty()) {
                return std::make_shared<TypeSymbol>(
                    TypeSymbol{"Map", {
                        TypeSymbol{"auto", {}},
                        TypeSymbol{"auto", {}}
                    }}
                );
            }

            auto first_k_ptr = validate_expression(pairs.front().first);
            auto first_v_ptr = validate_expression(pairs.front().second);

            if (!first_k_ptr || !first_v_ptr) {
                return std::make_shared<TypeSymbol>(
                    TypeSymbol{"Map", {
                        make_error_type(),
                        make_error_type()
                    }}
                );
            }

            auto first_k = *first_k_ptr;
            auto first_v = *first_v_ptr;

            for (size_t i = 1; i < pairs.size(); ++i) {
                auto current_k = validate_expression(pairs[i].first);
                auto current_v = validate_expression(pairs[i].second);
                
                if (!current_k || *current_k != first_k || !current_v || *current_v != first_v) {
                    error("Map contains hetrogenous pairs",
                        map->location.file_name,
                        map->location.line,
                        map->location.column);
                    return std::make_shared<TypeSymbol>(
                        TypeSymbol{"Map", {
                            make_error_type(),
                            make_error_type()
                        }});
                }
            }

            return std::make_shared<TypeSymbol>(
                TypeSymbol{"Map", {
                    first_k,
                    first_v
                }}
            );
        }
        default:
            return std::make_shared<TypeSymbol>(make_error_type());
    }
}
}