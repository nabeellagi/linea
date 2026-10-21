#pragma once
// =============================================================================
// solver_check.h - two helpers shared by the Gauss tests and the LU tests.
// They take the solver as a function pointer, so one test body can be reused.
// =============================================================================
#include <string>
#include <vector>
#include "test_util.h"

using SolveFn  = const char* (*)(const char*);   // e.g. solve_gauss
using StatusFn = int (*)();                      // e.g. solve_gauss_status

// The solver must succeed and its LaTeX must contain every string in `expected`.
inline void expect_solution(SolveFn solve, StatusFn status, const std::string& input,
                            const std::vector<std::string>& expected) {
    const std::string out = solve(input.c_str());        // copy at once: the buffer is reused
    CHECK_EQ(status(), 0);
    for (const std::string& e : expected) {
        if (!contains(out, e)) std::cout << "    missing: " << e << "\n";
        CHECK(contains(out, e));
    }
}

// The solver must fail with exactly `expected_status` and give a non-empty message.
inline void expect_error(SolveFn solve, StatusFn status, const std::string& input, int expected_status) {
    const std::string out = solve(input.c_str());
    CHECK_EQ(status(), expected_status);
    if (out.empty()) std::cout << "    error message was empty\n";
    CHECK(!out.empty());
}

// "v1 = 1\nv2 = 2\n..." : a diagonal system with `count` variables.
inline std::string diagonal_system(int count) {
    std::string s;
    for (int i = 1; i <= count; ++i) s += "v" + std::to_string(i) + " = " + std::to_string(i) + "\n";
    return s;
}