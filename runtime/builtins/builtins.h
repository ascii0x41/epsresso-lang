#pragma once

#include <string>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <cmath>
#include <string_view>
#include <algorithm>
#include <utility>
#include <optional>
#include <type_traits>
#include <format>
#include <iostream>
#include <variant>


#define FORCE_INLINE __attribute__((always_inline)) inline
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

// Forward declarations
struct Int;
struct Float;
struct Complex;
struct Bool;
struct String;

// ===== Concepts ===
template<typename T>
concept Typed = requires() {
    // Has a class name
    { T::class_name } -> std::convertible_to<const char*>;
};

template<typename T>
concept Numeric = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    { a - b } -> std::convertible_to<T>;
    { a * b } -> std::convertible_to<T>;
    { a / b } -> std::convertible_to<T>;
};

template<typename T>
concept Printable = requires(T t) {
    // Has .toString() that returns String
    { t.toString() } -> std::convertible_to<String>;
};

template<typename T>
concept Hashable = requires(T t) {
    // Has .getHash() that returns Int
    { t.getHash() } -> std::convertible_to<Int>;
};

template<typename T>
struct Array;

template<typename A, typename B>
struct Pair;

// ===== Error Types (defined early for use in String) =====

struct RuntimeError : public std::exception {
    static constexpr const char* class_name = "runtime_error";
    std::string msg;
        
    RuntimeError(const std::string& s) : msg(s) {}
    RuntimeError(const char* s) : msg(s) {}
    RuntimeError(const String& s);
    
    const char* what() const noexcept override {
        return msg.c_str();
    }
    
    String getMessage() const;
};

struct ValueError : public RuntimeError {
    using RuntimeError::RuntimeError;
    
    static constexpr const char* class_name = "value_error";
};

struct TypeError : public RuntimeError {
    using RuntimeError::RuntimeError;
    
    static constexpr const char* class_name = "type_error";
};

struct IndexError : public RuntimeError {
    using RuntimeError::RuntimeError;

    static constexpr const char* class_name = "index_error";
};

struct KeyError : public RuntimeError {
    using RuntimeError::RuntimeError;
    
    static constexpr const char* class_name = "key_error";
};

struct ZeroDivisionError : public RuntimeError {
    using RuntimeError::RuntimeError;
    
    static constexpr const char* class_name = "zero_division_error";
};

struct NotImplimentedError : public RuntimeError {
    using RuntimeError::RuntimeError;
    static constexpr const char* class_name = "not_implimented_error";
};

// ===== Int, Float, Complex, Bool, String =====

// ===== Int =====
struct Int final {
    static constexpr const char* class_name = "int";
    int64_t value;

    // ========== Constructors ==========
    Int() = default;
    Int(const Int& other) = default;
    Int(Int&&) = default;
    Int& operator=(const Int& other) = default;
    Int& operator=(Int&&) = default;
    ~Int() = default;
    explicit Int(int64_t v) : value(v) {}

    // ========== Type Conversions ==========
    Int toInt() const;
    Float toFloat() const;
    Complex toComplex() const;
    Bool toBool() const;
    String toString() const;
    FORCE_INLINE Int getHash() const { return *this; }

    // ========== Arithmetic Operators ==========
    // With Int
    Int operator+(const Int& other) const;
    Int operator-(const Int& other) const;
    Int operator*(const Int& other) const;
    Int operator/(const Int& other) const;
    Int operator%(const Int& other) const;
    
    // With Float
    Float operator+(const Float& other) const;
    Float operator-(const Float& other) const;
    Float operator*(const Float& other) const;
    Float operator/(const Float& other) const;
    
    // With Complex
    Complex operator+(const Complex& other) const;
    Complex operator-(const Complex& other) const;
    Complex operator*(const Complex& other) const;
    Complex operator/(const Complex& other) const;

    // ========== Assignment Operators ==========
    Int& operator+=(const Int& other);
    Int& operator-=(const Int& other);
    Int& operator*=(const Int& other);
    Int& operator/=(const Int& other);
    Int& operator%=(const Int& other);

    // ========== Unary Operators ==========
    Int operator+() const;  // Unary plus
    Int operator-() const;  // Unary minus

    // ========== Comparison Operators ==========
    // With Int
    Bool operator==(const Int& other) const;
    Bool operator!=(const Int& other) const;
    Bool operator<(const Int& other) const;
    Bool operator>(const Int& other) const;
    Bool operator<=(const Int& other) const;
    Bool operator>=(const Int& other) const;

