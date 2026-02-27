#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <optional>

namespace espresso_compiler {

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

struct ASTNode;
struct Statement;
struct Name;
struct Expression;
struct Declaration;
struct TypeExpression;

// ============================================================================
// SMART POINTER ALIASES
// ============================================================================

template<typename T>
using Ptr = std::shared_ptr<T>;

using ASTNodePtr = Ptr<ASTNode>;
using StatementPtr = Ptr<Statement>;
using ExpressionPtr = Ptr<Expression>;
using NamePtr = Ptr<Name>;
using DeclarationPtr = Ptr<Declaration>;
using TypeExprPtr = Ptr<TypeExpression>;

// ============================================================================
// ENUMERATIONS
// ============================================================================

enum class NodeKind {
    // ========== TYPE SYSTEM ==========
    TYPE_SIMPLE,              // Int, Float, String
    TYPE_GENERIC,             // Array<Int>, Tuple<Int, String>
    TYPE_REFERENCE,           // ref Type

    // ========== EXPRESSIONS ==========
    // Literals
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_COMPLEX,
    LITERAL_STRING,
    LITERAL_RAW_STRING,
    LITERAL_INTERP_STRING,    // $"Hello {name}"
    LITERAL_BOOL,
    LITERAL_ARRAY,            // [1, 2, 3]
    LITERAL_MAP,

    // Names and access
    NAME_EXPR,                // identifier

    // Operations
    BINARY_EXPR,              // a + b
    UNARY_EXPR,               // -x, !flag
    TERNARY_EXPR,             // a ? b : c
    UNPACK_EXPR,              // unpack (a, b) = b, a;

    // Calls and indexing
    CALL_EXPR,                // func(args)
    INDEX_EXPR,               // arr[0]
    MEMBER_ACCESS,            // obj.field

    // Lambda
    LAMBDA_EXPR,              // (x:Int) => Int { return x * 2; }

    // Type expressions in expression context
    GENERIC_INSTANTIATION,    // Pair<Int, String>

    // ========== STATEMENTS ==========
    BLOCK_STMT,
    EXPRESSION_STMT,

    // Control flow
    IF_STMT,
    WHILE_STMT,
    DO_WHILE_STMT,
    FOR_STMT,
    FOREACH_STMT,

    // Jump statements
    RETURN_STMT,
    BREAK_STMT,
    CONTINUE_STMT,

    // Exception handling
    THROW_STMT,
    TRY_CATCH_STMT,

    // ========== DECLARATIONS ==========
    VARIABLE_DECL,
    DESTRUCTURE_DECL,
    FUNCTION_DECL,
    STRUCT_DECL,
    TRAIT_DECL,
    TYPE_ALIAS_DECL,
    OPERATOR_OVERLOAD_DECL,
    ENUM_DECL,
    SCOPE_DECL,

    // ========== MODULE SYSTEM ==========
    IMPORT_DECL,
    EXPORT_DECL,

    // ========== TAGS ==========
    TAG_STATICMEMBER,
    TAG_CONSTMETHOD,
    TAG_DEPRECATED,
    TAG_CONSTEXPR,
    TAG_INLINE,
    TAG_DOCSTRING,

    // ========== OTHER ==========
    EOF_STATEMENT,            // Special node to represent end of file
};

enum class BinaryOp {
    // Arithmetic
    ADD,                // +
    SUBTRACT,           // -
    MULTIPLY,           // *
    DIVIDE,             // /
    MODULO,             // %

    // Comparison
    EQUAL,              // ==
    NOT_EQUAL,          // !=
    LESS,               // <
    LESS_EQUAL,         // <=
    GREATER,            // >
    GREATER_EQUAL,      // >=

    // Logical
    LOGICAL_AND,        // &&
    LOGICAL_OR,         // ||

    // Bitwise
    BIT_AND,            // &
    BIT_OR,             // |
    BIT_XOR,            // ^
    SHIFT_LEFT,         // <<
    SHIFT_RIGHT,        // >>

    // Assignment
    ASSIGN,             // =
    ADD_ASSIGN,         // +=
    SUBTRACT_ASSIGN,    // -=
    MULTIPLY_ASSIGN,    // *=
    DIVIDE_ASSIGN,      // /=
    MODULO_ASSIGN,      // %=
    BIT_AND_ASSIGN,     // &=
    BIT_OR_ASSIGN,      // |=
    BIT_XOR_ASSIGN,     // ^=
    TILDE_ASSIGN,       // ~=
    SHIFT_LEFT_ASSIGN,  // <<=
    SHIFT_RIGHT_ASSIGN, // >>=
};

enum class UnaryOp {
    PLUS,               // +x
    MINUS,              // -x
    LOGICAL_NOT,        // !x
    BIT_NOT,            // ~x
    ADDRESS_OF,         // &x  (take address)
};

// ============================================================================
// SOURCE LOCATION
// ============================================================================

struct SourceLocation {
    int line;
    int column;
    std::string file_name;

    SourceLocation(int l = 0, int c = 0, std::string fn = "") : line(l), column(c), file_name(std::move(fn)) {}
};

// ============================================================================
// BASE NODES
// ============================================================================

struct ASTNode {
    NodeKind kind;
    SourceLocation location;

