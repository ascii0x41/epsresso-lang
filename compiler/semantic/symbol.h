#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <optional>

#include "ast/astlib.h"

namespace espresso_compiler {

struct OperatorOverloadTypeHasher {
    size_t operator()(OperatorOverloadType t) const {
        return std::hash<int>()(static_cast<int>(t));
    }
};

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

struct Symbol;
struct TypeSymbol;
struct VariableSymbol;
struct StructSymbol;
struct FunctionSymbol;
struct OperatorOverloadSymbol;
struct TraitSymbol;
struct EnumSymbol;
struct ScopeSymbol;
struct TypeAliasSymbol;
struct GenericParamSymbol;

using SymbolPtr                 = std::shared_ptr<Symbol>;
using TypeSymbolPtr             = std::shared_ptr<TypeSymbol>;
using VariableSymbolPtr         = std::shared_ptr<VariableSymbol>;
using StructSymbolPtr           = std::shared_ptr<StructSymbol>;
using FunctionSymbolPtr         = std::shared_ptr<FunctionSymbol>;
using OperatorOverloadSymbolPtr = std::shared_ptr<OperatorOverloadSymbol>;
using TraitSymbolPtr            = std::shared_ptr<TraitSymbol>;
using EnumSymbolPtr             = std::shared_ptr<EnumSymbol>;
using ScopeSymbolPtr            = std::shared_ptr<ScopeSymbol>;
using TypeAliasSymbolPtr        = std::shared_ptr<TypeAliasSymbol>;

// ============================================================================
// SYMBOL KIND
// ============================================================================

enum class SymbolKind {
    Variable,
    Function,
    Struct,
    Trait,
    Enum,
    Operator,
    Scope,
    TypeAlias
};

// ============================================================================
// BASE
// ============================================================================

struct Symbol {
    SymbolKind kind;
    std::string name;

    virtual ~Symbol() = default;
};

// ============================================================================
// TYPE SYMBOL
// Represents a resolved type. Handles simple, generic, and reference types.
// ============================================================================

struct TypeSymbol {
    std::string name;                   // "Int", "Array", "MyStruct", etc.
    std::vector<TypeSymbol> generic_args;
    bool is_reference = false;          // true for 'ref T'
    bool is_const     = false;          // from TypeExpression::is_const

    static TypeSymbol make_error() {
        return TypeSymbol{"<error>", {}, false, false};
    }

    static TypeSymbol make_void() {
        return TypeSymbol{"Void", {}, false, false};
    }

    static TypeSymbol make_reference(TypeSymbol inner) {
        TypeSymbol t;
        t.name       = inner.name;
        t.generic_args = std::move(inner.generic_args);
        t.is_reference = true;
        t.is_const   = inner.is_const;
        return t;
    }

    bool is_error() const { return name == "<error>"; }

    bool operator==(const TypeSymbol& other) const {
        return name         == other.name
            && generic_args == other.generic_args
            && is_reference == other.is_reference;
    }

    bool operator!=(const TypeSymbol& other) const { return !(*this == other); }
};

// ============================================================================
// GENERIC PARAM SYMBOL
// Represents a generic parameter with its (unresolved) trait constraints.
// ============================================================================

struct GenericParamSymbol {
    struct TraitConstraint {
        std::string trait_name;
        std::vector<TypeSymbol> args;
    };
    std::string name;
    std::vector<TraitConstraint> constraint_names;  // e.g. ["Printable", "Comparable"]
};

using GenericParamSymbols = std::vector<GenericParamSymbol>;

// ============================================================================
// PARAMETER SYMBOL
// Preserves name + type + whether a default exists, needed for named-arg calls.
// ============================================================================

struct ParamSymbol {
    std::string name;
    TypeSymbol type;
    bool has_default = false;
    bool is_static = false;
    bool is_private = false;
};

// ============================================================================
// VARIABLE SYMBOL
// ============================================================================

struct VariableSymbol : Symbol {
    TypeSymbol type;
};

// ============================================================================
// FUNCTION SYMBOL
// ============================================================================

struct FunctionSymbol : Symbol {
    std::vector<GenericParamSymbol> generic_params;
    std::vector<ParamSymbol> params;       // name + type + has_default
    TypeSymbol return_type;
    bool is_static = false;
    bool is_private = false;
};

// ============================================================================
// OPERATOR OVERLOAD SYMBOL
// ============================================================================

struct OperatorOverloadSymbol : Symbol {
    OperatorOverloadType op_type;
    std::vector<ParamSymbol> params;
    TypeSymbol return_type;
};

// ============================================================================
// STRUCT SYMBOL
// ============================================================================

struct StructSymbol : Symbol {
    std::vector<GenericParamSymbol> generic_params;
    std::vector<std::string> trait_constraints; // resolved trait names
    std::unordered_map<std::string, VariableSymbolPtr> fields;
    std::unordered_map<std::string, std::vector<FunctionSymbolPtr>> methods;
    std::unordered_map<OperatorOverloadType,
                   std::vector<OperatorOverloadSymbolPtr>,
                   OperatorOverloadTypeHasher> operators;
    bool is_static = false;
    bool is_private = false;
};

// ============================================================================
// TRAIT SYMBOL
// Mirrors TraitDeclNode's three requirement kinds.
// ============================================================================

struct TraitSymbol : Symbol {
    std::vector<GenericParamSymbol> generic_params;