    // With Float
    Bool operator==(const Float& other) const;
    Bool operator!=(const Float& other) const;
    Bool operator<(const Float& other) const;
    Bool operator>(const Float& other) const;
    Bool operator<=(const Float& other) const;
    Bool operator>=(const Float& other) const;

    // With Complex
    Bool operator==(const Complex& other) const;
    Bool operator!=(const Complex& other) const;
    Bool operator<(const Complex& other) const;
    Bool operator>(const Complex& other) const;
    Bool operator<=(const Complex& other) const;
    Bool operator>=(const Complex& other) const;
};

// ===== Float =====
struct Float final {
    static constexpr const char* class_name = "float";
    double value;

    // ========== Constructors ==========
    Float() = default;
    Float(const Float& other) = default;
    Float(Float&&) = default;
    Float& operator=(const Float& other) = default;
    Float& operator=(Float&&) = default;
    ~Float() = default;
    explicit Float(double v) : value(v) {}

    // ========== Type Conversions ==========
    Int toInt() const;
    Float toFloat() const;
    Complex toComplex() const;
    Bool toBool() const;
    String toString() const;
    FORCE_INLINE Int getHash() const {
        static_assert(sizeof(double) == sizeof(int64_t));
        int64_t bits;
        std::memcpy(&bits, &value, sizeof(double));
        return Int(bits);
    }

    // ========== Arithmetic Operators ==========
    // With Int
    Float operator+(const Int& other) const;
    Float operator-(const Int& other) const;
    Float operator*(const Int& other) const;
    Float operator/(const Int& other) const;
    
    // With Float
    Float operator+(const Float& other) const;
    Float operator-(const Float& other) const;
    Float operator*(const Float& other) const;
    Float operator/(const Float& other) const;
    
    // With Complex
    Complex operator+(const Complex& other) const;
    Complex operator-(const Complex& other) const;
    Complex operator*(const Complex& other) const;
    Complex operator/(const Complex& other) const;

    // ========== Assignment Operators ==========
    Float& operator+=(const Float& other);
    Float& operator-=(const Float& other);
    Float& operator*=(const Float& other);
    Float& operator/=(const Float& other);

    // ========== Unary Operators ==========
    Float operator+() const;  // Unary plus
    Float operator-() const;  // Unary minus

    // ========== Comparison Operators ==========
    // With Int
    Bool operator==(const Int& other) const;
    Bool operator!=(const Int& other) const;
    Bool operator<(const Int& other) const;
    Bool operator>(const Int& other) const;
    Bool operator<=(const Int& other) const;
    Bool operator>=(const Int& other) const;

    // With Float
    Bool operator==(const Float& other) const;
    Bool operator!=(const Float& other) const;
    Bool operator<(const Float& other) const;
    Bool operator>(const Float& other) const;
    Bool operator<=(const Float& other) const;
    Bool operator>=(const Float& other) const;

    // With Complex
    Bool operator==(const Complex& other) const;
    Bool operator!=(const Complex& other) const;
    Bool operator<(const Complex& other) const;
    Bool operator>(const Complex& other) const;
    Bool operator<=(const Complex& other) const;
    Bool operator>=(const Complex& other) const;

};

// ===== Complex =====
struct Complex final {
    static constexpr const char* class_name = "complex";
    double re, im;

    // ========== Constructors ==========
    Complex() = default;
    Complex(const Complex& other) = default;
    Complex(Complex&&) = default;
    Complex& operator=(const Complex& other) = default;
    Complex& operator=(Complex&&) = default;
    ~Complex() = default;
    explicit Complex(double r, double i) : re(r), im(i) {}

    // Conversion constructors from pairs
    constexpr Complex(const Int& r, const Int& i) 
        : re(static_cast<double>(r.value)), im(static_cast<double>(i.value)) {}
    constexpr Complex(const Float& r, const Float& i) 
        : re(r.value), im(i.value) {}

    // ========== Type Conversions ==========
    Int toInt() const;
    Float toFloat() const;
    Complex toComplex() const;
    Bool toBool() const;
    String toString() const;
    FORCE_INLINE Int getHash() const {
        size_t getHash_value = std::hash<double>()(re);
        getHash_value ^= std::hash<double>()(im) + 0x9e3779b9 + (getHash_value << 6) + (getHash_value >> 2);
        return Int(static_cast<int64_t>(getHash_value));
    }