    virtual ~ASTNode() = default;

protected:
    explicit ASTNode(NodeKind k, SourceLocation loc = {})
        : kind(k), location(loc) {}
};

struct Statement : public ASTNode {
protected:
    explicit Statement(NodeKind k, SourceLocation loc = {})
        : ASTNode(k, loc) {}
};

struct Expression : public ASTNode {
protected:
    explicit Expression(NodeKind k, SourceLocation loc = {})
        : ASTNode(k, loc) {}
};

struct Declaration : public Statement {
protected:
    explicit Declaration(NodeKind k, SourceLocation loc = {})
        : Statement(k, loc) {}
};

// ============================================================================
// TAGS
// ============================================================================

struct Tag : public ASTNode {
protected:
    explicit Tag(NodeKind k, SourceLocation loc = {})
        : ASTNode(k, loc) {}
};

struct TagStaticmember : public Tag {
    explicit TagStaticmember(SourceLocation loc = {})
        : Tag(NodeKind::TAG_STATICMEMBER, loc) {}
};

struct TagConstmethod : public Tag {
    explicit TagConstmethod(SourceLocation loc = {})
        : Tag(NodeKind::TAG_CONSTMETHOD, loc) {}
};

struct TagConstexpr : public Tag {
    explicit TagConstexpr(SourceLocation loc = {})
        : Tag(NodeKind::TAG_CONSTEXPR, loc) {}
};

struct TagInline : public Tag {
    explicit TagInline(SourceLocation loc = {})
        : Tag(NodeKind::TAG_INLINE, loc) {}
};

struct TagDeprecated : public Tag {
    std::string message;  // Optional deprecation message
    explicit TagDeprecated(std::string msg = "", SourceLocation loc = {})
        : Tag(NodeKind::TAG_DEPRECATED, loc), message(std::move(msg)) {}
};

struct TagDocstring : public Tag {
    std::string doc;  // The documentation string
    explicit TagDocstring(std::string d, SourceLocation loc = {})
        : Tag(NodeKind::TAG_DOCSTRING, loc), doc(std::move(d)) {}
};

using TagPtr = Ptr<Tag>;
using TagList = std::vector<TagPtr>;


// ============================================================================
// TYPE SYSTEM
// ============================================================================

/**
 * Base for all type expressions
 */
struct TypeExpression : public ASTNode {
    bool is_const;

protected:
    explicit TypeExpression(NodeKind k, bool is_const = false, SourceLocation loc = {})
        : ASTNode(k, loc), is_const(is_const) {}
};

/**
 * Simple type: Int, Float, MyClass
 */
struct SimpleType : public TypeExpression {
    std::string name;

    SimpleType(std::string n, bool is_const = false, SourceLocation loc = {})
        : TypeExpression(NodeKind::TYPE_SIMPLE, is_const, loc)
        , name(std::move(n)) {}
};

/**
 * Generic type: Array<Int>, Tuple<Int, String, Float>
 */
struct GenericType : public TypeExpression {
    std::string base_name;                    // "Array", "Tuple", etc.
    std::vector<TypeExprPtr> type_arguments;  // The types inside <>

    GenericType(std::string base, std::vector<TypeExprPtr> args,
                bool is_const = false, SourceLocation loc = {})
        : TypeExpression(NodeKind::TYPE_GENERIC, is_const, loc)
        , base_name(std::move(base))
        , type_arguments(std::move(args)) {}
};

/**
 * Pointer/Reference type: ref Type
 */
struct ReferenceType : public TypeExpression {
    TypeExprPtr pointee_type;

    ReferenceType(TypeExprPtr pointee, bool is_const = false, SourceLocation loc = {})
        : TypeExpression(NodeKind::TYPE_REFERENCE, is_const, loc)
        , pointee_type(std::move(pointee)) {}
};

/**
 * Represents a trait constraint like "Printable" or "Comparable<T>"
 */
struct TraitConstraint {
    NamePtr trait_name;               // Printable, Comparable (as Name expression)
    std::vector<TypeExprPtr> args;    // For generic traits: [T] for Comparable<T>

    TraitConstraint(NamePtr name, std::vector<TypeExprPtr> a = {})
        : trait_name(std::move(name)), args(std::move(a)) {}
};

/**
 * Represents a generic parameter with its constraints.
 * Examples:
 *   T                        -- no constraints
 *   T:Printable              -- single trait
 *   T:Printable,Comparable   -- multiple traits
 *   T:Comparable<U>          -- generic trait with args
 */
struct GenericParam {
    std::string name;                     // "T"
    std::vector<TraitConstraint> traits;  // constraints on this param

    GenericParam(std::string n, std::vector<TraitConstraint> t = {})
        : name(std::move(n)), traits(std::move(t)) {}

    bool has_constraints() const { return !traits.empty(); }
};

using GenericParams = std::vector<GenericParam>;

// ============================================================================
// LITERAL EXPRESSIONS
// ============================================================================

struct LiteralIntNode : public Expression {
    std::string value;  // stored as string to preserve formatting (e.g. 0xFF)

    explicit LiteralIntNode(std::string v, SourceLocation loc = {})
        : Expression(NodeKind::LITERAL_INT, loc)
        , value(std::move(v)) {}
};

struct LiteralFloatNode : public Expression {
    std::string value;

    explicit LiteralFloatNode(std::string v, SourceLocation loc = {})
        : Expression(NodeKind::LITERAL_FLOAT, loc)
        , value(std::move(v)) {}
};

struct LiteralComplexNode : public Expression {
    std::string value;

