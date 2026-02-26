#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace espresso_compiler {

struct Symbol;
struct TypeSymbol;
struct VariableSymbol;
struct StructSymbolPtr;
struct FunctionSymbol;
struct TraitSymbol;

using SymbolPtr = std::shared_ptr<Symbol>;
using TypeSymbolPtr = std::shared_ptr<TypeSymbol>;
using VariableSymbolPtr = std::shared_ptr<VariableSymbol>;
using StructSymbolPtr = std::shared_ptr<StructSymbolPtr>;
using FunctionSymbolPtr = std::shared_ptr<FunctionSymbol>;
using TraitSymbolPtr = std::shared_ptr<TraitSymbol>;


enum class SymbolKind {
    Variable,
    Function,
    Struct,
    Trait,
    Enum,
    Operator
};

struct Symbol {
    SymbolKind kind;
    std::string name;

    virtual ~Symbol() = default;
};

struct TypeSymbol {
    std::string name;
    std::vector<TypeSymbol> generic_args;

    bool operator==(const TypeSymbol& other) const {
        return name == other.name && generic_args == other.generic_args;
    }
};

struct VariableSymbol : Symbol {
    TypeSymbol type;
};

struct StructSymbol : Symbol {
    std::unordered_map<std::string, VariableSymbolPtr> fields;
    std::unordered_map<std::string, std::vector<FunctionSymbolPtr>> methods;

    std::vector<std::string> generic_params;  // just names
};

struct FunctionSymbol : Symbol {
    std::vector<TypeSymbol> parameters;
    TypeSymbol return_type;
    bool is_static = false;
};

struct TraitSymbol : Symbol {
    std::unordered_map<std::string, FunctionSymbolPtr> required_methods;
};

class ScopeStack {
    std::vector<std::unordered_map<std::string, SymbolPtr>> scopes;

public:
    void push() { scopes.emplace_back(); }
    void pop() { scopes.pop_back(); }

    void declare(SymbolPtr sym) { scopes.back()[sym->name] = sym; }

    SymbolPtr lookup(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end())
                return found->second;
        }
        return nullptr;
    }
};

std::unordered_map<std::string, StructSymbolPtr> structs;
std::unordered_map<std::string, std::vector<FunctionSymbolPtr>> functions;

}