#include <string>

#include "solver_check.h"
#include "solvers/solvers.h"
#include "test_util.h"

TEST(gauss_two_by_two) {                                   // the example from the task
    expect_solution(solve_gauss, solve_gauss_status,
                    "x + 2y = 200\n2x + 3y = 120", {"x = -360", "y = 280"});
}

TEST(gauss_shows_its_steps) {
    expect_solution(solve_gauss, solve_gauss_status, "x + 2y = 200\n2x + 3y = 120",
                    {"Forward elimination", "Back substitution", "R_{2} \\leftarrow R_{2} - 2R_{1}"});
}

TEST(gauss_fraction_answer) {
    expect_solution(solve_gauss, solve_gauss_status, "2x + y = 1\nx + 3y = 1",
                    {"x = \\frac{2}{5}", "y = \\frac{1}{5}"});
}

TEST(gauss_three_by_three) {
    expect_solution(solve_gauss, solve_gauss_status, "x + y + z = 6\n2y + 5z = -4\n2x + 5y - z = 27",
                    {"x = 5", "y = 3", "z = -2"});
}

TEST(gauss_needs_row_swap) {                               // 0x in the first pivot spot
    expect_solution(solve_gauss, solve_gauss_status, "0x + y = 2\nx + y = 3",
                    {"\\leftrightarrow", "x = 1", "y = 2"});
}

TEST(gauss_variables_and_numbers_on_both_sides) {
    expect_solution(solve_gauss, solve_gauss_status, "x = y + 3\nx + 1 = 8 - y", {"x = 5", "y = 2"});
}

TEST(gauss_decimals_and_star) {
    expect_solution(solve_gauss, solve_gauss_status, "0.5x + y = 3\n2*x - y = 1",
                    {"x = \\frac{8}{5}", "y = \\frac{11}{5}"});
}

TEST(gauss_windows_line_endings_and_blank_lines) {
    expect_solution(solve_gauss, solve_gauss_status, "x + y = 3\r\n\r\nx - y = 1\r\n", {"x = 2", "y = 1"});
}

TEST(gauss_missing_variable_in_a_line_means_zero) {         // second line has no y
    expect_solution(solve_gauss, solve_gauss_status, "x + y = 3\nx = 1", {"x = 1", "y = 2"});
}

TEST(gauss_exactly_100_variables) {
    expect_solution(solve_gauss, solve_gauss_status, diagonal_system(100),
                    {"\\mathit{v1} = 1", "\\mathit{v100} = 100"});
}

// ------------------------------- error paths --------------------------------

TEST(gauss_empty_input) {
    expect_error(solve_gauss, solve_gauss_status, "", 1);
    expect_error(solve_gauss, solve_gauss_status, "  \n\n  ", 1);
}

TEST(gauss_syntax_errors) {
    expect_error(solve_gauss, solve_gauss_status, "x + 2y 200\ny = 1", 2);        // no '='
    expect_error(solve_gauss, solve_gauss_status, "x = 1 = 2", 2);                // two '='
    expect_error(solve_gauss, solve_gauss_status, "x + = 3", 2);                  // dangling '+'
    expect_error(solve_gauss, solve_gauss_status, "= 3", 2);                      // empty left side
    expect_error(solve_gauss, solve_gauss_status, "x + 2y = 3 $ 4", 2);           // stray symbol
    expect_error(solve_gauss, solve_gauss_status, "x 2 = 3", 2);                  // missing operator
    expect_error(solve_gauss, solve_gauss_status, "3 * = 3", 2);                  // '*' without variable
}

TEST(gauss_bad_numbers) {
    expect_error(solve_gauss, solve_gauss_status, "x + 2..5y = 3\ny = 1", 3);
    expect_error(solve_gauss, solve_gauss_status, "x + 1.2.3y = 3\ny = 1", 3);
    expect_error(solve_gauss, solve_gauss_status, "x = 1.\ny = 1", 3);
    expect_error(solve_gauss, solve_gauss_status, "x = .5\ny = 1", 3);
    expect_error(solve_gauss, solve_gauss_status, "x = 1234567890123456\ny = 1", 3);   // 16 digits
}

TEST(gauss_duplicate_variables) {
    expect_error(solve_gauss, solve_gauss_status, "x + 2x = 5", 4);
    expect_error(solve_gauss, solve_gauss_status, "x = x + 1", 4);                // x on both sides
}

TEST(gauss_wrong_number_of_equations) {
    expect_error(solve_gauss, solve_gauss_status, "x + y = 3", 6);                       // 1 eq, 2 vars
    expect_error(solve_gauss, solve_gauss_status, "x + y = 3\nx - y = 1\nx + 2y = 4", 6); // 3 eq, 2 vars
    expect_error(solve_gauss, solve_gauss_status, "x + y + z = 3\nx - y = 1", 6);         // 2 eq, 3 vars
}

TEST(gauss_more_than_100_variables) {
    expect_error(solve_gauss, solve_gauss_status, diagonal_system(101), 5);
}

TEST(gauss_singular_systems) {
    expect_error(solve_gauss, solve_gauss_status, "x + y = 2\n2x + 2y = 4", 7);   // dependent
    expect_error(solve_gauss, solve_gauss_status, "x + y = 2\nx + y = 3", 7);      // contradiction
}

TEST(gauss_overflow_is_reported) {
    expect_error(solve_gauss, solve_gauss_status,
                 "99999999999999x + y = 1\nx + 99999999999999y = 1", 8);
}

TEST(gauss_null_input) {
    const std::string msg = solve_gauss(nullptr);
    CHECK_EQ(solve_gauss_status(), 9);
    CHECK(!msg.empty());
}