    // "func toString() -> String"
    struct MethodRequirement {
        std::string name;
        std::vector<TypeSymbol> param_types;
        TypeSymbol return_type;
        bool is_static = false;
    };

    // "where Self:Printable" -- a type-param must satisfy a trait
    struct TypeRequirement {
        std::string type_param;      // "Self" or "T"
        std::string constraint_name; // "Printable"
    };

    // "operator+(Self) -> Self"
    struct OperatorRequirement {
        OperatorOverloadType op_type;
        std::vector<TypeSymbol> param_types;
        TypeSymbol return_type;
    };

    std::vector<MethodRequirement>   required_methods;
    std::vector<TypeRequirement>     type_requirements;
    std::vector<OperatorRequirement> operator_requirements;
};

// ============================================================================
// ENUM SYMBOL
// ============================================================================

struct EnumSymbol : Symbol {
    std::vector<std::string> variants;
};

// ============================================================================
// SCOPE SYMBOL
// Represents a named scope declaration: scope math { ... }
// ============================================================================

struct ScopeSymbol : Symbol {
    std::unordered_map<std::string, SymbolPtr> members;
};

// ============================================================================
// TYPE ALIAS SYMBOL
// type Point2D = Pair<Float, Float>;
// ============================================================================

struct TypeAliasSymbol : Symbol {
    std::vector<GenericParamSymbol> generic_params;
    TypeSymbol                      target_type;
};

// ============================================================================
// SCOPE STACK
// ============================================================================

class ScopeStack {
    // Lexical variable scopes (let bindings, params, etc.)
    std::vector<std::unordered_map<std::string, SymbolPtr>> scopes;

    // Top-level type declarations -- looked up by name globally
    std::unordered_map<std::string, StructSymbolPtr> structs;
    std::unordered_map<std::string, TraitSymbolPtr> traits;
    std::unordered_map<std::string, EnumSymbolPtr> enums;
    std::unordered_map<std::string, TypeAliasSymbolPtr> type_aliases;
    std::unordered_map<std::string, ScopeSymbolPtr> named_scopes;
    std::unordered_map<std::string, std::vector<FunctionSymbolPtr>> functions;  // overload sets

public:
    // ---- Lexical scope management ----

    void push() { scopes.emplace_back(); }
    void pop()  { if (!scopes.empty()) scopes.pop_back(); }

    // Declare a variable/param into the innermost scope
    void declare(SymbolPtr sym) {
        if (!scopes.empty())
            scopes.back()[sym->name] = sym;
    }

    // Walk from innermost scope outward looking for a variable
    SymbolPtr lookup(const std::string& name) const {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end())
                return found->second;
        }
        return nullptr;
    }

    int depth() const { return static_cast<int>(scopes.size()); }

    // ---- Type declarations ----

    void declare_struct(StructSymbolPtr s) { structs[s->name] = std::move(s); }
    void declare_trait(TraitSymbolPtr t) { traits[t->name] = std::move(t); }
    void declare_enum(EnumSymbolPtr e) { enums[e->name] = std::move(e); }
    void declare_type_alias(TypeAliasSymbolPtr a) { type_aliases[a->name] = std::move(a); }
    void declare_scope(ScopeSymbolPtr s) { named_scopes[s->name] = std::move(s); }
    void declare_function(FunctionSymbolPtr f) { functions[f->name].push_back(std::move(f)); }

    StructSymbolPtr lookup_struct(const std::string& n) const { return find_or_null(structs, n); }
    TraitSymbolPtr lookup_trait(const std::string& n) const { return find_or_null(traits, n); }
    EnumSymbolPtr lookup_enum(const std::string& n) const { return find_or_null(enums, n); }
    TypeAliasSymbolPtr lookup_type_alias(const std::string& n) const { return find_or_null(type_aliases, n); }
    ScopeSymbolPtr lookup_scope(const std::string& n) const { return find_or_null(named_scopes, n); }

    // Returns all overloads for a given function name (empty vector if none)
    const std::vector<FunctionSymbolPtr>& lookup_functions(const std::string& n) const {
        static const std::vector<FunctionSymbolPtr> empty;
        auto it = functions.find(n);
        return it != functions.end() ? it->second : empty;
    }

private:
    template<typename Map>
    static typename Map::mapped_type find_or_null(const Map& m, const std::string& key) {
        auto it = m.find(key);
        return it != m.end() ? it->second : nullptr;
    }
};

} // namespace espresso_compiler