    explicit LiteralComplexNode(std::string v, SourceLocation loc = {})
        : Expression(NodeKind::LITERAL_COMPLEX)
        , value(std::move(v)) {}
};

struct LiteralStringNode : public Expression {
    std::string value;

    explicit LiteralStringNode(std::string v, SourceLocation loc = {})
        : Expression(NodeKind::LITERAL_STRING, loc)
        , value(std::move(v)) {}
};

struct LiteralRawStringNode : public Expression {
    std::string value;

    explicit LiteralRawStringNode(std::string v, SourceLocation loc = {})
        : Expression(NodeKind::LITERAL_RAW_STRING, loc)
        , value(std::move(v)) {}
};

/**
 * Interpolated string: $"Hello {name}, you are {age} years old"
 */
struct LiteralInterpStringNode : public Expression {
    struct StringPart {
        bool is_expression;
        std::string text;           // if !is_expression
        ExpressionPtr expression;   // if  is_expression
    };

    std::vector<StringPart> parts;

    explicit LiteralInterpStringNode(std::vector<StringPart> p, SourceLocation loc = {})
        : Expression(NodeKind::LITERAL_INTERP_STRING, loc)
        , parts(std::move(p)) {}
};

struct LiteralBoolNode : public Expression {
    bool value;

    explicit LiteralBoolNode(bool v, SourceLocation loc = {})
        : Expression(NodeKind::LITERAL_BOOL, loc)
        , value(v) {}
};

/**
 * Array literal: [1, 2, 3, 4]
 */
struct LiteralArrayNode : public Expression {
    std::vector<ExpressionPtr> elements;

    explicit LiteralArrayNode(std::vector<ExpressionPtr> elems, SourceLocation loc = {})
        : Expression(NodeKind::LITERAL_ARRAY, loc)
        , elements(std::move(elems)) {}
};

/**
 * Map literal: {"a": 1, "b": 2, "c": 3}
 */
struct LiteralMapNode : public Expression {
    std::vector<std::pair<ExpressionPtr, ExpressionPtr>> pairs;

    explicit LiteralMapNode(std::vector<std::pair<ExpressionPtr, ExpressionPtr>> p, SourceLocation loc = {})
        : Expression(NodeKind::LITERAL_MAP)
        , pairs(std::move(p)) {}
    };

// ============================================================================
// NAME AND ACCESS EXPRESSIONS
// ============================================================================

/**
 * Base class for name-like expressions: identifiers and qualified names
 */
struct Name : public Expression {
    // Convert this name expression to a fully qualified string (e.g., "math::real")
    virtual std::string to_string() const = 0;

protected:
    explicit Name(NodeKind k, SourceLocation loc = {})
        : Expression(k, loc) {}
};

/**
 * Simple identifier: x, myVar, functionName
 */
struct NameExpression : public Name {
    std::string name;

    explicit NameExpression(std::string n, SourceLocation loc = {})
        : Name(NodeKind::NAME_EXPR, loc)
        , name(std::move(n)) {}

    std::string to_string() const override {
        return name;
    }
};

/**
 * Qualified name access: obj.field, math::sin, std::vector::value_type
 */
struct MemberAccessExpr : public Name {
    ExpressionPtr object;
    std::string member_name;
    bool is_static;  // true for :: access, false for . access

    MemberAccessExpr(ExpressionPtr obj, std::string member,
                     bool is_static = false, SourceLocation loc = {})
        : Name(NodeKind::MEMBER_ACCESS, loc)
        , object(std::move(obj))
        , member_name(std::move(member))
        , is_static(is_static) {}

    std::string to_string() const override {
        if (auto name_ptr = std::dynamic_pointer_cast<Name>(object)) {
            std::string sep = is_static ? "::" : ".";
            return name_ptr->to_string() + sep + member_name;
        }
        return member_name;  // Fallback
    }
};

// ============================================================================
// OPERATOR EXPRESSIONS
// ============================================================================

struct BinaryExprNode : public Expression {
    BinaryOp op;
    ExpressionPtr left;
    ExpressionPtr right;

    BinaryExprNode(BinaryOp operation, ExpressionPtr l, ExpressionPtr r, SourceLocation loc = {})
        : Expression(NodeKind::BINARY_EXPR, loc)
        , op(operation)
        , left(std::move(l))
        , right(std::move(r)) {}
};

struct UnaryExprNode : public Expression {
    UnaryOp op;
    ExpressionPtr operand;

    UnaryExprNode(UnaryOp operation, ExpressionPtr operand_expr, SourceLocation loc = {})
        : Expression(NodeKind::UNARY_EXPR, loc)
        , op(operation)
        , operand(std::move(operand_expr)) {}
};

struct TernaryExprNode : public Expression {
    ExpressionPtr condition;
    ExpressionPtr true_expr;
    ExpressionPtr false_expr;

    TernaryExprNode(ExpressionPtr cond, ExpressionPtr true_e, ExpressionPtr false_e,
                    SourceLocation loc = {})
        : Expression(NodeKind::TERNARY_EXPR, loc)
        , condition(std::move(cond))
        , true_expr(std::move(true_e))
        , false_expr(std::move(false_e)) {}
};

struct UnpackExprNode : public Expression {
    std::vector<ExpressionPtr> lvalues;
    std::vector<ExpressionPtr> rvalues;
    BinaryOp op;

