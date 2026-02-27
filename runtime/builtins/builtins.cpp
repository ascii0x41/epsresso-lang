#include "builtins.h"
#include <format>

// ===== Int =====

// ========== Type Conversions ==========
Int Int::toInt() const { return *this; }
Float Int::toFloat() const { return Float(static_cast<double>(value)); }
Complex Int::toComplex() const { return Complex(static_cast<double>(value), 0); }
Bool Int::toBool() const { return Bool(value != 0); }
String Int::toString() const { return String(std::to_string(value)); }

// ========== Arithmetic Operators ==========
// With Int
Int Int::operator+(const Int& other) const { return Int(value + other.value); }
Int Int::operator-(const Int& other) const { return Int(value - other.value); }
Int Int::operator*(const Int& other) const { return Int(value * other.value); }
Int Int::operator/(const Int& other) const {
    if (UNLIKELY(other.value == 0)) {
        throw ZeroDivisionError("division by zero");
    }
    return Int(value / other.value);
}
Int Int::operator%(const Int& other) const {
    if (UNLIKELY(other.value == 0)) {
        throw ZeroDivisionError("modulo by zero");
    }
    return Int(value % other.value);
}

// With Float
Float Int::operator+(const Float& other) const { return Float(static_cast<double>(value) + other.value); }
Float Int::operator-(const Float& other) const { return Float(static_cast<double>(value) - other.value); }
Float Int::operator*(const Float& other) const { return Float(static_cast<double>(value) * other.value); }
Float Int::operator/(const Float& other) const {
    if (UNLIKELY(other.value == 0.0)) {
        throw ZeroDivisionError("division by zero");
    }
    return Float(value / other.value);
}

// With Complex
Complex Int::operator+(const Complex& other) const { return Complex(static_cast<double>(value) + other.re, other.im); }
Complex Int::operator-(const Complex& other) const { return Complex(static_cast<double>(value) - other.re, -other.im); }
Complex Int::operator*(const Complex& other) const { return Complex(static_cast<double>(value) * other.re, static_cast<double>(value)* other.im); }
Complex Int::operator/(const Complex& other) const {
    double denom = other.re * other.re + other.im * other.im;

    if (UNLIKELY(denom == 0.0)) {
        throw ZeroDivisionError("division by zero");
    }

    return Complex((static_cast<double>(value) * other.re) / denom, -(static_cast<double>(value) * other.im) / denom);
}

// ========== Assignment Operators ==========
Int& Int::operator+=(const Int& other) { value += other.value; return *this; }
Int& Int::operator-=(const Int& other) { value -= other.value; return *this; }
Int& Int::operator*=(const Int& other) { value *= other.value; return *this; }
// Division and modulo (need overflow checks!)
Int& Int::operator/=(const Int& other) {
    if (UNLIKELY(other.value == 0)) {
        throw ZeroDivisionError("division by zero");
    }
    value /= other.value;  // Be careful with integer division
    return *this;
}
Int& Int::operator%=(const Int& other) {
    if (UNLIKELY(other.value == 0)) {
        throw ZeroDivisionError("modulo by zero");
    }
    value %= other.value;
    return *this;
}

// ========== Unary Operators ==========
Int Int::operator+() const { return Int(+value); }
Int Int::operator-() const { return Int(-value); }

// ========== Comparison Operators ==========
// With Int
Bool Int::operator==(const Int& other) const { return Bool(value == other.value); }
Bool Int::operator!=(const Int& other) const { return Bool(value != other.value); }
Bool Int::operator<(const Int& other) const { return Bool(value < other.value); }
Bool Int::operator>(const Int& other) const { return Bool(value > other.value); }
Bool Int::operator<=(const Int& other) const { return Bool(value <= other.value); }
Bool Int::operator>=(const Int& other) const { return Bool(value >= other.value); }

