#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <exception>

namespace espresso_compiler {

// ============================================================================
// INTERNAL COMPILER EXCEPTION
// (Use only for internal bugs, not user errors)
// ============================================================================

class CompilerException : public std::exception {
public:
    std::string message_;
    std::string filepath;
    int line;
    int column;

    explicit CompilerException(std::string message, int l, int c, std::string fn = "")
        : message_(std::move(message)), line(l), column(c), filepath(std::move(fn)) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }
};

// ============================================================================
// ERROR SEVERITY
// ============================================================================

enum class Severity {
    Note,
    Warning,
    Error,
    Fatal
};

inline const char* severity_to_string(Severity s) {
    switch (s) {
        case Severity::Note:    return "note";
        case Severity::Warning: return "warning";
        case Severity::Error:   return "error";
        case Severity::Fatal:   return "fatal error";
    }
    return "unknown";
}

inline const char* severity_color(Severity s) {
    switch (s) {
        case Severity::Note:    return "\033[36m";   // cyan
        case Severity::Warning: return "\033[33m";   // yellow
        case Severity::Error:   return "\033[31m";   // red
        case Severity::Fatal:   return "\033[1;31m"; // bold red
    }
    return "";
}

inline bool is_error(Severity s) {
    return s == Severity::Error || s == Severity::Fatal;
}

// ============================================================================
// DIAGNOSTIC
// ============================================================================

struct Diagnostic {
    Severity severity;
    std::string message;
    std::string file;
    int line = -1;
    int column = -1;
    std::string source_line;
    std::string hint;

    std::string format(bool use_color = true) const {
        std::string out;

        // Location
        if (!file.empty())
            out += file + ":";

        if (line != -1)
            out += std::to_string(line) + ":";

        if (column != -1)
            out += std::to_string(column) + ":";

        if (!out.empty())
            out += " ";

        // Severity
        if (use_color)
            out += severity_color(severity);

        out += severity_to_string(severity);

        if (use_color)
            out += "\033[0m";

        out += ": ";

        // Message
        if (use_color)
            out += "\033[1m";

        out += message;

        if (use_color)
            out += "\033[0m";

        out += "\n";

        // Source context
        if (!source_line.empty() && column > 0) {
            out += source_line + "\n";
            out += std::string(column - 1, ' ') + "^\n";
        }

        // Hint
        if (!hint.empty()) {
            if (use_color)
                out += "\033[36m";

            out += "hint: " + hint + "\n";

            if (use_color)
                out += "\033[0m";
        }

        return out;
    }
};

// ============================================================================
// DIAGNOSTIC COLLECTOR
// ============================================================================

class DiagnosticCollector {
    std::vector<Diagnostic> diagnostics_;
    bool has_errors_ = false;

public:
    void add(Diagnostic d) {
        if (is_error(d.severity))
            has_errors_ = true;

        diagnostics_.push_back(std::move(d));
    }

    void note(std::string msg,
              std::string file = "",
              int line = -1,
              int col = -1) {
        add({Severity::Note, std::move(msg), std::move(file), line, col});
    }

    void warning(std::string msg,
                 std::string file = "",
                 int line = -1,
                 int col = -1) {
        add({Severity::Warning, std::move(msg), std::move(file), line, col});
    }

    void error(std::string msg,
               std::string file = "",
               int line = -1,
               int col = -1) {
        add({Severity::Error, std::move(msg), std::move(file), line, col});
    }

    void fatal(std::string msg,
               std::string file = "",
               int line = -1,
               int col = -1) {
        add({Severity::Fatal, std::move(msg), std::move(file), line, col});
    }

    void add_source_context(const std::string& src) {
        if (!diagnostics_.empty())
            diagnostics_.back().source_line = src;
    }

    void add_hint(const std::string& hint) {
        if (!diagnostics_.empty())
            diagnostics_.back().hint = hint;
    }

    bool has_errors() const { return has_errors_; }

    size_t error_count() const {
        size_t count = 0;
        for (const auto& d : diagnostics_)
            if (is_error(d.severity))
                ++count;
        return count;
    }

    size_t warning_count() const {
        size_t count = 0;
        for (const auto& d : diagnostics_)
            if (d.severity == Severity::Warning)
                ++count;
        return count;
    }

    void print_all(bool use_color = true) const {
        for (const auto& d : diagnostics_)
            std::cerr << d.format(use_color);
    }

    void print_summary(bool use_color = true) const {
        size_t errors = error_count();
        size_t warnings = warning_count();

        if (errors == 0 && warnings == 0) {
            if (use_color) std::cerr << "\033[32m";
            std::cerr << "Compilation successful\n";
            if (use_color) std::cerr << "\033[0m";
            return;
        }

        if (errors > 0) {
            if (use_color) std::cerr << "\033[31m";
            std::cerr << errors << " error"
                      << (errors != 1 ? "s" : "");
            if (use_color) std::cerr << "\033[0m";
        }

        if (warnings > 0) {
            if (errors > 0) std::cerr << ", ";
            if (use_color) std::cerr << "\033[33m";
            std::cerr << warnings << " warning"
                      << (warnings != 1 ? "s" : "");
            if (use_color) std::cerr << "\033[0m";
        }

        std::cerr << " generated\n";
    }

    void clear() {
        diagnostics_.clear();
        has_errors_ = false;
    }

    const std::vector<Diagnostic>& diagnostics() const {
        return diagnostics_;
    }
};

// ============================================================================
// GLOBAL DIAGNOSTICS (simple v1 approach)
// ============================================================================

inline thread_local DiagnosticCollector g_diagnostics;

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

inline void note(const std::string& message, const std::string& file = "",
                 int line = -1, int col = -1) {
    g_diagnostics.note(message, file, line, col);
}

inline void warn(const std::string& message, const std::string& file = "",
                 int line = -1, int col = -1) {
    g_diagnostics.warning(message, file, line, col);
}

inline void error(const std::string& message, const std::string& file = "",
                  int line = -1, int col = -1) {
    g_diagnostics.error(message, file, line, col);
}

inline void fatal(const std::string& message, const std::string& file = "",
                  int line = -1, int col = -1) {
    g_diagnostics.fatal(message, file, line, col);
}

} // namespace espresso_compiler