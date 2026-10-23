#include "determinants/determinants.h"

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "common/latex.h"

namespace{
    constexpr int MAX_DET_N = 100;

    std::string g_result;
    int g_status = DET_OK;

    const char* finish(DetStatus status, const std::string& text){
        g_status = status;
        g_result = text;
        return g_result.c_str();
    }

    bool is_square_matrix(const Matrix& A){
        if(A.empty()) return false;
        const size_t n = A.size();
        for(const auto& row : A)
            if(row.size() != n) return false;
        return true;
    }

    bool same_fraction(const Fraction& a, const Fraction& b) {
        return a.n == b.n && a.d == b.d;
    }

    bool parse_number(const std::string& token, Fraction& out){
        if(token.empty()) return false;

        size_t start = 0;
        bool negative = false;
        if (token[start] == '+' || token[start] == '-') {
            negative = token[start] == '-';
            start++;
        }
        if (start >= token.size()) return false;

        const std::string body = token.substr(start);
        const size_t dot = body.find('.');
        const std::string int_part = (dot == std::string::npos)
            ? body : body.substr(0, dot);
        const std::string frac_part = (dot == std::string::npos)
            ? "" : body.substr(dot + 1);
        
        if(int_part.empty()) return false;
        if(dot != std::string::npos && frac_part.empty()) return false;
        if (frac_part.find('.') != std::string::npos) return false;
        if (int_part.size() + frac_part.size() > 15) return false;

        for (char ch : int_part)
            if (!std::isdigit(static_cast<unsigned char>(ch))) return false;
        for (char ch : frac_part)
            if (!std::isdigit(static_cast<unsigned char>(ch))) return false;

        __int128 numerator = 0;
        __int128 denominator = 1;

        for(char ch : int_part + frac_part)
            numerator = numerator * 10 + (ch - '0');
        for (size_t i = 0; i < frac_part.size(); ++i)
            denominator *= 10;

        out = Fraction::make(numerator, denominator);
        if (negative) out = -out;
        return true;
    }

    DetStatus parse_matrix_input(const std::string& input, Matrix& A, std::string& error){
        frac_detail::overflow = false;
        std::vector<std::vector<Fraction>> rows;
        std::istringstream input_stream(input);
        std::string line;
        int line_no = 0;

        while(std::getline(input_stream, line)) {
            line_no++;
            std::istringstream row_stream(line);
            std::vector<Fraction> row;
            std::string token;

            while(row_stream >> token){
                Fraction value;
                if(!parse_number(token, value)){
                    error = "Line " + std::to_string(line_no) +
                        ": invalid number '" + token +
                        "' (use integers or decimals such as -4 or 2.5).";
                    return DET_BAD_NUMBER;
                }
                if (frac_detail::overflow) {
                    error = "A number in the input is too large to represent exactly.";
                    return DET_OVERFLOW;
                }
                row.push_back(value);
                if(row.size() > MAX_DET_N){
                    error = "Matrix size is limited to 100 rows and 100 columns.";
                    return DET_TOO_LARGE;
                }
            }
            if (!row.empty()) {
                rows.push_back(row);
                if (rows.size() > MAX_DET_N) {
                    error = "Matrix size is limited to 100 rows and 100 columns.";
                    return DET_TOO_LARGE;
                }
            }
        }
        if(rows.empty()){
            error = "No matrix entries found. Enter one row per line, with spaces between entries.";
            return DET_EMPTY;
        }

        const size_t cols = rows[0].size();
        for(size_t r = 0; r < rows.size(); r++){
            if (rows[r].size() != cols) {
                error = "Row " + std::to_string(r + 1) +
                    " has a different number of entries from the other rows.";
                return DET_BAD_ROW;
            }
        }
        
         if (rows.size() != cols) {
            error = "The matrix must be square: the number of rows must equal the number of columns.";
            return DET_NOT_SQUARE;
        }
    
        A = rows;
        return DET_OK;
    }

    Matrix permutation_matrix(const std::vector<int>& perm){
        const int n = static_cast<int>(perm.size());
        Matrix P(n, std::vector<Fraction>(n));
        for (int r = 0; r < n; r++) P[r][perm[r]] = Fraction(1);
        return P;
    }

    std::string parenthesize(const Fraction& f) {
        return "\\left(" + f.latex() + "\\right)";
    }