// With Float
Bool Int::operator==(const Float& other) const { return Bool(static_cast<double>(value) == other.value); }
Bool Int::operator!=(const Float& other) const { return Bool(static_cast<double>(value) != other.value); }
Bool Int::operator<(const Float& other) const { return Bool(static_cast<double>(value) < other.value); }
Bool Int::operator>(const Float& other) const { return Bool(static_cast<double>(value) > other.value); }
Bool Int::operator<=(const Float& other) const { return Bool(static_cast<double>(value) <= other.value); }
Bool Int::operator>=(const Float& other) const { return Bool(static_cast<double>(value) >= other.value); }

// With Complex
Bool Int::operator==(const Complex& other) const { return Bool(static_cast<double>(value * value) == (other.re * other.re + other.im * other.im)); }
Bool Int::operator!=(const Complex& other) const { return Bool(static_cast<double>(value * value) != (other.re * other.re + other.im * other.im)); }
Bool Int::operator<(const Complex& other) const { return Bool(static_cast<double>(value * value) < (other.re * other.re + other.im * other.im)); }
Bool Int::operator>(const Complex& other) const { return Bool(static_cast<double>(value * value) > (other.re * other.re + other.im * other.im)); }
Bool Int::operator<=(const Complex& other) const { return Bool(static_cast<double>(value * value) <= (other.re * other.re + other.im * other.im)); }
Bool Int::operator>=(const Complex& other) const { return Bool(static_cast<double>(value * value) >= (other.re * other.re + other.im * other.im)); }


// ===== FLoat =====

// ========== Type Conversions ==========
Int Float::toInt() const { return Int(static_cast<int64_t>(value)); }
Float Float::toFloat() const { return *this; }
Complex Float::toComplex() const { return Complex(value, 0); }
Bool Float::toBool() const { return Bool(value != 0.0); }
String Float::toString() const { return String(std::to_string(value)); }
// FORCE_INLINE Int Float::getHash() - Now inline in header

// ========== Arithmetic Operators ==========
// With Int
Float Float::operator+(const Int& other) const { return Float(value + static_cast<double>(other.value)); }
Float Float::operator-(const Int& other) const { return Float(value - static_cast<double>(other.value)); }
Float Float::operator*(const Int& other) const { return Float(value * static_cast<double>(other.value)); }
Float Float::operator/(const Int& other) const { return Float(value / static_cast<double>(other.value)); }

// With Float
Float Float::operator+(const Float& other) const { return Float(value + other.value); }
Float Float::operator-(const Float& other) const { return Float(value - other.value); }
Float Float::operator*(const Float& other) const { return Float(value * other.value); }
Float Float::operator/(const Float& other) const { return Float(value / other.value); }

// With Complex
Complex Float::operator+(const Complex& other) const { return Complex(value + other.re, other.im); }
Complex Float::operator-(const Complex& other) const { return Complex(value - other.re, -other.im); }
Complex Float::operator*(const Complex& other) const { return Complex(value * other.re, value * other.im); }
Complex Float::operator/(const Complex& other) const {
    double denom = other.re * other.re + other.im * other.im;

    if (UNLIKELY(denom == 0.0)) {
        throw ZeroDivisionError("division by zero");
    }

    return Complex(value * other.re / denom, -value * other.im / denom);
}


// ========== Assignment Operators ==========
Float& Float::operator+=(const Float& other) { value += other.value; return *this; }
Float& Float::operator-=(const Float& other) { value -= other.value; return *this; }
Float& Float::operator*=(const Float& other) { value *= other.value; return *this; }
Float& Float::operator/=(const Float& other) { value /= other.value; return *this; }

// ========== Unary Operators ==========
Float Float::operator+() const { return Float(+value); }
Float Float::operator-() const { return Float(-value); }

// ========== Comparison Operators ==========
// With Int
Bool Float::operator==(const Int& other) const { return Bool(value == static_cast<double>(other.value)); }
Bool Float::operator!=(const Int& other) const { return Bool(value != static_cast<double>(other.value)); }
Bool Float::operator<(const Int& other) const { return Bool(value < static_cast<double>(other.value)); }
Bool Float::operator>(const Int& other) const { return Bool(value > static_cast<double>(other.value)); }
Bool Float::operator<=(const Int& other) const { return Bool(value <= static_cast<double>(other.value)); }
Bool Float::operator>=(const Int& other) const { return Bool(value >= static_cast<double>(other.value)); }