    // ========== Arithmetic Operators ==========
    // With Int
    Complex operator+(const Int& other) const;
    Complex operator-(const Int& other) const;
    Complex operator*(const Int& other) const;
    Complex operator/(const Int& other) const;
    
    // With Float
    Complex operator+(const Float& other) const;
    Complex operator-(const Float& other) const;
    Complex operator*(const Float& other) const;
    Complex operator/(const Float& other) const;
    
    // With Complex
    Complex operator+(const Complex& other) const;
    Complex operator-(const Complex& other) const;
    Complex operator*(const Complex& other) const;
    Complex operator/(const Complex& other) const;

    // ========== Assignment Operators ==========
    Complex& operator+=(const Complex& other);
    Complex& operator-=(const Complex& other);
    Complex& operator*=(const Complex& other);
    Complex& operator/=(const Complex& other);
    Complex& operator%=(const Complex& other);

    // ========== Unary Operators ==========
    Complex operator+() const;  // Unary plus
    Complex operator-() const;  // Unary minus

    // ========== Comparison Operators ==========
    // With Int
    Bool operator==(const Int& other) const;
    Bool operator!=(const Int& other) const;
    Bool operator<(const Int& other) const;
    Bool operator>(const Int& other) const;
    Bool operator<=(const Int& other) const;
    Bool operator>=(const Int& other) const;

    // With Float
    Bool operator==(const Float& other) const;
    Bool operator!=(const Float& other) const;
    Bool operator<(const Float& other) const;
    Bool operator>(const Float& other) const;
    Bool operator<=(const Float& other) const;
    Bool operator>=(const Float& other) const;

    // With Complex
    Bool operator==(const Complex& other) const;
    Bool operator!=(const Complex& other) const;
    Bool operator<(const Complex& other) const;
    Bool operator>(const Complex& other) const;
    Bool operator<=(const Complex& other) const;
    Bool operator>=(const Complex& other) const;

    // ========== Mathematical Operations ==========
    static Complex fromPolar(const Float& rad, const Float& theta) { return Complex(rad.value * std::cos(theta.value), rad.value * std::sin(theta.value)); }
    Float argument() const;
    Complex conjugate() const;
    Float magnitude() const;
};

// ===== Bool =====
struct Bool final {
    bool value;

    constexpr Bool() = default;
    Bool(const Bool& other) = default;
    Bool(Bool&&) = default;
    Bool& operator=(const Bool& other) = default;
    Bool& operator=(Bool&&) = default;
    ~Bool() = default;
    constexpr explicit Bool(bool v) : value(v) {}

    Int toInt() const;
    Float toFloat() const;
    Complex toComplex() const;
    Bool toBool() const;
    String toString() const;
    FORCE_INLINE Int getHash() const { return Int(value ? 1 : 0); }

    constexpr FORCE_INLINE operator bool() const { return value; }

    constexpr Bool operator!=(const Bool& other) const;
    constexpr Bool operator==(const Bool& other) const;
};

// ===== String =====
struct String final {
    static constexpr const char* class_name = "string";
    std::vector<char> bytes;
    mutable std::vector<size_t> char_starts;
    mutable bool indexed;

    String() = default;
    String(const String&) = default;
    String(String&&) = default;
    String& operator=(const String&) = default;
    String& operator=(String&&) = default;
    ~String() = default;
    explicit String(std::string s);

    Int toInt() const;
    Float toFloat() const;
    String toString() const;
    Bool toBool() const;
    FORCE_INLINE Int getHash() const {
        size_t getHash_value = 0;
        for (char c : bytes) {
            getHash_value = getHash_value * 31 + static_cast<unsigned char>(c);
        }
        return Int(static_cast<int64_t>(getHash_value));
    }

    const char* c_str() const;

    Int length() const;
    String operator[](const Int& index) const;


    String operator+(const String& other) const;
    String& operator+=(const String& other);
    String operator*(const Int& times) const;
    String& operator*=(const Int& times);

    Bool operator==(const String& other) const;
    Bool operator!=(const String& other) const;

    Bool isUpper() const;
    Bool isLower() const;
    Bool isAlpha() const;
    Bool isDigit() const;
    Bool isAlnum() const;
    Bool isWhitespace() const;

    String toUpper() const;
    String toLower() const;

