#include <string>

#include "regression/linear_regression.h"
#include "test_util.h"

TEST(regression_example_from_user) {
    const std::string input = "(0, 1) (1, 2) (1, 3) (1.5, 4) (2, 5) (3, 6)";
    const std::string out = linear_regression(input.c_str());

    CHECK_EQ(linear_regression_status(), REG_OK);
    CHECK(contains(out, "\\hat{y}"));
    CHECK(contains(out, "b_0 = \\frac{123}{125}"));
    CHECK(contains(out, "b_1 = \\frac{222}{125}"));
    CHECK(contains(out, "\\frac{134}{375}"));
    CHECK(contains(out, "\\frac{4107}{4375}"));
    CHECK(contains(out, "Mean Absolute Error"));
    CHECK(contains(out, "R-squared"));
}

TEST(regression_shows_matrix_steps) {
    const std::string out = linear_regression("(0, 1) (1, 2) (2, 3)");
    CHECK_EQ(linear_regression_status(), REG_OK);
    CHECK(contains(out, "design matrix"));
    CHECK(contains(out, "normal equation"));
    CHECK(contains(out, "LUP"));
    CHECK(contains(out, "forward and back substitution"));
    CHECK(contains(out, "predicted values"));
    CHECK(contains(out, "SSE"));
    CHECK(contains(out, "SST"));
}

TEST(regression_perfect_positive_line) {
    const std::string out = linear_regression("(0, 1) (1, 3) (2, 5)");
    CHECK_EQ(linear_regression_status(), REG_OK);
    CHECK(contains(out, "b_0 = 1"));
    CHECK(contains(out, "b_1 = 2"));
    CHECK(contains(out, "\\hat{y} = 1 + 2x"));
    CHECK(contains(out, "MAE = \\frac{1}{3}\\left(0 + 0 + 0\\right) = 0"));
    CHECK(contains(out, "R^2 = 1"));
}

TEST(regression_negative_slope) {
    const std::string out = linear_regression("(0, 5) (1, 3) (2, 1)");
    CHECK_EQ(linear_regression_status(), REG_OK);
    CHECK(contains(out, "b_0 = 5"));
    CHECK(contains(out, "b_1 = -2"));
    CHECK(contains(out, "\\hat{y} = 5 - 2x"));
    CHECK(contains(out, "MAE = \\frac{1}{3}\\left(0 + 0 + 0\\right) = 0"));
}

TEST(regression_repeated_x_values_are_allowed) {
    // Repeated x-values are fine as long as the full data set has at least
    // two distinct x-values. The public example also exercises this condition.
    const std::string out = linear_regression("(0, 1) (1, 2) (1, 3) (2, 5)");
    CHECK_EQ(linear_regression_status(), REG_OK);
    CHECK(contains(out, "\\hat{y}"));
}

TEST(regression_constant_y_makes_r_squared_undefined) {
    const std::string out = linear_regression("(0, 2) (1, 2) (2, 2)");
    CHECK_EQ(linear_regression_status(), REG_OK);
    CHECK(contains(out, "MAE = \\frac{1}{3}\\left(0 + 0 + 0\\right) = 0"));
    CHECK(contains(out, "R^2 = \\text{undefined (SST = 0)}"));
}

TEST(regression_fraction_and_decimal_input) {
    const std::string out = linear_regression("(-1.5, 4.25) (0.5, 2.25) (2.5, 0.25)");
    CHECK_EQ(linear_regression_status(), REG_OK);
    CHECK(contains(out, "\\hat{y}"));
    CHECK(contains(out, "MAE ="));
    CHECK(contains(out, "R^2 ="));
}

TEST(regression_rejects_empty_input) {
    const std::string out = linear_regression("   \n  ");
    CHECK_EQ(linear_regression_status(), REG_EMPTY);
    CHECK(!out.empty());
}

TEST(regression_rejects_bad_coordinate_syntax) {
    const std::string out = linear_regression("(0 1) (1, 2)");
    CHECK_EQ(linear_regression_status(), REG_SYNTAX);
    CHECK(!out.empty());
}

TEST(regression_rejects_bad_number) {
    const std::string out = linear_regression("(0, abc) (1, 2)");
    CHECK_EQ(linear_regression_status(), REG_BAD_NUMBER);
    CHECK(!out.empty());
}

TEST(regression_needs_at_least_two_points) {
    const std::string out = linear_regression("(0, 1)");
    CHECK_EQ(linear_regression_status(), REG_TOO_FEW_POINTS);
    CHECK(!out.empty());
}

TEST(regression_rejects_same_x_for_every_point) {
    const std::string out = linear_regression("(2, 1) (2, 3) (2, 5)");
    CHECK_EQ(linear_regression_status(), REG_SINGULAR_X);
    CHECK(!out.empty());
}

TEST(regression_rejects_more_than_100_points) {
    std::string input;
    for (int i = 0; i < 101; ++i) {
        if (i > 0) input += " ";
        input += "(" + std::to_string(i) + ", " + std::to_string(2 * i + 1) + ")";
    }
    const std::string out = linear_regression(input.c_str());
    CHECK_EQ(linear_regression_status(), REG_TOO_MANY_POINTS);
    CHECK(!out.empty());
}

TEST(regression_null_input) {
    const std::string out = linear_regression(nullptr);
    CHECK_EQ(linear_regression_status(), REG_NULL);
    CHECK(!out.empty());
}