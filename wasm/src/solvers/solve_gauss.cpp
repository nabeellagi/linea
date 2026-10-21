#include <string>
#include <utility>
#include <vector>

#include "common/latex.h"
#include "solvers/gauss_parser.h"
#include "solvers/solvers.h"

namespace {
    constexpr int MAX_STEP_N = 8;

    std::string g_result;
    int g_status = SOLVE_OK;

    const char* finish(int status, const std::string& text){
        g_status = status;
        g_result = text;
        return g_result.c_str();
    }

    SolveStatus gauss_steps(const ParsedSystem& ps, LatexDoc& doc, std::string err) {
        const int n = (int)ps.vars.size();
        const bool show = n <= MAX_STEP_N;
        Matrix a = ps.aug;

        doc.heading("Step 1: The system we read");
        for(int r = 0; r < n; r++) doc.math(equation_tex(a[r], ps.vars));
        
        doc.heading("Step 2: The augmented matrix");
        if (show) doc.math(matrix_tex(a, true));
        else doc.text("The system is large, so intermediate matrices are not shown");

        doc.heading("Step 3: Forward Elimination");
        for(int c = 0; c < n; c++){
            int p = -1;
            for (int r = c; r < n; ++r)
                if (!a[r][c].is_zero()) { p = r; break; }
            if (p < 0) {
                err = "No unique solution: the variable '" + ps.vars[c] +
                    "' cannot be pinned down (the equations are dependent or contradict each other).";
                return SOLVE_SINGULAR;
            }

            if(p != c){
                std::swap(a[p], a[c]);
                if (show) {
                    doc.math("R_{" + std::to_string(c + 1) + "} \\leftrightarrow R_{" + std::to_string(p + 1) + "}");
                    doc.math(matrix_tex(a, true));
                }
            }

            for(int r = c+1; r<n; r++){
                if (a[r][c].is_zero()) continue;             // already zero, nothing to do
                const Fraction m = a[r][c] / a[c][c];        // the multiplier
                for (int k = c; k <= n; ++k) a[r][k] = a[r][k] - m * a[c][k];   // R_r <- R_r - m*R_c
                if (show) {
                    doc.math(row_op_tex(r, m, c));
                    doc.math(matrix_tex(a, true));
                }
            }

            if (frac_detail::overflow) {
                err = "The numbers became too large to handle exactly.";
                return SOLVE_OVERFLOW;
            }
        }

        doc.heading("Step 4: Upper triangular form");
        if (show) doc.math(matrix_tex(a, true));
        else doc.text("Elimination finished!");

        doc.heading("Step 5: Back substitution!");
        doc.text("Start at the last row and walk upward");
        std::vector<Fraction> x(n);
        for (int i = n - 1; i >= 0; --i) {
            Fraction sum = a[i][n]; 
            for (int j = i + 1; j < n; ++j) sum = sum - a[i][j] * x[j]; 
            x[i] = sum / a[i][i];
            if (show) doc.math(back_line_tex(a[i], a[i][n], i, x, ps.vars));
        }
        if (frac_detail::overflow) {
            err = "The numbers became too large to handle exactly.";
            return SOLVE_OVERFLOW;
        }

        doc.heading("Solution");
        for (int i = 0; i < n; ++i) doc.math(var_tex(ps.vars[i]) + " = " + x[i].latex());
        return SOLVE_OK;
    }
}

MAT_API const char* solve_gauss(const char* input) {
    if (input == nullptr) return finish(SOLVE_NULL, "Input is null.");

    const ParsedSystem ps = parse_system(input);         // also resets the overflow flag
    if (ps.status != SOLVE_OK) return finish(ps.status, ps.message);

    LatexDoc doc;
    std::string err;
    const SolveStatus st = gauss_steps(ps, doc, err);
    if (st != SOLVE_OK) return finish(st, err);
    return finish(SOLVE_OK, doc.str());
}

MAT_API int solve_gauss_status() { return g_status; }