// With Float
Bool Float::operator==(const Float& other) const { return Bool(value == other.value); }
Bool Float::operator!=(const Float& other) const { return Bool(value != other.value); }
Bool Float::operator<(const Float& other) const { return Bool(value < other.value); }
Bool Float::operator>(const Float& other) const { return Bool(value > other.value); }
Bool Float::operator<=(const Float& other) const { return Bool(value <= other.value); }
Bool Float::operator>=(const Float& other) const { return Bool(value >= other.value); }

// With Complex
Bool Float::operator==(const Complex& other) const { return Bool((value * value == other.re * other.re + other.im * other.im)); }
Bool Float::operator!=(const Complex& other) const { return Bool((value * value != other.re * other.re + other.im * other.im)); }
Bool Float::operator<(const Complex& other) const { return Bool((value * value < other.re * other.re + other.im * other.im)); }
Bool Float::operator>(const Complex& other) const { return Bool((value * value > other.re * other.re + other.im * other.im)); }
Bool Float::operator<=(const Complex& other) const { return Bool((value * value <= other.re * other.re + other.im * other.im)); }
Bool Float::operator>=(const Complex& other) const { return Bool((value * value >= other.re * other.re + other.im * other.im)); }


// ===== Complex =====

// ========== Type Conversions ==========
Int Complex::toInt() const { return Int(static_cast<int64_t>(re)); }
Float Complex::toFloat() const { return Float(re); }
Complex Complex::toComplex() const { return *this; }
Bool Complex::toBool() const { return Bool(re != 0.0 && im != 0.0); }
String Complex::toString() const {
    if (im == 0) return String(std::to_string(re));
    if (re == 0) return String(std::to_string(im) + "i");
    return String(std::to_string(re) + (im > 0 ? " + " : " - ") + std::to_string(std::abs(im)) + "i");
}
// FORCE_INLINE Int Complex::getHash() - Now inline in header

// ========== Arithmetic Operators ==========
// With Int
Complex Complex::operator+(const Int& other) const { return Complex(re + static_cast<double>(other.value), im); }
Complex Complex::operator-(const Int& other) const { return Complex(re - static_cast<double>(other.value), im); }
Complex Complex::operator*(const Int& other) const { return Complex(re * static_cast<double>(other.value), im * static_cast<double>(other.value)); }
Complex Complex::operator/(const Int& other) const { return Complex(re / static_cast<double>(other.value), im / static_cast<double>(other.value)); }
    

// With Float
Complex Complex::operator+(const Float& other) const { return Complex(re + other.value, im); }
Complex Complex::operator-(const Float& other) const { return Complex(re - other.value, im); }
Complex Complex::operator*(const Float& other) const { return Complex(re * other.value, im * static_cast<double>(other.value)); }
Complex Complex::operator/(const Float& other) const { return Complex(re / other.value, im / other.value); }
    

// With Complex
Complex Complex::operator+(const Complex& other) const { return Complex(re + other.re, im + other.im); }
Complex Complex::operator-(const Complex& other) const { return Complex(re - other.re, im - other.im); }
Complex Complex::operator*(const Complex& other) const { return Complex(re * other.re - im * other.im, re * other.im + im * other.re); }
Complex Complex::operator/(const Complex& other) const {
    double denom = other.re * other.re + other.im * other.im;
    return Complex((re * other.re + im * other.im) / denom, (im * other.re - re * other.im) / denom);
}


// ========== Assignment Operators ==========
Complex& Complex::operator+=(const Complex& other) { re += other.re; im += other.im; return *this; }
Complex& Complex::operator-=(const Complex& other) { re -= other.re; im -= other.im; return *this; }
Complex& Complex::operator*=(const Complex& other) { 
    auto old_re = re; auto old_im = im;
    re = old_re * other.re - old_im * other.im; 
    im = old_re * other.im + old_im * other.re; 
    return *this;
}
Complex& Complex::operator/=(const Complex& other) {
    auto old_re = re; auto old_im = im;
    auto denom = other.re * other.re + other.im * other.im;
    re = (old_re * other.re + old_im * other.im) / denom;
    im = (old_im * other.re - old_re * other.im) / denom;
    return *this;
}