    Bool contains(const String& substr) const;
    Bool startsWith(const String& prefix) const;
    Bool endsWith(const String& suffix) const;
    Int find(const String& substr) const;
    Int rfind(const String& substr) const;

    String strip() const;
    String lstrip() const;
    String rstrip() const;
    String replace(const String& old_str, const String& new_str) const;

    template<Printable T>
    String join(const Array<T>& arr) const;
    String substring(const Int& start, const Int& length) const;

    void clear();
    void reserve(Int capacity);

    friend std::ostream& operator<<(std::ostream& os, const String& s) {
        os << s.c_str();
        return os;
    }

private:
    void build_index() const {
        if (indexed) return;
        char_starts.clear();
        size_t i = 0;
        while (i < bytes.size()) {
            char_starts.push_back(i);
            unsigned char c = static_cast<unsigned char>(bytes[i]);
            size_t len = utf8_char_length(c);
            if (len == 0 || i + len > bytes.size()) {
                throw ValueError("Invalid UTF-8 sequence");
            }
            // Validate continuation bytes
            for (size_t j = 1; j < len; ++j) {
                if ((static_cast<unsigned char>(bytes[i + j]) & 0xC0) != 0x80) {
                    throw ValueError("Invalid UTF-8 continuation byte");
                }
            }
            i += len;
        }
        indexed = true;
    }
    
    size_t utf8_char_length(unsigned char first_byte) const {
        if (first_byte < 0x80) return 1;
        if ((first_byte & 0xE0) == 0xC0) return 2;
        if ((first_byte & 0xF0) == 0xE0) return 3;
        if ((first_byte & 0xF8) == 0xF0) return 4;
        return 0;  // Invalid
    }
};

// ===== Array, Set, Map =====

// ===== Array =====
template<typename T>
struct Array final {
    static constexpr const char* class_name = "array";
    std::vector<T> data;

    Array() = default;
    Array(const Array<T>&) = default;
    Array(Array&&) = default;
    Array<T>& operator=(const Array<T>&) = default;
    Array<T>& operator=(Array<T>&&) = default;
    ~Array() = default;
    explicit Array(std::vector<T> v) : data(std::move(v)) {}

    String toString() const 
    requires Printable<T> {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < data.size(); ++i) {
            oss << data[i].toString();
            if (i + 1 < data.size()) {
                oss << ", ";
            }
        }
        oss << "]";
        return String(oss.str());
    }
    constexpr Int getHash() const {
        size_t getHash_value = 0;
        for (const T& item : data) {
            getHash_value = getHash_value * 31 + static_cast<size_t>(item.getHash().value);
        }
        return Int(static_cast<int64_t>(getHash_value));
    }

    T& operator[](Int index) {
        if (UNLIKELY(index.value < 0 || index.value >= static_cast<int64_t>(data.size()))) {
            throw IndexError("Array index out of bounds");
        }
        return data[index.value];
    }
    const T& operator[](Int index) const {
        if (UNLIKELY(index.value < 0 || index.value >= static_cast<int64_t>(data.size()))) {
            throw IndexError("Array index out of bounds");
        }
        return data[index.value];
    }

    Int length() const {
        return Int(static_cast<int64_t>(data.size()));
    }
    Bool empty() const {
        return Bool(data.empty());
    }


    struct Iterator {
        T* ptr;

        Iterator(T* p) : ptr(p) {}

        T& operator*() { return *ptr; }
        Iterator& operator++() { ++ptr; return *this; }
        bool operator==(const Iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
    };

    Iterator begin() { return Iterator(data.data()); }
    Iterator end()   { return Iterator(data.data() + data.size()); }

    // const versions for read-only contexts
    struct ConstIterator {
        const T* ptr;

        ConstIterator(const T* p) : ptr(p) {}

        const T& operator*() const { return *ptr; }
        ConstIterator& operator++() { ++ptr; return *this; }
        bool operator==(const ConstIterator& other) const { return ptr == other.ptr; }
        bool operator!=(const ConstIterator& other) const { return ptr != other.ptr; }
    };

    ConstIterator begin() const { return ConstIterator(data.data()); }
    ConstIterator end()   const { return ConstIterator(data.data() + data.size()); }

    void append(const T& item) {
        data.push_back(item);
    }

    void prepend(const T& item) {
        data.insert(data.begin(), item);
    }

    T pop() {
        if (UNLIKELY(data.empty())) {
            throw IndexError("Cannot pop from empty array");
        }
        T item = data.back();
        data.pop_back();
        return item;
    }

    void insert(Int index, const T& item) {
        if (UNLIKELY(index.value < 0 || index.value > static_cast<int64_t>(data.size()))) {
            throw IndexError("Array index out of bounds");
        }
        data.insert(data.begin() + index.value, item);
    }

    T remove(Int index) {
        if (UNLIKELY(index.value < 0 || index.value >= static_cast<int64_t>(data.size()))) {
            throw IndexError("Array index out of bounds");
        }
        T item = data[index.value];
        data.erase(data.begin() + index.value);
        return item;
    }

    void clear() {
        data.clear();
    }

    Array<T> slice(Int start, Int end) const {
        if (UNLIKELY(start.value < 0 || end.value > static_cast<int64_t>(data.size()) || start.value > end.value)) {
            throw IndexError("Invalid slice indices");
        }
        return Array<T>(std::vector<T>(data.begin() + start.value, data.begin() + end.value));
    }

    void reserve(Int capacity) {
        if (UNLIKELY(capacity.value < 0)) {
            throw ValueError("Capacity cannot be negative");
        }
        data.reserve(capacity.value);
    }
};

