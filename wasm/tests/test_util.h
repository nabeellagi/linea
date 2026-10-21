#pragma once
// =============================================================================
// test_util.h - a tiny test framework (about 40 lines, no downloads needed).
//   TEST(name) { ... }       defines and auto-registers a test
//   CHECK(cond)              records a failure if cond is false
//   CHECK_EQ(a, b)           same, but prints both values
// =============================================================================
#include <iostream>
#include <string>
#include <vector>

inline int g_checks = 0;       // how many CHECKs ran
inline int g_failures = 0;     // how many failed

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++g_checks;                                                              \
        if (!(cond)) {                                                           \
            ++g_failures;                                                        \
            std::cout << "    FAILED " << __FILE__ << ":" << __LINE__ << "  " #cond << "\n"; \
        }                                                                        \
    } while (0)

#define CHECK_EQ(a, b)                                                           \
    do {                                                                         \
        ++g_checks;                                                              \
        auto _va = (a);                                                          \
        auto _vb = (b);                                                          \
        if (!(_va == _vb)) {                                                     \
            ++g_failures;                                                        \
            std::cout << "    FAILED " << __FILE__ << ":" << __LINE__ << "  " #a " == " #b \
                      << "   (got " << _va << ", expected " << _vb << ")\n";     \
        }                                                                        \
    } while (0)

inline bool contains(const std::string& text, const std::string& piece) {
    return text.find(piece) != std::string::npos;
}

// ---- self-registration: every TEST(...) adds itself to this list before main() ----
using TestFn = void (*)();
struct TestCase { const char* name; TestFn fn; };

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> list;                   // created on first use
    return list;
}
struct Registrar {
    Registrar(const char* name, TestFn fn) { registry().push_back({name, fn}); }
};

#define TEST(name)                                       \
    static void name();                                  \
    static Registrar registrar_##name(#name, name);      \
    static void name()