    UnpackExprNode(std::vector<ExpressionPtr> lvs, std::vector<ExpressionPtr>  rvs, BinaryOp op, SourceLocation loc = {})
        : Expression(NodeKind::UNPACK_EXPR, loc)
        , lvalues(std::move(lvs))
        , rvalues(std::move(rvs))
        , op(op) {}
};

// ============================================================================
// CALL AND INDEX EXPRESSIONS
// ============================================================================

/**
 * Function call with named or positional arguments.
 */
struct CallExprNode : public Expression {
    struct Argument {
        ExpressionPtr value;
        std::string name;  // empty if positional
    };
    ExpressionPtr callee;
    std::vector<Argument> arguments;

    CallExprNode(ExpressionPtr callee_expr, std::vector<Argument> args, SourceLocation loc = {})
        : Expression(NodeKind::CALL_EXPR, loc)
        , callee(std::move(callee_expr))
        , arguments(std::move(args)) {}
};

/**
 * Array/container indexing: arr[0], matrix[i][j]
 */
struct IndexExprNode : public Expression {
    ExpressionPtr object;
    ExpressionPtr index;

    IndexExprNode(ExpressionPtr obj, ExpressionPtr idx, SourceLocation loc = {})
        : Expression(NodeKind::INDEX_EXPR, loc)
        , object(std::move(obj))
        , index(std::move(idx)) {}
};

/**
 * Generic type instantiation in expression context: Pair<Int, String>(42, "hello")
 */
struct GenericInstantiationExpr : public Expression {
    ExpressionPtr base;                       // usually a NameExpression
    std::vector<TypeExprPtr> type_arguments;  // <Int, String>

    GenericInstantiationExpr(ExpressionPtr base_expr, std::vector<TypeExprPtr> type_args,
                             SourceLocation loc = {})
        : Expression(NodeKind::GENERIC_INSTANTIATION, loc)
        , base(std::move(base_expr))
        , type_arguments(std::move(type_args)) {}
};

// ============================================================================
// LAMBDA EXPRESSIONS
// ============================================================================

struct LambdaExprNode : public Expression {
    struct Parameter {
        std::string name;
        TypeExprPtr type;
        ExpressionPtr default_value;  // optional

        Parameter(std::string n, TypeExprPtr t, ExpressionPtr def = nullptr)
            : name(std::move(n)), type(std::move(t)), default_value(std::move(def)) {}
    };

    GenericParams generic_params;      // <T:Printable, U>
    std::vector<Parameter> parameters; // (x:Int, y:Int)
    TypeExprPtr return_type;           // => Int  (nullptr means inferred/void)
    StatementPtr body;                 // { return x + y; }

    LambdaExprNode(GenericParams generics,
                   std::vector<Parameter> params,
                   TypeExprPtr ret_type,
                   StatementPtr body_stmt,
                   SourceLocation loc = {})
        : Expression(NodeKind::LAMBDA_EXPR, loc)
        , generic_params(std::move(generics))
        , parameters(std::move(params))
        , return_type(std::move(ret_type))
        , body(std::move(body_stmt)) {}
};

// ============================================================================
// STATEMENTS
// ============================================================================

struct BlockStatement : public Statement {
    std::vector<StatementPtr> statements;

    explicit BlockStatement(std::vector<StatementPtr> stmts = {}, SourceLocation loc = {})
        : Statement(NodeKind::BLOCK_STMT, loc)
        , statements(std::move(stmts)) {}
};

struct ExpressionStmt : public Statement {
    ExpressionPtr expression;

    explicit ExpressionStmt(ExpressionPtr expr, SourceLocation loc = {})
        : Statement(NodeKind::EXPRESSION_STMT, loc)
        , expression(std::move(expr)) {}
};

// ============================================================================
// CONTROL FLOW STATEMENTS
// ============================================================================

struct IfStatementNode : public Statement {
    ExpressionPtr condition;
    Ptr<BlockStatement> then_block;
    std::vector<std::pair<ExpressionPtr, Ptr<BlockStatement>>> elif_branches;
    Ptr<BlockStatement> else_block;  // optional

    IfStatementNode(ExpressionPtr cond,
                    Ptr<BlockStatement> then_b,
                    std::vector<std::pair<ExpressionPtr, Ptr<BlockStatement>>> elif_b = {},
                    Ptr<BlockStatement> else_b = nullptr,
                    SourceLocation loc = {})
        : Statement(NodeKind::IF_STMT, loc)
        , condition(std::move(cond))
        , then_block(std::move(then_b))
        , elif_branches(std::move(elif_b))
        , else_block(std::move(else_b)) {}
};

struct WhileLoopNode : public Statement {
    ExpressionPtr condition;
    Ptr<BlockStatement> body;

    WhileLoopNode(ExpressionPtr cond, Ptr<BlockStatement> b, SourceLocation loc = {})
        : Statement(NodeKind::WHILE_STMT, loc)
        , condition(std::move(cond))
        , body(std::move(b)) {}
};

struct DoWhileLoopNode : public Statement {
    Ptr<BlockStatement> body;
    ExpressionPtr condition;

    DoWhileLoopNode(Ptr<BlockStatement> b, ExpressionPtr cond, SourceLocation loc = {})
        : Statement(NodeKind::DO_WHILE_STMT, loc)
        , body(std::move(b))
        , condition(std::move(cond)) {}
};

/**
 * C-style for loop: for (let i:Int = 0; i < 10; i++) { }
 */
struct ForLoopNode : public Statement {
    StatementPtr initialiser;  // let i:Int = 0  or expression statement
    ExpressionPtr condition;   // i < 10
    ExpressionPtr increment;   // i++
    Ptr<BlockStatement> body;