// ===== Set =====
template<Hashable T>
struct Set final {
    static constexpr const char* class_name = "set";
    struct Hasher {
        size_t operator()(const T& value) const {
            return static_cast<size_t>(value.getHash().value);
        }
    };
    struct Equal {
        bool operator()(const T& a, const T& b) const {
            return (a == b).value;
        }
    };
    std::unordered_set<T, Hasher, Equal> data;

    Set() = default;
    Set(const Set<T>&) = default;
    Set(Set<T>&&) = default;
    Set<T>& operator=(const Set<T>&) = default;
    Set<T>& operator=(Set<T>&&) = default;
    ~Set() = default;

    explicit Set(Array<T> arr) {
        for (const T& item : arr.data) {
            data.insert(item);
        }
    }
    String toString() const {
        std::ostringstream oss;
        oss << "{";
        size_t n = 0;
        for (const T& item : data) {
            oss << item.toString();
            if (++n < data.size()) {
                oss << ", ";
            }
        }
        oss << "}";
        return String(oss.str());
    }
    constexpr Int getHash() const {
        size_t hash_value = 0;
        for (const T& item : data) {
            hash_value ^= static_cast<size_t>(item.getHash().value);
        }
        return Int(static_cast<int64_t>(hash_value));
    }

    Int length() const {
        return Int(static_cast<int64_t>(data.size()));
    }

    auto begin() { return data.begin(); }
    auto end()   { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end()   const { return data.end(); }

    void insert(const T& value) {
        data.insert(value);
    }

    Bool remove(const T& value) {
        return Bool(data.erase(value) > 0);
    }

    Bool contains(const T& value) const {
        return Bool(data.find(value) != data.end());
    }

    Bool empty() const {
        return Bool(data.empty());
    }

    void clear() {
        data.clear();
    }
};

// ===== Map =====
template<Hashable K, typename V>
struct Map final {
    static constexpr const char* class_name = "map";
    struct getHasher {
        size_t operator()(const K& key) const {
            return static_cast<size_t>(key.getHash().value);
        }
    };
    struct Equal {
        bool operator()(const K& a, const K& b) const {
            return (a == b).value;
        }
    };

    std::unordered_map<K, V, getHasher, Equal> data;

    Map() = default;
    Map(const Map<K, V>&) = default;
    Map(Map<K, V>&&) = default;
    Map<K, V>& operator=(const Map<K, V>&) = default;
    Map<K, V>& operator=(Map<K, V>&&) = default;
    ~Map() = default;

    String toString() const 
    requires (Printable<K> && Printable<V>) {
        std::ostringstream oss;
        oss << "{";
        size_t n = 0;
        for (const auto& [key, value] : data) {
            oss << key.toString() << ": " << value.toString();
            if (++n < data.size()) {
                oss << ", ";
            }
        }
        oss << "}";
        return String(oss.str());
    }
    constexpr Int getHash() const {
        size_t getHash_value = 0;
        for (const auto& [key, value] : data) {
            getHash_value ^= static_cast<size_t>(key.getHash().value) ^ static_cast<size_t>(value.getHash().value);
        }
        return Int(static_cast<int64_t>(getHash_value));
    }

