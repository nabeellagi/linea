#include <initializer_list>
#include <string>
#include <vector>
 
#include "determinants/determinants.h"
#include "test_util.h"
 
namespace {
 
bool same(const Fraction& a, const Fraction& b) {
    return a.n == b.n && a.d == b.d;
}
 
Matrix integer_matrix(std::initializer_list<std::initializer_list<long long>> rows) {
    Matrix result;
    for (const auto& source_row : rows) {
        std::vector<Fraction> row;
        for (long long value : source_row) row.push_back(Fraction(value));
        result.push_back(row);
    }
    return result;
}
 
std::string diagonal_matrix_input(int n) {
    std::string input;
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (c > 0) input += ' ';
            input += (r == c) ? "1" : "0";
        }
        if (r + 1 < n) input += '\n';
    }
    return input;
}
 
void expect_error(const std::string& input, int expected_status) {
    const std::string output = det(input.c_str());
    CHECK_EQ(det_status(), expected_status);
    CHECK(!output.empty());
}
 
}  // namespace
 
// --------------------- LUP determinant: the pure algorithm ------------------
 
TEST(det_lu_counts_row_swaps_and_sign) {
    // This matrix needs one row swap. Its determinant is 9.
    const Matrix A = integer_matrix({{0, 2, 1}, {1, 1, 1}, {2, 5, -1}});
    const DetLUResult result = det_lu(A);
 
    CHECK(result.valid);
    CHECK(!result.singular);
    CHECK_EQ(result.row_swaps, 1);
    CHECK(same(result.value, Fraction(9)));
    CHECK(result.lu.ok);
}
 
TEST(det_lu_returns_zero_for_singular_matrix) {
    const Matrix A = integer_matrix({{1, 2}, {2, 4}});
    const DetLUResult result = det_lu(A);
 
    CHECK(result.valid);
    CHECK(result.singular);
    CHECK(same(result.value, Fraction(0)));
    CHECK_EQ(result.bad_col, 1);
}
 
TEST(det_mc_handles_one_two_and_three_dimensions) {
    const Matrix one = integer_matrix({{-7}});
    const Matrix two = integer_matrix({{1, 2}, {3, 4}});
    const Matrix three = integer_matrix({{0, 2, 1}, {1, 1, 1}, {2, 5, -1}});
 
    CHECK(same(det_mc(one), Fraction(-7)));
    CHECK(same(det_mc(two), Fraction(-2)));
    CHECK(same(det_mc(three), Fraction(9)));
}
 
TEST(det_lu_and_mc_agree_for_small_matrix) {
    const Matrix A = integer_matrix({{2, -1, 3}, {1, 0, 4}, {-2, 5, 1}});
    const DetLUResult lu = det_lu(A);
    const Fraction mc = det_mc(A);
 
    CHECK(lu.valid);
    CHECK(!lu.singular);
    CHECK(same(lu.value, mc));
}
 
// -------------------------- Exported det() API ------------------------------
 
TEST(det_two_by_two_returns_latex) {
    const std::string output = det("1 2\n3 4");
 
    CHECK_EQ(det_status(), DET_OK);
    CHECK(contains(output, "\\begin{aligned}"));
    CHECK(contains(output, "\\det(A)"));
    CHECK(contains(output, "-2"));
    CHECK(contains(output, "\\boxed"));
    CHECK(contains(output, "Minor and cofactor expansion"));
}
 
TEST(det_three_by_three_shows_lup_and_cofactor_steps) {
    const std::string output = det("0 2 1\n1 1 1\n2 5 -1");
 
    CHECK_EQ(det_status(), DET_OK);
    CHECK(contains(output, "P A = L U"));
    CHECK(contains(output, "\\leftrightarrow"));
    CHECK(contains(output, "\\frac{3}{2}"));
    CHECK(contains(output, "a_{11}(a_{22}a_{33}-a_{23}a_{32})"));
    CHECK(contains(output, "\\boxed{\\det(A)=9}"));
}
 
TEST(det_decimal_input_stays_exact) {
    const std::string output = det("0.5 1\n2 3");
 
    CHECK_EQ(det_status(), DET_OK);
    CHECK(contains(output, "\\frac{1}{2}"));
    CHECK(contains(output, "-\\frac{1}{2}"));
}
 
TEST(det_skips_minor_cofactor_expansion_above_three) {
    const std::string input =
        "1 0 0 0\n"
        "0 2 0 0\n"
        "0 0 3 0\n"
        "0 0 0 4";
    const std::string output = det(input.c_str());
 
    CHECK_EQ(det_status(), DET_OK);
    CHECK(contains(output, "N > 3"));
    CHECK(contains(output, "\\boxed{\\det(A)=24}"));
    CHECK(!contains(output, "a_{11}(a_{22}a_{33}-a_{23}a_{32})"));
}
 
TEST(det_singular_matrix_is_a_successful_zero_determinant) {
    const std::string output = det("1 2\n2 4");
 
    CHECK_EQ(det_status(), DET_OK);
    CHECK(contains(output, "matrix is singular"));
    CHECK(contains(output, "\\boxed{\\det(A)=0}"));
    CHECK(contains(output, "The LUP and minor/cofactor results agree."));
}
 
// -------------------------------- Input errors ------------------------------
 
TEST(det_rejects_empty_input) {
    expect_error("\n  \n", DET_EMPTY);
}
 
TEST(det_rejects_bad_number) {
    expect_error("1 2\n3 nope", DET_BAD_NUMBER);
    expect_error(".5 1\n2 3", DET_BAD_NUMBER);  // follows the project's number style
}
 
TEST(det_rejects_ragged_and_non_square_matrices) {
    expect_error("1 2\n3", DET_BAD_ROW);
    expect_error("1 2 3\n4 5 6", DET_NOT_SQUARE);
}
 
TEST(det_rejects_matrix_larger_than_one_hundred) {
    expect_error(diagonal_matrix_input(101), DET_TOO_LARGE);
}
 
TEST(det_reports_exact_arithmetic_overflow) {
    expect_error("99999999999999 1\n1 99999999999999", DET_OVERFLOW);
}
 
TEST(det_rejects_null_input) {
    const std::string output = det(nullptr);
    CHECK_EQ(det_status(), DET_NULL);
    CHECK(!output.empty());
}