    ForLoopNode(StatementPtr init, ExpressionPtr cond, ExpressionPtr inc,
                Ptr<BlockStatement> b, SourceLocation loc = {})
        : Statement(NodeKind::FOR_STMT, loc)
        , initialiser(std::move(init))
        , condition(std::move(cond))
        , increment(std::move(inc))
        , body(std::move(b)) {}
};

/**
 * Range-based for loop: foreach (item in collection) { }
 */
struct ForEachLoopNode : public Statement {
    std::string iterator_name;
    TypeExprPtr iterator_type;  // optional (can be inferred)
    ExpressionPtr iterable;
    Ptr<BlockStatement> body;

    ForEachLoopNode(std::string iter_name, TypeExprPtr iter_type,
                    ExpressionPtr iter_expr, Ptr<BlockStatement> b, SourceLocation loc = {})
        : Statement(NodeKind::FOREACH_STMT, loc)
        , iterator_name(std::move(iter_name))
        , iterator_type(std::move(iter_type))
        , iterable(std::move(iter_expr))
        , body(std::move(b)) {}
};

// ============================================================================
// JUMP STATEMENTS
// ============================================================================

struct ReturnStatementNode : public Statement {
    ExpressionPtr value;  // optional (void return)

    explicit ReturnStatementNode(ExpressionPtr val = nullptr, SourceLocation loc = {})
        : Statement(NodeKind::RETURN_STMT, loc)
        , value(std::move(val)) {}
};

struct BreakStatementNode : public Statement {
    explicit BreakStatementNode(SourceLocation loc = {})
        : Statement(NodeKind::BREAK_STMT, loc) {}
};

struct ContinueStatementNode : public Statement {
    explicit ContinueStatementNode(SourceLocation loc = {})
        : Statement(NodeKind::CONTINUE_STMT, loc) {}
};

// ============================================================================
// EXCEPTION HANDLING
// ============================================================================

struct ThrowStatementNode : public Statement {
    ExpressionPtr exception;

    explicit ThrowStatementNode(ExpressionPtr exc, SourceLocation loc = {})
        : Statement(NodeKind::THROW_STMT, loc)
        , exception(std::move(exc)) {}
};

struct TryCatchStatementNode : public Statement {
    struct CatchClause {
        std::string exception_name;  // "e"
        TypeExprPtr exception_type;  // IOError
        Ptr<BlockStatement> handler;
    };

    Ptr<BlockStatement> try_block;
    std::vector<CatchClause> catch_clauses;
    Ptr<BlockStatement> finally_block;  // optional

    TryCatchStatementNode(Ptr<BlockStatement> try_b,
                          std::vector<CatchClause> catches,
                          Ptr<BlockStatement> finally_b = nullptr,
                          SourceLocation loc = {})
        : Statement(NodeKind::TRY_CATCH_STMT, loc)
        , try_block(std::move(try_b))
        , catch_clauses(std::move(catches))
        , finally_block(std::move(finally_b)) {}
};

// ============================================================================
// DECLARATIONS
// ============================================================================

/**
 * Variable declaration: let x:Int = 42;
 */
struct VariableDeclNode : public Declaration {
    std::string name;
    TypeExprPtr type;           // nullptr means inferred
    ExpressionPtr initialiser;  // optional
    TagList directives;

    VariableDeclNode(std::string n, TypeExprPtr t, ExpressionPtr init = nullptr,
                     TagList dirs = {}, SourceLocation loc = {})
        : Declaration(NodeKind::VARIABLE_DECL, loc)
        , name(std::move(n))
        , type(std::move(t))
        , initialiser(std::move(init))
        , directives(std::move(dirs)) {}
};

/**
 * Destructuring declaration:
 *   let (first, second) = expr;              //  single RHS, types inferred
 *   let (x:Int, y:String) = expr;            //  single RHS, types explicit

 */
struct DestructureDeclNode : public Declaration {

    struct Binding {
        std::string name;
        TypeExprPtr type;       // usually inferred, but can be explicit

        Binding(std::string n, TypeExprPtr t = nullptr)
            : name(std::move(n)), type(std::move(t)) {}
    };

    std::vector<Binding> bindings;


    ExpressionPtr value;

    TagList directives;

    DestructureDeclNode(std::vector<Binding> b,
                        ExpressionPtr val,
                        TagList dirs = {}, SourceLocation loc = {})
        : Declaration(NodeKind::DESTRUCTURE_DECL, loc)
        , bindings(std::move(b))
        , value(std::move(val))
        , directives(std::move(dirs)) {}
};

/**
 * Function declaration
 */
struct FunctionDeclNode : public Declaration {
    struct Parameter {
        std::string name;
        TypeExprPtr type;
        ExpressionPtr default_value;  // optional

        Parameter(std::string n, TypeExprPtr t, ExpressionPtr def = nullptr)
            : name(std::move(n)), type(std::move(t)), default_value(std::move(def)) {}
    };

    std::string name;
    GenericParams generic_params;       // <T, U:Printable>
    std::vector<Parameter> parameters;
    TypeExprPtr return_type;            // nullptr means Void
    Ptr<BlockStatement> body;           // nullptr for prototypes (trait requirements etc.)
    TagList directives;