    void insert(const K& key, const V& value) {
        data[key] = value;
    }
    V& get(const K& key) {
        auto it = data.find(key);
        if (it == data.end()) {
            throw KeyError("Key not found");
        }
        return it->second;
    }
    const V& get(const K& key) const {
        auto it = data.find(key);
        if (it == data.end()) {
            throw KeyError("Key not found");
        }
        return it->second;
    }
    V& operator[](const K& key) {
        return data[key];
    }

    struct Iterator {
        using inner = typename std::unordered_map<K, V, getHasher, Equal>::iterator;
        inner it;

        Iterator(inner i) : it(i) {}

        Pair<const K&, V&> operator*() const {
            return Pair<const K&, V&>(it->first, it->second);
        }

        Iterator& operator++() { ++it; return *this; }
        bool operator==(const Iterator& other) const { return it == other.it; }
        bool operator!=(const Iterator& other) const { return it != other.it; }
    };

    struct ConstIterator {
        using inner = typename std::unordered_map<K, V, getHasher, Equal>::const_iterator;
        inner it;

        ConstIterator(inner i) : it(i) {}

        Pair<const K&, V&> operator*() const {
            return Pair<const K&, V&>(it->first, it->second);
        }

        ConstIterator& operator++() { ++it; return *this; }
        bool operator==(const ConstIterator& other) const { return it == other.it; }
        bool operator!=(const ConstIterator& other) const { return it != other.it; }
    };

    Iterator begin() { return Iterator(data.begin()); }
    Iterator end()   { return Iterator(data.end()); }
    ConstIterator begin() const { return ConstIterator(data.begin()); }
    ConstIterator end()   const { return ConstIterator(data.end()); }

    
    Bool remove(const K& key) {
        return Bool(data.erase(key) > 0);
    }
    Bool contains(const K& key) const {
        return Bool(data.find(key) != data.end());
    }
    Int length() const {
        return Int(data.size());
    }
    Bool empty() const {
        return data.empty();
    }
    void clear() {
        data.clear();
    }

    Array<K> keys() const {
        Array<K> result;
        for (const auto& [key, _] : data) {
            result.data.push_back(key);
        }
        return result;
    }
    Array<V> values() const {
        Array<V> result;
        for (const auto& [_, value] : data) {
            result.data.push_back(value);
        }
        return result;
    }
};

// ===== Union, Optional, Tuple, Lambda =====

// ===== Union =====
template<typename... Ts>
struct Union final {
    static constexpr const char* class_name = "union";
    std::variant<Ts...> data;

    Union(const Union<Ts...>&) = default;
    Union(Union<Ts...>&&) = default;
    Union<Ts...>& operator=(const Union<Ts...>&) = default;
    Union<Ts...>& operator=(Union<Ts...>&&) = default;
    ~Union() = default;

    template<typename T>
    explicit Union(T value) : data(std::move(value)) {}

    String toString() const 
    requires (Printable<Ts> && ...)
    {
        return std::visit([](const auto& value) { return value.toString(); }, data);
    }
    Int getHash() const 
    requires (Hashable<Ts> && ...){
        return std::visit([](const auto& value) { return value.getHash(); }, data);
    }
    
    template<typename T>
    bool is() const {
        return std::holds_alternative<T>(data);
    }

    template<typename T>
    T& get() {
        if (!is<T>()) {
            throw TypeError("Union does not hold the requested type");
        }
        return std::get<T>(data);
    }

    template<typename T>
    const T& get() const {
        if (!is<T>()) {
            throw TypeError("Union does not hold the requested type");
        }
        return std::get<T>(data);
    }
};

// ===== Optional ====
template<typename T>
struct Optional final {
    static constexpr const char* class_name = "optional";
    std::optional<T> data;

    Optional() = default;
    Optional(const Optional<T>&) = default;
    Optional(Optional<T>&&) = default;
    Optional<T>& operator=(const Optional<T>&) = default;
    Optional<T>& operator=(Optional<T>&&) = default;
    ~Optional() = default;

    explicit Optional(const T& value) : data(value) {}

    String toString() const {
        if (data.has_value()) {
            return String("Optional(") + data->toString() + String(")");
        } else {
            return String("Optional(None)");
        }
    }
    constexpr FORCE_INLINE Int getHash() const 
    requires (Hashable<T>) {
        if (data.has_value()) {
            return data->getHash();
        } else {
            return Int(0);  // Arbitrary getHash for None
        }
    }