// ========== Unary Operators ==========
Complex Complex::operator+() const { return Complex(+re, +im); }
Complex Complex::operator-() const { return Complex(-re, -im); }


// ========== Comparison Operators ==========
// With Int
Bool Complex::operator==(const Int& other) const { return Bool((re * re + im * im) == static_cast<double>(other.value * other.value)); }
Bool Complex::operator!=(const Int& other) const { return Bool((re * re + im * im) != static_cast<double>(other.value * other.value)); }
Bool Complex::operator<(const Int& other) const { return Bool((re * re + im * im) < static_cast<double>(other.value * other.value)); }
Bool Complex::operator>(const Int& other) const { return Bool((re * re + im * im) > static_cast<double>(other.value * other.value)); }
Bool Complex::operator<=(const Int& other) const { return Bool((re * re + im * im) <= static_cast<double>(other.value * other.value)); }
Bool Complex::operator>=(const Int& other) const { return Bool((re * re + im * im) >= static_cast<double>(other.value * other.value)); }


// With Float
Bool Complex::operator==(const Float& other) const { return Bool((re * re + im * im) == other.value * other.value); }
Bool Complex::operator!=(const Float& other) const { return Bool((re * re + im * im) != other.value * other.value); }
Bool Complex::operator<(const Float& other) const { return Bool((re * re + im * im) < other.value * other.value); }
Bool Complex::operator>(const Float& other) const { return Bool((re * re + im * im) > other.value * other.value); }
Bool Complex::operator<=(const Float& other) const { return Bool((re * re + im * im) <= other.value * other.value); }
Bool Complex::operator>=(const Float& other) const { return Bool((re * re + im * im) >= other.value * other.value); }


// With Complex
Bool Complex::operator==(const Complex& other) const { return Bool((re * re + im * im) == (other.re * other.re + other.im * other.im)); }
Bool Complex::operator!=(const Complex& other) const { return Bool((re * re + im * im) != (other.re * other.re + other.im * other.im)); }
Bool Complex::operator<(const Complex& other) const { return Bool((re * re + im * im) < (other.re * other.re + other.im * other.im)); }
Bool Complex::operator>(const Complex& other) const { return Bool((re * re + im * im) > (other.re * other.re + other.im * other.im)); }
Bool Complex::operator<=(const Complex& other) const { return Bool((re * re + im * im) <= (other.re * other.re + other.im * other.im)); }
Bool Complex::operator>=(const Complex& other) const { return Bool((re * re + im * im) >= (other.re * other.re + other.im * other.im)); }


// ========== Mathematical Operations ==========
Float Complex::argument() const { return Float(std::atan2(im, re)); }
Complex Complex::conjugate() const { return Complex(re, -im); }
Float Complex::magnitude() const { return Float(std::sqrt(re * re + im * im)); }


// ===== Bool =====

Int Bool::toInt() const { return Int(value ? 1 : 0); }
Float Bool::toFloat() const { return Float(value ? 1.0 : 0.0);}
Complex Bool::toComplex() const { return Complex(value ? 1.0 : 0.0, 0.0); }
Bool Bool::toBool() const { return *this; }
String Bool::toString() const { return String(value ? "true" : "false"); }
// FORCE_INLINE Int Bool::getHash() - Now inline in header

constexpr Bool Bool::operator==(const Bool& other) const { return Bool(value == other.value); }
constexpr Bool Bool::operator!=(const Bool& other) const { return Bool(value != other.value); }


// ===== String =====

String::String(std::string s) : indexed(false) {
    bytes.assign(s.begin(), s.end());
    bytes.push_back('\0');
}