    FunctionDeclNode(std::string n,
                     GenericParams generics,
                     std::vector<Parameter> params,
                     TypeExprPtr ret_type,
                     Ptr<BlockStatement> b,
                     TagList dirs = {},
                     SourceLocation loc = {})
        : Declaration(NodeKind::FUNCTION_DECL, loc)
        , name(std::move(n))
        , generic_params(std::move(generics))
        , parameters(std::move(params))
        , return_type(std::move(ret_type))
        , body(std::move(b))
        , directives(std::move(dirs)) {}
};

/**
 * Struct declaration with members
 */
struct StructDeclNode : public Declaration {
    struct Member {
        DeclarationPtr declaration;
        bool is_private;

        Member(DeclarationPtr decl, bool priv = false)
            : declaration(std::move(decl)), is_private(priv) {}
    };

    std::string name;
    GenericParams generic_params;
    std::vector<TraitConstraint> constraints;
    std::vector<Member> members;
    TagList directives;

    StructDeclNode(std::string n,
                   GenericParams generics = {},
                   std::vector<TraitConstraint> c = {},
                   std::vector<Member> mems = {},
                   TagList dirs = {},
                   SourceLocation loc = {})
        : Declaration(NodeKind::STRUCT_DECL, loc)
        , name(std::move(n))
        , generic_params(std::move(generics))
        , constraints(std::move(c))
        , members(std::move(mems))
        , directives(std::move(dirs)) {}
};

/**
 * Operator overload declaration
 */

enum class OperatorOverloadType {
    UNARY_PLUS, UNARY_MINUS,
    BINARY_PLUS, BINARY_MINUS, BINARY_MULTIPLY, BINARY_DIVIDE,
    EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
    COMPOUND_ADD, COMPOUND_SUB, COMPOUND_MUL, COMPOUND_DIV,
    INDEX,  // operator[]
};

struct OperatorOverloadNode : public Declaration {


    OperatorOverloadType op_type;
    std::vector<FunctionDeclNode::Parameter> parameters;
    TypeExprPtr return_type;
    Ptr<BlockStatement> body;
    TagList directives;

    OperatorOverloadNode(OperatorOverloadType op,
                         std::vector<FunctionDeclNode::Parameter> params,
                         TypeExprPtr ret_type,
                         Ptr<BlockStatement> b,
                         TagList dirs = {},
                         SourceLocation loc = {})
        : Declaration(NodeKind::OPERATOR_OVERLOAD_DECL, loc)
        , op_type(op)
        , parameters(std::move(params))
        , return_type(std::move(ret_type))
        , body(std::move(b))
        , directives(std::move(dirs)) {}
};

/**
 * Trait declaration for interface/constraint definitions
 */
struct TraitDeclNode : public Declaration {
    // Base for all requirements
    struct Requirement {
        virtual ~Requirement() = default;
    };

    // "Self:Printable"
    struct TypeRequirement : public Requirement {
        std::string type_param;      // "Self" or "T"
        TraitConstraint constraint;  // "Printable" or "Comparable<T>"

        TypeRequirement(std::string param, TraitConstraint c)
            : type_param(std::move(param)), constraint(std::move(c)) {}
    };

    // "func toString() -> String"
    struct MethodRequirement : public Requirement {
        std::string name;
        std::vector<TypeExprPtr> parameters;
        TypeExprPtr return_type;
        bool is_static;

        MethodRequirement(std::string n, std::vector<TypeExprPtr> params,
                          TypeExprPtr ret, bool s = false)
            : name(std::move(n)), parameters(std::move(params))
            , return_type(std::move(ret)), is_static(s) {}
    };

    // "operator+(Self) -> Self"
    struct OperatorRequirement : public Requirement {
        OperatorOverloadType operator_type;
        std::vector<TypeExprPtr> parameters;
        TypeExprPtr return_type;

        OperatorRequirement(OperatorOverloadType op,
                            std::vector<TypeExprPtr> params, TypeExprPtr ret)
            : operator_type(op), parameters(std::move(params))
            , return_type(std::move(ret)) {}
    };

    std::string name;
    GenericParams generic_params;
    std::vector<Ptr<Requirement>> requirements;

    TraitDeclNode(std::string n,
                  GenericParams generics = {},
                  std::vector<Ptr<Requirement>> reqs = {},
                  SourceLocation loc = {})
        : Declaration(NodeKind::TRAIT_DECL, loc)
        , name(std::move(n))
        , generic_params(std::move(generics))
        , requirements(std::move(reqs)) {}  // BUG FIX: was discarding reqs
};

/**
 * Type alias: type Point2D = Pair<Float, Float>;
 */
struct TypeAliasDeclNode : public Declaration {
    std::string alias_name;
    GenericParams generic_params;
    TypeExprPtr target_type;

    TypeAliasDeclNode(std::string name,
                      GenericParams generics,
                      TypeExprPtr target,
                      SourceLocation loc = {})
        : Declaration(NodeKind::TYPE_ALIAS_DECL, loc)
        , alias_name(std::move(name))
        , generic_params(std::move(generics))
        , target_type(std::move(target)) {}
};

/**
 * Enum declaration: enum Colours { RED, ORANGE, YELLOW }
 */
struct EnumDeclNode : public Declaration {
    std::string name;
    std::vector<std::string> variants;

