#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "common/common.h"
#include "common/fraction.h"
#include "common/latex.h"
#include "common/matrix.h"
#include "lu/lu_core.h"
#include "regression/linear_regression.h"

namespace{
    constexpr int MAX_POINTS = 100;
    constexpr int MAX_DETAIL_ROWS = 12;
    constexpr long long FRACTION_LIMIT = 4000000000000000000LL;

    struct Point{
        Fraction x;
        Fraction y;
    };
    struct ParsedPoints{
        int status = REG_OK;
        std::string message;
        std::vector<Point> points;
    };

    std::string g_result;
    int g_status = REG_OK;

    const char* finish(int status, const std::string& text){
        g_status = status;
        g_result = text;
        return g_result.c_str();
    }

    bool is_space(char ch){
        return std::isspace(static_cast<unsigned char>(ch)) != 0;
    }

    void skip_spaces(const std::string& input, size_t& pos) {
        while (pos < input.size() && is_space(input[pos])) pos++;
    }

    // Convert a signed integer or finite decimal token to an exact Fraction.
    bool parse_decimal_token(const std::string& token, Fraction& value, int& error_status){
        if(token.empty()){
            error_status = REG_BAD_NUMBER;
            return false;
        }
        size_t start = 0;
        bool negative = false;
        if(token[start] == '+' || token[start] == '-'){
            negative = token[start] == '-';
            start++;
        }
        if(start >= token.size()){
            error_status = REG_BAD_NUMBER;
            return false;
        }

        size_t dot_pos = std::string::npos;
        int digits_before = 0;
        int digits_after = 0;
        for(size_t i = start; i < token.size(); i++){
            const char ch = token[i];
            if(ch == '.'){
                if (dot_pos != std::string::npos) {
                    error_status = REG_BAD_NUMBER;
                    return false;
                }
                dot_pos = i;
            }else if(ch >= '0' && ch <= '9'){
                if(dot_pos == std::string::npos) digits_before++;
                else digits_after++;
            }else{
                error_status = REG_BAD_NUMBER;
                return false;
            }
        }

        if(digits_before == 0 || (dot_pos != std::string::npos && digits_after == 0)) {
            error_status = REG_BAD_NUMBER;
            return false;
        }

        __int128 whole = 0;
        const size_t whole_end = dot_pos == std::string::npos ? token.size() : dot_pos;
        for (size_t i = start; i < whole_end; ++i) {
            whole = whole * 10 + (token[i] - '0');
            if (whole > FRACTION_LIMIT) {
                frac_detail::overflow = true;
                error_status = REG_OVERFLOW;
                return false;
            }
        }

        __int128 denominator = 1;
        __int128 fractional = 0;
        if(dot_pos != std::string::npos){
            for(size_t i = dot_pos + 1; i < token.size(); i++){
                if (denominator > FRACTION_LIMIT / 10) {
                    frac_detail::overflow = true;
                    error_status = REG_OVERFLOW;
                    return false;
                }
                denominator *= 10;
                fractional = fractional * 10 + (token[i] - '0');
            }
        }

        if(whole > FRACTION_LIMIT / denominator){
            frac_detail::overflow = true;
            error_status = REG_OVERFLOW;
            return false;
        }

        __int128 numerator = whole * denominator + fractional;
        if (numerator > FRACTION_LIMIT) {
            frac_detail::overflow = true;
            error_status = REG_OVERFLOW;
            return false;
        }

        if(negative) numerator = -numerator;

        value = Fraction::make(numerator, denominator);
        if (frac_detail::overflow) {
            error_status = REG_OVERFLOW;
            return false;
        }
        return true;
    }