Int String::toInt() const {
    try {
        int value = std::stoi(c_str());
        return Int(value);
    } catch (const std::invalid_argument&) {
        throw ValueError(std::string("Invalid integer string: ") + c_str());
    } catch (const std::out_of_range&) {
        throw ValueError(std::string("Integer out of range: ") + c_str());
    }
}
Float String::toFloat() const {
    try {
        double value = std::stod(c_str());
        return Float(value);
    } catch (const std::invalid_argument&) {
        throw ValueError(std::string("Invalid float string: ") + c_str());
    } catch (const std::out_of_range&) {
        throw ValueError(std::string("Float out of range: ") + c_str());
    }
}
String String::toString() const { return *this; }
Bool String::toBool() const { return Bool(!bytes.empty() && !(bytes.size() == 1 && bytes[0] == '\0')); }
// FORCE_INLINE Int String::getHash() - Now inline in header

const char* String::c_str() const { return bytes.data(); }

Int String::length() const { 
    build_index();
    return Int(char_starts.size() - 1);  // char_starts includes the start of the null terminator
}
String String::operator[](const Int& index) const {
    build_index();
    if (UNLIKELY(index.value < 0 || index.value >= static_cast<int64_t>(char_starts.size()))) {
        throw IndexError("String index out of bounds");
    }
    
    size_t start = char_starts[index.value];
    size_t end = (index.value + 1 < static_cast<int64_t>(char_starts.size())) 
                    ? char_starts[index.value + 1] 
                    : bytes.size() - 1;
    
    String result("");
    result.bytes.assign(bytes.begin() + start, bytes.begin() + end);
    result.bytes.push_back('\0');
    result.indexed = false;
    return result;
}

String String::operator+(const String& other) const {
    std::string combined = c_str() + std::string(other.c_str());
    return String(combined);
}
String& String::operator+=(const String& other) {
    bytes.insert(bytes.end() - 1, other.c_str(), other.c_str() + other.length().value);
    return *this;
}
String String::operator*(const Int& times) const {
    if (times.value < 0) {
        throw ValueError("Cannot multiply string by negative integer");
    }
    std::string result;
    result.reserve(bytes.size() * times.value);
    for (int i = 0; i < times.value; ++i) {
        result += c_str();
    }
    return String(result);
}
String& String::operator*=(const Int& times) {
    if (times.value < 0) {
        throw ValueError("Cannot multiply string by negative integer");
    }
    std::string result;
    result.reserve(bytes.size() * times.value);
    for (int i = 0; i < times.value; ++i) {
        result += c_str();
    }
    bytes.assign(result.begin(), result.end());
    bytes.push_back('\0');
    return *this;
}

Bool String::operator==(const String& other) const { return Bool(std::strcmp(c_str(), other.c_str()) == 0); }
Bool String::operator!=(const String& other) const { return Bool(std::strcmp(c_str(), other.c_str()) != 0); }

Bool String::isUpper() const {
    if (bytes.empty() || (bytes.size() == 1 && bytes[0] == '\0')) {
        return Bool(false);  // Empty string is not considered uppercase
    }
    for (size_t i = 0; i < bytes.size() - 1; ++i) {
        if (std::isalpha(static_cast<unsigned char>(bytes[i])) && !std::isupper(static_cast<unsigned char>(bytes[i]))) {
            return Bool(false);
        }
    }
    return Bool(true);
}
Bool String::isLower() const {
    if (bytes.empty() || (bytes.size() == 1 && bytes[0] == '\0')) {
        return Bool(false);  // Empty string is not considered lowercase
    }
    for (size_t i = 0; i < bytes.size() - 1; ++i) {
        if (std::isalpha(static_cast<unsigned char>(bytes[i])) && !std::islower(static_cast<unsigned char>(bytes[i]))) {
            return Bool(false);
        }
    }
    return Bool(true);
}
Bool String::isAlpha() const {
    if (bytes.empty() || (bytes.size() == 1 && bytes[0] == '\0')) {
        return Bool(false);  // Empty string is not considered alphabetic
    }
    for (size_t i = 0; i < bytes.size() - 1; ++i) {
        if (!std::isalpha(static_cast<unsigned char>(bytes[i]))) {
            return Bool(false);
        }
    }
    return Bool(true);
}
Bool String::isDigit() const {
    if (bytes.empty() || (bytes.size() == 1 && bytes[0] == '\0')) {
        return Bool(false);  // Empty string is not considered digit
    }
    for (size_t i = 0; i < bytes.size() - 1; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(bytes[i]))) {
            return Bool(false);
        }
    }
    return Bool(!bytes.empty() && bytes.size() > 1);  // At least one character and not just null terminator
}
Bool String::isAlnum() const {
    if (bytes.empty() || (bytes.size() == 1 && bytes[0] == '\0')) {
        return Bool(false);  // Empty string is not considered alphanumeric
    }
    for (size_t i = 0; i < bytes.size() - 1; ++i) {
        if (!std::isalnum(static_cast<unsigned char>(bytes[i]))) {
            return Bool(false);
        }
    }
    return Bool(true);
}
Bool String::isWhitespace() const {
    if (bytes.empty() || (bytes.size() == 1 && bytes[0] == '\0')) {
        return Bool(false);  // Empty string is not considered whitespace
    }
    for (size_t i = 0; i < bytes.size() - 1; ++i) {
        if (!std::isspace(static_cast<unsigned char>(bytes[i]))) {
            return Bool(false);
        }
    }
    return Bool(true);
}