    void cofactor_steps(const Matrix& A, const Fraction& value, LatexDoc& doc){
        const int n = static_cast<int>(A.size());
        doc.heading("Step 3: Minor and cofactor expansion");

        if (n == 1) {
            doc.math("\\det(A) = a_{11} = " + value.latex());
            return;
        }
    
        if (n == 2) {
            doc.math("\\det(A) = a_{11}a_{22} - a_{12}a_{21}");
            const std::string expanded = parenthesize(A[0][0]) + "\\cdot" +
                parenthesize(A[1][1]) + " - " + parenthesize(A[0][1]) + "\\cdot" +
                parenthesize(A[1][0]) + " = " + value.latex();
            doc.math("= " + expanded);
            return;
        }
    
        // This display is only called for N <= 3.
        doc.math("\\det(A)=a_{11}(a_{22}a_{33}-a_{23}a_{32})"
                "-a_{12}(a_{21}a_{33}-a_{23}a_{31})"
                "+a_{13}(a_{21}a_{32}-a_{22}a_{31})");
    
        const std::string m0 = parenthesize(A[1][1]) + "\\cdot" + parenthesize(A[2][2]) +
            "-" + parenthesize(A[1][2]) + "\\cdot" + parenthesize(A[2][1]);
        const std::string m1 = parenthesize(A[1][0]) + "\\cdot" + parenthesize(A[2][2]) +
            "-" + parenthesize(A[1][2]) + "\\cdot" + parenthesize(A[2][0]);
        const std::string m2 = parenthesize(A[1][0]) + "\\cdot" + parenthesize(A[2][1]) +
            "-" + parenthesize(A[1][1]) + "\\cdot" + parenthesize(A[2][0]);
    
        const std::string expanded = parenthesize(A[0][0]) + "\\left(" + m0 + "\\right) - " +
            parenthesize(A[0][1]) + "\\left(" + m1 + "\\right) + " +
            parenthesize(A[0][2]) + "\\left(" + m2 + "\\right) = " + value.latex();
        doc.math("= " + expanded);
    }
}

DetLUResult det_lu(const Matrix& A){
    DetLUResult result;
    frac_detail::overflow = false;

    if (!is_square_matrix(A)) return result;
    result.valid = true;
    result.lu = lu_decompose(A);
    result.bad_col = result.lu.bad_col;
 
    for (const LUStep& step : result.lu.steps)
        if (step.is_swap) ++result.row_swaps;
 
    if (!result.lu.ok) {
        if (!frac_detail::overflow) {
            result.singular = true;
            result.value = Fraction(0);
        }
        return result;
    }

    Fraction product(1);
    const int n = static_cast<int>(A.size());
    for(int i = 0; i < n; i++)
        product = product * result.lu.U[i][i];
    if (result.row_swaps % 2 != 0) product = -product;
    result.value = product;
    return result;
}

// Main func

MAT_API const char* det(const char* input){
    if(input == nullptr) return finish(DET_NULL, "Input is null.");

    Matrix A;
    std::string error;
    const DetStatus parse_status = parse_matrix_input(input, A, error);
    if (parse_status != DET_OK) return finish(parse_status, error);

    const DetLUResult lu_result = det_lu(A);
    if(frac_detail::overflow)
        return finish(DET_OVERFLOW, "The exact calculation exceeded the supported number range.");
    if (!lu_result.valid)
        return finish(DET_BAD_ROW, "The input could not be represented as a square matrix.");
    
    const bool use_mc = A.size() <= 3;
    Fraction mc_value;
    if (use_mc) {
        mc_value = det_mc(A);
        if (frac_detail::overflow)
            return finish(DET_OVERFLOW, "The exact calculation exceeded the supported number range.");
        if (!same_fraction(lu_result.value, mc_value)) {
            return finish(DET_METHOD_MISMATCH,
                "Internal check failed: LUP and minor/cofactor expansion returned different values.");
        }
    }

    LatexDoc doc;
    doc.heading("Step 1: Read the matrix");
    doc.math("A = " + matrix_tex(A, false));

    doc.heading("Step 2: Determinant using LUP decomposition");
    if(lu_result.singular){
        doc.text("No non-zero pivot could be found in column " +
                 std::to_string(lu_result.bad_col + 1) + ". The matrix is singular.");
        doc.math("\\det(A) = 0");
    } else {
        doc.math("P A = L U");
        for (const LUStep& step : lu_result.lu.steps) {
            if (step.is_swap) {
                doc.math("R_{" + std::to_string(step.row + 1) + "} \\leftrightarrow R_{" +
                         std::to_string(step.other + 1) + "}");
            } else {
                doc.math(row_op_tex(step.row, step.mult, step.other));
            }
        }

        doc.math("P = " + matrix_tex(permutation_matrix(lu_result.lu.perm), false));
        doc.math("L = " + matrix_tex(lu_result.lu.L, false));
        doc.math("U = " + matrix_tex(lu_result.lu.U, false));

        std::string diagonal_product;
        for (size_t i = 0; i < lu_result.lu.U.size(); ++i) {
            if (!diagonal_product.empty()) diagonal_product += " \\cdot ";
            diagonal_product += parenthesize(lu_result.lu.U[i][i]);
        }
        doc.math("\\det(A)=(-1)^{" + std::to_string(lu_result.row_swaps) +
                 "}\\left(" + diagonal_product + "\\right)=" + lu_result.value.latex());
    }

    if(use_mc){
        cofactor_steps(A, mc_value, doc);
        doc.text("The LUP and minor/cofactor results agree.");
    }else{
        doc.heading("Step 3: Verification strategy");
        doc.text("N > 3, so minor/cofactor expansion is skipped. LUP remains the main method.");
    }

    doc.heading("Final result");
    doc.math("\\boxed{\\det(A)=" + lu_result.value.latex() + "}");
    return finish(DET_OK, doc.str());
}

MAT_API int det_status() { return g_status; }