    EnumDeclNode(std::string n, std::vector<std::string> vars, SourceLocation loc = {})
        : Declaration(NodeKind::ENUM_DECL, loc)
        , name(std::move(n))
        , variants(std::move(vars)) {}
};

/**
 * Scope declaration: scope math { ... }
 */
struct ScopeDeclNode : public Declaration {
    std::string name;
    std::vector<DeclarationPtr> declares;

    ScopeDeclNode(std::string n, std::vector<DeclarationPtr> decls, SourceLocation loc = {})
        : Declaration(NodeKind::SCOPE_DECL)
        , name(std::move(n))
        , declares(std::move(decls)) {}
};

// ============================================================================
// MODULE SYSTEM
// ============================================================================

/**
 * Import declaration: import "std/math" { sin, cos };
 */
struct ImportDeclNode : public Statement {
    struct ImportedSymbol {
        std::string original_name;
        std::string alias;  // empty if no alias
    };

    std::string module_path;
    std::optional<std::string> module_alias;  // "m" in "as m"
    std::vector<ImportedSymbol> symbols;      // empty = import all / just the module

    ImportDeclNode(std::string path,
                   std::optional<std::string> alias = std::nullopt,
                   std::vector<ImportedSymbol> syms = {},
                   SourceLocation loc = {})
        : Statement(NodeKind::IMPORT_DECL, loc)
        , module_path(std::move(path))
        , module_alias(std::move(alias))
        , symbols(std::move(syms)) {}
};

/**
 * Export declaration: export { Point3D, add, Colours };
 */
struct ExportDeclNode : public Statement {
    std::vector<std::string> symbols;