    Bool hasValue() const { return Bool(data.has_value()); }
    T& value() {
        if (!data.has_value()) {
            throw ValueError("Optional has no value");
        }
        return data.value();
    }
    const T& value() const {
        if (!data.has_value()) {
            throw ValueError("Optional has no value");
        }
        return data.value();
    } 

    void reset() { data.reset(); }
};

// ===== Tuple =====
template <typename... Ts>
struct Tuple {
    static constexpr const char* class_name = "tuple";
    std::tuple<Ts...> data;

    // Constructors
    Tuple(Ts&&... args) : data(std::forward<Ts>(args)...) {}
    Tuple(const Tuple<Ts...>&) = default;
    Tuple(Tuple<Ts...>&&) = default;
    Tuple<Ts...>& operator=(const Tuple<Ts...>&) = default;
    Tuple<Ts...>& operator=(Tuple<Ts...>&&) = default;
    ~Tuple() = default;

    // Unpack function for structured bindings
    std::tuple<Ts...>& unpack() { return data; }
    const std::tuple<Ts...>& unpack() const { return data; }

    // Example: Python-style toString
    String toString() const 
    requires (Printable<Ts> && ...)
    {
        std::ostringstream oss;
        oss << "(";
        std::apply([&oss](const auto&... args){
            size_t n = 0;
            ((oss << args.toString() << (++n < sizeof...(Ts) ? ", " : "")), ...);
        }, data);
        oss << ")";
        return String(oss.str());
    }

    constexpr Int getHash() const 
    requires (Hashable<Ts> && ...)
    {
        size_t getHash_value = 0;
        std::apply([&getHash_value](const auto&... args){
            ((getHash_value = getHash_value * 31 + static_cast<size_t>(args.getHash().value)), ...);
        }, data);
        return Int(static_cast<int64_t>(getHash_value));
    }
};

template <typename... Ts>
Tuple(Ts...) -> Tuple<Ts...>;  // Deduction guide

// ===== Pair =====
template<typename A, typename B>
struct Pair final {
    static constexpr const char* class_name = "pair";
    A first;
    B second;

    Pair() = default;
    Pair(const A& a, const B& b) : first(a), second(b) {}
    Pair(A&& a, B&& b) : first(std::move(a)), second(std::move(b)) {}
    Pair(const Pair&) = default;
    Pair(Pair&&) = default;
    Pair& operator=(const Pair&) = default;
    Pair& operator=(Pair&&) = default;
    ~Pair() = default;

    String toString() const
    requires (Printable<A> && Printable<B>) {
        return String("(") + first.toString() + String(", ") + second.toString() + String(")");
    }

    Int getHash() const
    requires (Hashable<A> && Hashable<B>) {
        size_t h = static_cast<size_t>(first.getHash().value);
        h ^= static_cast<size_t>(second.getHash().value) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return Int(static_cast<int64_t>(h));
    }

    Bool operator==(const Pair<A, B>& other) const {
        return Bool((first == other.first).value && (second == other.second).value);
    }
    Bool operator!=(const Pair<A, B>& other) const {
        return Bool(!operator==(other).value);
    }
};

template<typename A, typename B>
Pair(A, B) -> Pair<A, B>;  // deduction guide

// ===== Lambda =====
template<typename Ret, typename... Args>
struct Lambda final {
    static constexpr const char* class_name = "lambda";
    std::function<Ret(Args...)> func;

    Lambda() = default;
    Lambda(const Lambda&) = default;
    Lambda(Lambda&&) = default;
    Lambda& operator=(const Lambda&) = default;
    Lambda& operator=(Lambda&&) = default;
    ~Lambda() = default;

    template<typename F>
    explicit Lambda(F&& f) : func(std::forward<F>(f)) {}

    Ret operator()(Args... args) const {
        return func(std::forward<Args>(args)...);
    }
};

// ===== Built-in Functions =====

template <Printable... Us>
void writeln(const Us&... args) {
    ((std::cout << args.toString().c_str()), ...);
    std::cout << std::endl;
}

template <Printable... Us>
void write(const Us&... args) {
    ((std::cout << args.toString().c_str()), ...);
}

inline String input(const String& prompt = String("")) {
    std::cout << prompt.c_str();
    std::string line;
    std::getline(std::cin, line);
    return String(line);
}

template<Typed T>
String type() {
    return String(T::__class_name);
}

template<Typed T, Typed Obj>
Bool isinstance(const Obj& obj) {
    return Bool(Obj::__class_name == T::__class_name);
}
