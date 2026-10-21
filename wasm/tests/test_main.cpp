#include "test_util.h"

int main() {
    int failed_tests = 0;
    for (const TestCase& t : registry()) {
        const int before = g_failures;
        t.fn();
        const bool ok = (g_failures == before);
        std::cout << (ok ? "[PASS] " : "[FAIL] ") << t.name << "\n";
        if (!ok) ++failed_tests;
    }
    std::cout << "\n" << registry().size() - failed_tests << "/" << registry().size()
              << " tests passed (" << g_checks << " checks)\n";
    return failed_tests == 0 ? 0 : 1;
}