    explicit ExportDeclNode(std::vector<std::string> syms, SourceLocation loc = {})
        : Statement(NodeKind::EXPORT_DECL, loc)
        , symbols(std::move(syms)) {}
};

struct EOFStatement : public Statement {
    explicit EOFStatement(SourceLocation loc = {})
        : Statement(NodeKind::EOF_STATEMENT, loc) {}
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

inline const char* node_kind_to_string(NodeKind kind) {
    switch (kind) {
        // Types
        case NodeKind::TYPE_SIMPLE:             return "TYPE_SIMPLE";
        case NodeKind::TYPE_GENERIC:            return "TYPE_GENERIC";
        case NodeKind::TYPE_REFERENCE:          return "TYPE_REFERENCE";

        // Literals
        case NodeKind::LITERAL_INT:             return "LITERAL_INT";
        case NodeKind::LITERAL_FLOAT:           return "LITERAL_FLOAT";
        case NodeKind::LITERAL_STRING:          return "LITERAL_STRING";
        case NodeKind::LITERAL_RAW_STRING:      return "LITERAL_RAW_STRING";
        case NodeKind::LITERAL_INTERP_STRING:   return "LITERAL_INTERP_STRING";
        case NodeKind::LITERAL_BOOL:            return "LITERAL_BOOL";
        case NodeKind::LITERAL_ARRAY:           return "LITERAL_ARRAY";

        // Names
        case NodeKind::NAME_EXPR:               return "NAME_EXPR";
        case NodeKind::MEMBER_ACCESS:           return "MEMBER_ACCESS";

        // Operations
        case NodeKind::BINARY_EXPR:             return "BINARY_EXPR";
        case NodeKind::UNARY_EXPR:              return "UNARY_EXPR";
        case NodeKind::TERNARY_EXPR:            return "TERNARY_EXPR";

        // Calls
        case NodeKind::CALL_EXPR:               return "CALL_EXPR";
        case NodeKind::INDEX_EXPR:              return "INDEX_EXPR";
        case NodeKind::GENERIC_INSTANTIATION:   return "GENERIC_INSTANTIATION";

        // Lambda
        case NodeKind::LAMBDA_EXPR:             return "LAMBDA_EXPR";

        // Statements
        case NodeKind::BLOCK_STMT:              return "BLOCK_STMT";
        case NodeKind::EXPRESSION_STMT:         return "EXPRESSION_STMT";
        case NodeKind::IF_STMT:                 return "IF_STMT";
        case NodeKind::WHILE_STMT:              return "WHILE_STMT";
        case NodeKind::DO_WHILE_STMT:           return "DO_WHILE_STMT";
        case NodeKind::FOR_STMT:                return "FOR_STMT";
        case NodeKind::FOREACH_STMT:            return "FOREACH_STMT";
        case NodeKind::RETURN_STMT:             return "RETURN_STMT";
        case NodeKind::BREAK_STMT:              return "BREAK_STMT";
        case NodeKind::CONTINUE_STMT:           return "CONTINUE_STMT";
        case NodeKind::THROW_STMT:              return "THROW_STMT";
        case NodeKind::TRY_CATCH_STMT:          return "TRY_CATCH_STMT";

        // Declarations
        case NodeKind::VARIABLE_DECL:           return "VARIABLE_DECL";
        case NodeKind::FUNCTION_DECL:           return "FUNCTION_DECL";
        case NodeKind::STRUCT_DECL:             return "STRUCT_DECL";
        case NodeKind::TRAIT_DECL:              return "TRAIT_DECL";
        case NodeKind::TYPE_ALIAS_DECL:         return "TYPE_ALIAS_DECL";
        case NodeKind::OPERATOR_OVERLOAD_DECL:  return "OPERATOR_OVERLOAD_DECL";
        case NodeKind::ENUM_DECL:               return "ENUM_DECL";

        // Module
        case NodeKind::IMPORT_DECL:             return "IMPORT_DECL";
        case NodeKind::EXPORT_DECL:             return "EXPORT_DECL";

        // Tags
        case NodeKind::TAG_STATICMEMBER:        return "TAG_STATICMEMBER";
        case NodeKind::TAG_CONSTMETHOD:         return "TAG_CONSTMETHOD";
        case NodeKind::TAG_DEPRECATED:          return "TAG_DEPRECATED";
        case NodeKind::TAG_CONSTEXPR:           return "TAG_CONSTEXPR";
        case NodeKind::TAG_INLINE:              return "TAG_INLINE";
        case NodeKind::TAG_DOCSTRING:           return "TAG_DOCSTRING";

        case NodeKind::EOF_STATEMENT:            return "EOF_STATEMENT";

        default: return "UNKNOWN";
    }
}

inline const char* binary_op_to_string(BinaryOp op) {
    switch (op) {
        case BinaryOp::ADD:                 return "+";
        case BinaryOp::SUBTRACT:            return "-";
        case BinaryOp::MULTIPLY:            return "*";
        case BinaryOp::DIVIDE:              return "/";
        case BinaryOp::MODULO:              return "%";
        case BinaryOp::EQUAL:               return "==";
        case BinaryOp::NOT_EQUAL:           return "!=";
        case BinaryOp::LESS:                return "<";
        case BinaryOp::LESS_EQUAL:          return "<=";
        case BinaryOp::GREATER:             return ">";
        case BinaryOp::GREATER_EQUAL:       return ">=";
        case BinaryOp::LOGICAL_AND:         return "&&";
        case BinaryOp::LOGICAL_OR:          return "||";
        case BinaryOp::BIT_AND:             return "&";
        case BinaryOp::BIT_OR:              return "|";
        case BinaryOp::BIT_XOR:             return "^";
        case BinaryOp::SHIFT_LEFT:          return "<<";
        case BinaryOp::SHIFT_RIGHT:         return ">>";
        case BinaryOp::ASSIGN:              return "=";
        case BinaryOp::ADD_ASSIGN:          return "+=";
        case BinaryOp::SUBTRACT_ASSIGN:     return "-=";
        case BinaryOp::MULTIPLY_ASSIGN:     return "*=";
        case BinaryOp::DIVIDE_ASSIGN:       return "/=";
        case BinaryOp::MODULO_ASSIGN:       return "%=";
        case BinaryOp::BIT_AND_ASSIGN:      return "&=";
        case BinaryOp::BIT_OR_ASSIGN:       return "|=";
        case BinaryOp::BIT_XOR_ASSIGN:      return "^=";
        case BinaryOp::TILDE_ASSIGN:        return "~=";
        case BinaryOp::SHIFT_LEFT_ASSIGN:   return "<<=";
        case BinaryOp::SHIFT_RIGHT_ASSIGN:  return ">>=";
        default: return "?";
    }
}

inline const char* unary_op_to_string(UnaryOp op) {
    switch (op) {
        case UnaryOp::PLUS:         return "+";
        case UnaryOp::MINUS:        return "-";
        case UnaryOp::LOGICAL_NOT:  return "!";
        case UnaryOp::BIT_NOT:      return "~";
        case UnaryOp::ADDRESS_OF:   return "&";
        default: return "?";
    }
}

inline const char* operator_type_to_string(OperatorOverloadType op) {
    switch (op) {
        case OperatorOverloadType::UNARY_PLUS:      return "unary +";
        case OperatorOverloadType::UNARY_MINUS:     return "unary -";
        case OperatorOverloadType::BINARY_PLUS:     return "+";
        case OperatorOverloadType::BINARY_MINUS:    return "-";
        case OperatorOverloadType::BINARY_MULTIPLY: return "*";
        case OperatorOverloadType::BINARY_DIVIDE:   return "/";
        case OperatorOverloadType::EQUAL:           return "==";
        case OperatorOverloadType::NOT_EQUAL:       return "!=";
        case OperatorOverloadType::LESS:            return "<";
        case OperatorOverloadType::GREATER:         return ">";
        case OperatorOverloadType::LESS_EQUAL:      return "<=";
        case OperatorOverloadType::GREATER_EQUAL:   return ">=";
        case OperatorOverloadType::COMPOUND_ADD:    return "+=";
        case OperatorOverloadType::COMPOUND_SUB:    return "-=";
        case OperatorOverloadType::COMPOUND_MUL:    return "*=";
        case OperatorOverloadType::COMPOUND_DIV:    return "/=";
        case OperatorOverloadType::INDEX:           return "[]";
        default: return "?";
    }
}

inline const char* tag_kind_to_string(NodeKind kind) {
    switch (kind) {
        case NodeKind::TAG_STATICMEMBER: return "#staticmember";
        case NodeKind::TAG_CONSTMETHOD:  return "#constmethod";
        case NodeKind::TAG_DEPRECATED:   return "#deprecated";
        case NodeKind::TAG_CONSTEXPR:    return "#constexpr";
        case NodeKind::TAG_INLINE:       return "#inline";
        case NodeKind::TAG_DOCSTRING:    return "#docstring";
        default: return "unknown_tag";
    }
}

} // namespace espresso_compiler