#include <string>
#include <vector>

#include "common/latex.h"
#include "lu/lu_core.h"
#include "solvers/solvers.h"
#include "solvers/gauss_parser.h"

namespace{
    constexpr int MAX_STEP_N = 8;

    std::string g_result;
    int g_status = SOLVE_OK;

    const char* finish(int status, const std::string& text){
        g_status = status;
        g_result = text;
        return g_result.c_str();
    };

    SolveStatus lu_steps(const ParsedSystem& ps, LatexDoc& doc, std::string& err){
        const int n = (int)ps.vars.size();
        const bool show = n <= MAX_STEP_N;

        Matrix A(n, std::vector<Fraction>(n));
        std::vector<Fraction> b(n);
        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) A[r][c] = ps.aug[r][c];
            b[r] = ps.aug[r][n];
        }

        doc.heading("Step 1: The system we read");
        for (int r = 0; r < n; ++r) doc.math(equation_tex(ps.aug[r], ps.vars));

        doc.heading("Step 2: Write it as A x = b");
        if (show) doc.math("A = " + matrix_tex(A, false) + ", \\quad \\mathbf{b} = " + column_tex(b));
        else doc.text("The system is large, so intermediate matrices are not shown");

        const LUResult f = lu_decompose(A);
        if(frac_detail::overflow){
            err = "The numbers became too large to handle exactly.";
            return SOLVE_OVERFLOW;
        }

        if (!f.ok) {
            err = "No unique solution: the variable '" + ps.vars[f.bad_col] +
                "' cannot be pinned down (the equations are dependent or contradict each other).";
            return SOLVE_SINGULAR;
        }

        doc.heading("Step 3: Factor P A = L U");
        if (show) {
            doc.text("Row operations used (each multiplier is saved in L)");
            for (const LUStep& s : f.steps) {
                if (s.is_swap) {
                    doc.math("R_{" + std::to_string(s.row + 1) + "} \\leftrightarrow R_{" + std::to_string(s.other + 1) + "}");
                } else {
                    doc.math(row_op_tex(s.row, s.mult, s.other) + " \\qquad l_{" + std::to_string(s.row + 1) + "," +
                            std::to_string(s.other + 1) + "} = " + s.mult.latex());
                }
            }
            Matrix P(n, std::vector<Fraction>(n));                // all zeros
            for (int i = 0; i < n; ++i) P[i][f.perm[i]] = Fraction(1);   // a single 1 per row
            doc.math("P = " + matrix_tex(P, false));
            doc.math("L = " + matrix_tex(f.L, false));
            doc.math("U = " + matrix_tex(f.U, false));
        } else {
            doc.text("Factorisation finished");
        }

        doc.heading("Step 4: Forward substitution, solve L c = P b");
        std::vector<Fraction> pb(n);
        for (int i = 0; i < n; ++i) pb[i] = b[f.perm[i]];
        const std::vector<Fraction> c = forward_substitute(f.L, pb);
        if (show) {
            doc.math("P\\mathbf{b} = " + column_tex(pb));
            for (int i = 0; i < n; ++i) doc.math(forward_line_tex(f.L[i], pb[i], i, c));
        } else {
            doc.text("Done");
        }

        doc.heading("Step 5: Back substitution, solve U x = c");
        const std::vector<Fraction> x = back_substitute(f.U, c);
        if (frac_detail::overflow) {
            err = "The numbers became too large to handle exactly.";
            return SOLVE_OVERFLOW;
        }
        if (show) {
            for (int i = n - 1; i >= 0; --i) doc.math(back_line_tex(f.U[i], c[i], i, x, ps.vars));
        } else {
            doc.text("Done");
        }

        doc.heading("Solution");
        for (int i = 0; i < n; ++i) doc.math(var_tex(ps.vars[i]) + " = " + x[i].latex());
        return SOLVE_OK;
    }
}

MAT_API const char* solve_lu(const char* input) {
    if (input == nullptr) return finish(SOLVE_NULL, "Input is null.");

    const ParsedSystem ps = parse_system(input);
    if (ps.status != SOLVE_OK) return finish(ps.status, ps.message);

    LatexDoc doc;
    std::string err;
    const SolveStatus st = lu_steps(ps, doc, err);
    if (st != SOLVE_OK) return finish(st, err);
    return finish(SOLVE_OK, doc.str());
}

MAT_API int solve_lu_status() { return g_status; }