String String::toUpper() const {
    std::string result = c_str();
    for (char& c : result) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return String(result);
}
String String::toLower() const {
    std::string result = c_str();
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return String(result);
}

Bool String::contains(const String& substring) const {
    return Bool(std::string(c_str()).find(substring.c_str()) != std::string::npos);
}

Bool String::startsWith(const String& prefix) const {
    return Bool(std::string(c_str()).rfind(prefix.c_str(), 0) == 0);
}
Bool String::endsWith(const String& suffix) const {
    std::string str(c_str());
    std::string suf(suffix.c_str());
    if (suf.size() > str.size()) {
        return Bool(false);
    }
    return Bool(str.compare(str.size() - suf.size(), suf.size(), suf) == 0);
}
Int String::find(const String& substring) const {
    build_index();
    size_t pos = std::string(c_str()).find(substring.c_str());
    if (pos == std::string::npos) {
        return Int(-1);
    }
    return Int(static_cast<int>(pos));
}
Int String::rfind(const String& substring) const {
    build_index();
    size_t pos = std::string(c_str()).rfind(substring.c_str());
    if (pos == std::string::npos) {
        return Int(-1);
    }
    return Int(static_cast<int>(pos));
}

String String::strip() const {
    std::string str(c_str());
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return String("");
    }
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return String(str.substr(start, end - start + 1));
}
String String::lstrip() const {
    std::string str(c_str());
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return String("");
    }
    return String(str.substr(start));
}
String String::rstrip() const {
    std::string str(c_str());
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    if (end == std::string::npos) {
        return String("");
    }
    return String(str.substr(0, end + 1));
}
String String::replace(const String& old_substring, const String& new_substring) const {
    std::string result = c_str();
    size_t pos = 0;
    while ((pos = result.find(old_substring.c_str(), pos)) != std::string::npos) {
        result.replace(pos, old_substring.length().value, new_substring.c_str());
        pos += new_substring.length().value;
    }
    return String(result);
}

template<Printable T>
String String::join(const Array<T>& elements) const {
    std::string result;
    for (size_t i = 0; i < elements.length(); ++i) {
        if (i > 0) {
            result += c_str();
        }
        result += elements[i].toString().c_str();
    }
    return String(result);
}
String String::substring(const Int& start, const Int& length) const {
    build_index();
    int s = start.value;
    int len = length.value;
    if (s < 0 || len < 0 || s + len > static_cast<int>(bytes.size() - 1)) {
        throw ValueError("Invalid start or length for substring");
    }
    return String(std::string(c_str() + s, len));
}

void String::clear() {
    bytes.clear();
    bytes.push_back('\0');
}
void String::reserve(Int capacity) {
    if (capacity.value < 0) {
        throw ValueError("Capacity cannot be negative");
    }
    bytes.reserve(capacity.value + 1);  // +1 for null terminator
}

// ===== RuntimeError Implementations =====
RuntimeError::RuntimeError(const String& s) : msg(s.c_str()) {}

String RuntimeError::getMessage() const {
    return String(msg);
}