    bool parse_number_at(const std::string& input, size_t& pos,
                        Fraction& value, int& error_status){
        skip_spaces(input, pos);
        const size_t start = pos;
        if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) ++pos;
        while (pos < input.size()) {
            const char ch = input[pos];
            if ((ch >= '0' && ch <= '9') || ch == '.') ++pos;
            else break;
        }
        const std::string token = input.substr(start, pos - start);
        return parse_decimal_token(token, value, error_status);
    }

    ParsedPoints parse_points(const std::string& input){
        ParsedPoints parsed;
        size_t pos = 0;
        skip_spaces(input, pos);
        if(pos == input.size()){
            parsed.status = REG_EMPTY;
            parsed.message = "Enter at least two coordinate pairs, for example (0, 1) (1, 2).";
            return parsed;
        }

        while(pos < input.size()) {
            skip_spaces(input, pos);
            if(pos == input.size()) break;
            if(input[pos] != '('){
                parsed.status = REG_SYNTAX;
                parsed.message = "Expected '(' before a coordinate pair. Use the format (x, y) (x, y) ...";
                return parsed;
            }
            pos++;

            Point point;
            int number_error = REG_BAD_NUMBER;
            if(!parse_number_at(input, pos, point.x, number_error)){
                parsed.status = number_error;
                parsed.message = number_error == REG_OVERFLOW
                    ? "A coordinate is too large to represent exactly."
                    : "An x-coordinate is not a supported number. Use integers or decimals such as 1.5.";
                return parsed;
            }

            skip_spaces(input, pos);
            if (pos >= input.size() || input[pos] != ',') {
                parsed.status = REG_SYNTAX;
                parsed.message = "Expected a comma between x and y, as in (1.5, 4).";
                return parsed;
            }
            pos++;

            if(!parse_number_at(input, pos, point.y, number_error)){
                parsed.status = number_error;
                parsed.message = number_error == REG_OVERFLOW
                    ? "A coordinate is too large to represent exactly."
                    : "A y-coordinate is not a supported number. Use integers or decimals such as 1.5.";
                return parsed;
            }

            skip_spaces(input, pos);

            if (pos >= input.size() || input[pos] != ')') {
                parsed.status = REG_SYNTAX;
                parsed.message = "Expected ')' after the y-coordinate.";
                return parsed;
            }
            pos++;

            if(static_cast<int>(parsed.points.size()) >= MAX_POINTS) {
                parsed.status = REG_TOO_MANY_POINTS;
                parsed.message = "Too many pairs!";
                return parsed;
            }
            parsed.points.push_back(point);

            skip_spaces(input, pos);

            if (pos < input.size() && input[pos] != '(') {
                parsed.status = REG_SYNTAX;
                parsed.message = "Separate coordinate pairs with whitespace, for example (0, 1) (1, 2).";
                return parsed;
            }
        }
        if (parsed.points.size() < 2) {
            parsed.status = REG_TOO_FEW_POINTS;
            parsed.message = "Linear regression needs at least two coordinate pairs.";
            return parsed;
        }
        return parsed;
    }

    Fraction absolute(const Fraction& value){
        return value.n < 0 ? -value : value;
    }

    std::string decimal_approx(const Fraction& value) {
        const long double number = static_cast<long double>(value.n) /
                                   static_cast<long double>(value.d);
        std::ostringstream out;
        out << std::fixed << std::setprecision(6) << number;
        std::string result = out.str();
        while (!result.empty() && result.back() == '0') result.pop_back();
        if (!result.empty() && result.back() == '.') result.pop_back();
        if (result == "-0") result = "0";
        return result.empty() ? "0" : result;
    }

    std::string regression_formula_tex(const Fraction& intercept, const Fraction& slope){
        std::string result = "\\hat{y} = ";
        bool has_term = false;
        if (!intercept.is_zero()) {
            result += intercept.latex();
            has_term = true;
        }

        if (!slope.is_zero()) {
            const bool negative = slope.n < 0;
            const Fraction magnitude = absolute(slope);
            if (has_term) result += negative ? " - " : " + ";
            else if (negative) result += "-";
            if (!is_one(magnitude)) result += magnitude.latex();
            result += "x";
            has_term = true;
        }
        if (!has_term) result += "0";
        return result;
    }

    std::string point_set_tex(const std::vector<Point>& points) {
        std::string result = R"(\mathcal{D} = \{)";
        for (size_t i = 0; i < points.size(); ++i) {
            if (i > 0) result += ",\\; ";
            result += "(" + points[i].x.latex() + ",\\," + points[i].y.latex() + ")";
        }
        result += R"(\})";
        return result;
    }

    std::string swap_tex(int row, int other) {
        return "R_{" + std::to_string(row + 1) + "} \\leftrightarrow R_{" +
               std::to_string(other + 1) + "}";
    }

    RegressionStatus calculate_regression(const std::vector<Point>& points, LatexDoc& doc, std::string& error) {
        const int n = static_cast<int>(points.size());
        const bool show_details = n <= MAX_DETAIL_ROWS;

        Matrix X(n, std::vector<Fraction>(2));
        std::vector<Fraction> y(n);
        Fraction sum_x, sum_x2, sum_y, sum_xy;
        for(int i = 0; i < n; i++){
            X[i][0] = Fraction(1);
            X[i][1] = points[i].x;
            y[i] = points[i].y;
            sum_x = sum_x + points[i].x;
            sum_x2 = sum_x2 + points[i].x * points[i].x;
            sum_y = sum_y + points[i].y;
            sum_xy = sum_xy + points[i].x * points[i].y;
        }
        if(frac_detail::overflow){
            error = "The coordinates became too large for exact Fraction arithmetic.";
            return static_cast<RegressionStatus>(REG_OVERFLOW);
        }

        // (X^T X) beta = X^T y, where beta = [intercept, slope]^T.
        Matrix normal = {
            {Fraction(n), sum_x},
            {sum_x, sum_x2}
        };
        std::vector<Fraction> rhs = {sum_y, sum_xy};

        doc.heading("Step 1: Read the coordinate data");
        if(show_details){
            doc.math(point_set_tex(points));
        }else{
            doc.math("\\mathcal{D} = \\{(x_i,y_i)\\}_{i=1}^{n}, \\quad n = " + std::to_string(n));
        }
        doc.text("Each pair is one observed point: the first number is x and the second number is y.");

        doc.heading("Step 2: Choose the straight-line model");
        doc.math("\\hat{y} = b_0 + b_1x");
        doc.text("\\(b_0\\) is the intercept and \\(b_1\\) is the slope. Least squares chooses the coefficients that minimise the sum of squared residuals.");

        doc.heading("Step 3: Build the design matrix and observation vector");
        if(show_details){
            doc.math("X = " + matrix_tex(X, false));
            doc.math("\\mathbf{y} = " + column_tex(y));
        }else{
            doc.text("The data contains more than 12 points, so the full design matrix and point-by-point table are omitted to keep the report readable.");

            doc.math(R"(X = \begin{bmatrix}1 & x_1\\1 & x_2\\\vdots & \vdots\\1 & x_n\end{bmatrix}, \quad \mathbf{y} = \begin{bmatrix}y_1\\y_2\\\vdots\\y_n\end{bmatrix})");
        }

        doc.heading("Step 4: Apply the normal equation");
        doc.math("X^T X\\boldsymbol{\\beta} = X^T\\mathbf{y}, \\quad \\boldsymbol{\\beta} = \\begin{bmatrix}b_0\\\\b_1\\end{bmatrix}");
        doc.text("This turns least-squares fitting into a 2 by 2 linear system, which the existing LUP code can solve.");

        doc.heading("Step 5: Calculate the sums and form the 2 by 2 system");
        doc.math("n = " + std::to_string(n) +
                ",\\quad \\sum x_i = " + sum_x.latex() +
                ",\\quad \\sum x_i^2 = " + sum_x2.latex());
        doc.math("\\sum y_i = " + sum_y.latex() +
                ",\\quad \\sum x_i y_i = " + sum_xy.latex());
        doc.math("X^T X = " + matrix_tex(normal, false));
        doc.math("X^T\\mathbf{y} = " + column_tex(rhs));
        if (frac_detail::overflow) {
            error = "The normal-equation values became too large for exact Fraction arithmetic.";
            return static_cast<RegressionStatus>(REG_OVERFLOW);
        }

        doc.heading("Step 6: Factor the normal matrix using LUP");
        doc.math("A = X^T X, \\quad A\\boldsymbol{\\beta} = \\mathbf{z}, \\quad PA = LU, \\quad \\mathbf{z} = X^T\\mathbf{y}");doc.math("A = X^T X, \\quad A\\boldsymbol{\\beta} = \\mathbf{z}, \\quad PA = LU, \\quad \\mathbf{z} = X^T\\mathbf{y}");

        const LUResult factor = lu_decompose(normal);
        if (frac_detail::overflow) {
            error = "The LUP factorisation exceeded the exact Fraction range.";
            return static_cast<RegressionStatus>(REG_OVERFLOW);
        }
        if (!factor.ok) {
            error = "The x-values do not contain two distinct values, so a unique slope and intercept cannot be calculated.";
            return static_cast<RegressionStatus>(REG_SINGULAR_X);
        }

        if(show_details){
             for (const LUStep& step : factor.steps) {
                if (step.is_swap) {
                    doc.math(swap_tex(step.row, step.other));
                } else {
                    doc.math(row_op_tex(step.row, step.mult, step.other) +
                             "\\qquad l_{" + std::to_string(step.row + 1) + "," +
                             std::to_string(step.other + 1) + "} = " + step.mult.latex());
                }
            }
            Matrix P(2, std::vector<Fraction>(2));
            for (int i = 0; i < 2; ++i) P[i][factor.perm[i]] = Fraction(1);
            doc.math("P = " + matrix_tex(P, false));
            doc.math("L = " + matrix_tex(factor.L, false));
            doc.math("U = " + matrix_tex(factor.U, false));
        }else{
            doc.text("The LUP factorisation was completed; its small 2 by 2 factors are omitted here.");
        }

        doc.heading("Step 7: Solve by forward and back substitution");
        std::vector<Fraction> permuted_rhs(2);
        for (int i = 0; i < 2; ++i) permuted_rhs[i] = rhs[factor.perm[i]];
        const std::vector<Fraction> c = forward_substitute(factor.L, permuted_rhs);
        if (frac_detail::overflow) {
            error = "Forward substitution exceeded the exact Fraction range.";
            return static_cast<RegressionStatus>(REG_OVERFLOW);
        }

        const std::vector<std::string> coefficient_names = {"b_0", "b_1"};
        if (show_details) {
            doc.math("P\\mathbf{z} = " + column_tex(permuted_rhs));
            for (int i = 0; i < 2; ++i) {
                doc.math(forward_line_tex(factor.L[i], permuted_rhs[i], i, c));
            }
        }

        const std::vector<Fraction> beta = back_substitute(factor.U, c);
        if (frac_detail::overflow) {
            error = "Back substitution exceeded the exact Fraction range.";
            return static_cast<RegressionStatus>(REG_OVERFLOW);
        }
        if (show_details) {
            for (int i = 1; i >= 0; --i) {
                doc.math(back_line_tex(factor.U[i], c[i], i, beta, coefficient_names));
            }
        } else {
            doc.text("The two-coefficient solve completed using forward and back substitution; intermediate lines are omitted for readability.");
        }

        const Fraction intercept = beta[0];
        const Fraction slope = beta[1];
        doc.heading("Step 8: Write the fitted function");
        doc.math("b_0 = " + intercept.latex() + " \\approx " + decimal_approx(intercept));
        doc.math("b_1 = " + slope.latex() + " \\approx " + decimal_approx(slope));
        doc.math("\\boxed{" + regression_formula_tex(intercept, slope) + "}");

        // Compute predictions, signed residuals, MAE, SSE, SST, and R-squared.
        std::vector<Fraction> predictions(n), residuals(n);
        Fraction absolute_error_sum;
        Fraction sse;
        for(int i = 0; i < n; i++){
            predictions[i] = intercept + slope * points[i].x;
            residuals[i] = points[i].y - predictions[i];  // observed minus predicted
            absolute_error_sum = absolute_error_sum + absolute(residuals[i]);
            sse = sse + residuals[i] * residuals[i];
        }
        if(frac_detail::overflow){
            error = "Prediction or residual calculations exceeded the exact Fraction range.";
            return static_cast<RegressionStatus>(REG_OVERFLOW);
        }

        doc.heading("Step 9: Compare observed and predicted values");
        doc.math("e_i = y_i - \\hat{y}_i");
        if (show_details) {
            std::string table = "\\begin{array}{r|r|r|r}x_i & y_i & \\hat{y}_i & e_i = y_i-\\hat{y}_i \\\\ \\hline ";
            for (int i = 0; i < n; ++i) {
                if (i > 0) table += " \\\\ ";
                table += points[i].x.latex() + " & " + points[i].y.latex() + " & " +
                         predictions[i].latex() + " & " + residuals[i].latex();
            }
            table += "\\end{array}";
            doc.math(table);
        } else {
            doc.text("The row-by-row table is omitted for readability; every point is still included in the metrics below.");
        }

        const Fraction mae = absolute_error_sum / Fraction(n);
        if (frac_detail::overflow) {
            error = "MAE calculation exceeded the exact Fraction range.";
            return static_cast<RegressionStatus>(REG_OVERFLOW);
        }

        doc.heading("Step 10: Calculate Mean Absolute Error (MAE)");
        doc.math(
            "MAE = \\frac{1}{n}\\sum_{i=1}^{n}|y_i-\\hat{y}_i|"
        );
        std::string mae_line;

        if (show_details) {
            mae_line = "MAE = \\frac{1}{" + std::to_string(n) + "}\\left(";
            for (int i = 0; i < n; ++i) {
                if (i > 0) {
                    mae_line += " + ";
                }
                mae_line += absolute(residuals[i]).latex();
            }
            mae_line += "\\right)";
            mae_line += " = " + mae.latex()
                    + " \\approx " + decimal_approx(mae);
        } else {
            mae_line = "MAE = " + mae.latex()
                    + " \\approx " + decimal_approx(mae);
        }
        doc.math(mae_line);
        doc.text(
            "MAE is the average absolute size of the residuals. "
            "Smaller is better, and zero means every prediction "
            "matches its observed y-value."
        );

        const Fraction mean_y = sum_y / Fraction(n);

        Fraction sst;
        for (int i = 0; i < n; ++i) {
            const Fraction centered = points[i].y - mean_y;
            sst = sst + centered * centered;
        }
        if (frac_detail::overflow) {
            error = "The R-squared sums exceeded the exact Fraction range.";
            return static_cast<RegressionStatus>(REG_OVERFLOW);
        }

        doc.heading("Step 11: Calculate R-squared");
        doc.math("\\bar{y} = \\frac{1}{n}\\sum_{i=1}^{n}y_i = " + mean_y.latex());
        doc.math("SSE = \\sum_{i=1}^{n}(y_i-\\hat{y}_i)^2 = " + sse.latex());
        doc.math("SST = \\sum_{i=1}^{n}(y_i-\\bar{y})^2 = " + sst.latex());

        bool r2_defined = !sst.is_zero();
        Fraction r_squared;
        if (r2_defined) {
            r_squared = Fraction(1) - sse / sst;
            if (frac_detail::overflow) {
                error = "R-squared calculation exceeded the exact Fraction range.";
                return static_cast<RegressionStatus>(REG_OVERFLOW);
            }
            doc.math("R^2 = 1 - \\frac{SSE}{SST} = 1 - \\frac{" + sse.latex() + "}{" +
                    sst.latex() + "} = " + r_squared.latex() + " \\approx " +
                    decimal_approx(r_squared));
            doc.text("R-squared measures how much of the variation in y is explained by the fitted line. A value closer to 1 indicates a closer fit; it can be negative for a poor fit.");
        } else {
            doc.math("R^2 = 1 - \\frac{SSE}{SST} \\quad \\text{is undefined because } SST = 0");
            doc.text("All observed y-values are identical, so the total variation is zero and the usual R-squared formula divides by zero. The regression itself is still valid.");
        }

        doc.heading("Final summary");
        doc.math("\\text{Fitted function: } " + regression_formula_tex(intercept, slope));
        doc.math("\\text{MAE} = " + mae.latex() + " \\approx " + decimal_approx(mae));
        if (r2_defined) {
            doc.math("R^2 = " + r_squared.latex() + " \\approx " + decimal_approx(r_squared));
        } else {
            doc.math("R^2 = \\text{undefined (SST = 0)}");
        }

        return static_cast<RegressionStatus>(REG_OK);
    }
}

MAT_API const char* linear_regression(const char* input) {
    frac_detail::overflow = false;
    if (input == nullptr) {
        return finish(REG_NULL, "Input is null.");
    }
    const ParsedPoints parsed = parse_points(input);

    if (frac_detail::overflow || parsed.status == REG_OVERFLOW) {
        return finish(REG_OVERFLOW, "A coordinate is too large to represent exactly.");
    }

    if (parsed.status != REG_OK) {
        return finish(parsed.status, parsed.message);
    }

    LatexDoc doc;
    std::string error;
    const RegressionStatus status = calculate_regression(parsed.points, doc, error);
    if (status != REG_OK) return finish(status, error);
    return finish(REG_OK, doc.str());
}

MAT_API int linear_regression_status() {
    return g_status;
}