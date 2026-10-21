#include <string>

#include "lu/lu_core.h"
#include "solver_check.h"
#include "solvers/solvers.h"
#include "test_util.h"

namespace {
bool same(const Fraction& a, const Fraction& b) { return a.n == b.n && a.d == b.d; }   // fractions are always reduced
}

// ------------------- the factorisation itself (pure math) -------------------

TEST(lu_factors_reconstruct_the_matrix) {
    // The first column starts with 0, so a row swap is unavoidable.
    const long long raw[3][3] = {{0, 2, 1}, {1, 1, 1}, {2, 5, -1}};
    Matrix A(3, std::vector<Fraction>(3));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) A[i][j] = Fraction(raw[i][j]);

    const LUResult f = lu_decompose(A);
    CHECK(f.ok);

    // The defining property:  (L * U)[i][j] must equal A[perm[i]][j]   (that is P*A = L*U)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Fraction sum;                                    // 0
            for (int k = 0; k < 3; ++k) sum = sum + f.L[i][k] * f.U[k][j];
            CHECK(same(sum, A[f.perm[i]][j]));
        }
    }
    // Shape rules: L is lower with 1s on the diagonal, U is upper.
    for (int i = 0; i < 3; ++i) {
        CHECK(same(f.L[i][i], Fraction(1)));
        for (int j = i + 1; j < 3; ++j) CHECK(f.L[i][j].is_zero());   // above L's diagonal
        for (int j = 0; j < i; ++j)     CHECK(f.U[i][j].is_zero());   // below U's diagonal
    }
}

TEST(lu_detects_singular_matrix) {
    Matrix A = {{Fraction(1), Fraction(2)}, {Fraction(2), Fraction(4)}};
    const LUResult f = lu_decompose(A);
    CHECK(!f.ok);
    CHECK_EQ(f.bad_col, 1);
}

// ------------------------------- solve_lu -----------------------------------

TEST(lu_two_by_two) {
    expect_solution(solve_lu, solve_lu_status, "x + 2y = 200\n2x + 3y = 120", {"x = -360", "y = 280"});
}

TEST(lu_shows_its_steps) {
    expect_solution(solve_lu, solve_lu_status, "x + 2y = 200\n2x + 3y = 120",
                    {"Forward substitution", "Back substitution", "l_{2,1} = 2"});
}

TEST(lu_fraction_answer) {
    expect_solution(solve_lu, solve_lu_status, "2x + y = 1\nx + 3y = 1",
                    {"x = \\frac{2}{5}", "y = \\frac{1}{5}"});
}

TEST(lu_three_by_three) {
    expect_solution(solve_lu, solve_lu_status, "x + y + z = 6\n2y + 5z = -4\n2x + 5y - z = 27",
                    {"x = 5", "y = 3", "z = -2"});
}

TEST(lu_needs_row_swap) {
    expect_solution(solve_lu, solve_lu_status, "0x + y = 2\nx + y = 3",
                    {"\\leftrightarrow", "x = 1", "y = 2"});
}

TEST(lu_exactly_100_variables) {
    expect_solution(solve_lu, solve_lu_status, diagonal_system(100), {"\\mathit{v100} = 100"});
}

TEST(lu_and_gauss_agree) {                                   // same answer from two different methods
    const std::string input = "3a + 2b - c = 1\n2a - 2b + 4c = -2\n-a + 0.5b - c = 0";
    const std::string g = solve_gauss(input.c_str());
    CHECK_EQ(solve_gauss_status(), 0);
    const std::string l = solve_lu(input.c_str());
    CHECK_EQ(solve_lu_status(), 0);
    // everything after the "Solution" heading is the answer list: it must be identical
    const std::string g_ans = g.substr(g.rfind("Solution"));
    const std::string l_ans = l.substr(l.rfind("Solution"));
    CHECK(g_ans == l_ans);
}

// ------------------------------- errors -------------------------------------

TEST(lu_shares_the_parser_errors) {
    expect_error(solve_lu, solve_lu_status, "", 1);
    expect_error(solve_lu, solve_lu_status, "x + 2y 200\ny = 1", 2);
    expect_error(solve_lu, solve_lu_status, "x + 2..5y = 3\ny = 1", 3);
    expect_error(solve_lu, solve_lu_status, "x + 2x = 5", 4);
    expect_error(solve_lu, solve_lu_status, diagonal_system(101), 5);
    expect_error(solve_lu, solve_lu_status, "x + y = 3", 6);
}

TEST(lu_singular_and_overflow) {
    expect_error(solve_lu, solve_lu_status, "x + y = 2\n2x + 2y = 4", 7);
    expect_error(solve_lu, solve_lu_status, "99999999999999x + y = 1\nx + 99999999999999y = 1", 8);
}

TEST(lu_null_input) {
    const std::string msg = solve_lu(nullptr);
    CHECK_EQ(solve_lu_status(), 9);
    CHECK(!msg.empty());
}