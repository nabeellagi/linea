#pragma once

#include <string>
#include <vector>
#include "common/fraction.h"
#include "common/matrix.h"

inline std::string var_tex(const std::string& name){
    if (name.size() == 1) return name;
    return "\\mathit{" + name + "}";
}

inline bool is_one(const Fraction& f) { return f.n == 1 && f.d ==1; };

inline std::string coef_var_tex(const Fraction& c, const std::string& name) {
    if (is_one(c)) return var_tex(name);
    if (c.n == -1 && c.d == 1) return "-" + var_tex(name);
    return c.latex() + var_tex(name);
}

inline std::string equation_tex(const std::vector<Fraction>& row,
                                const std::vector<std::string>& vars) {
    const int n = (int)vars.size();
    std::string out;
    for (int j = 0; j < n; ++j) {
        if (row[j].is_zero()) continue;
        const bool neg = row[j].n < 0;
        const Fraction mag = neg ? -row[j] : row[j];   // |coefficient|
        if (out.empty()) { if (neg) out += "-"; }       // first term: only a "-" if needed
        else out += neg ? " - " : " + ";                // later terms: " + " or " - "
        out += coef_var_tex(mag, vars[j]);
    }
    if (out.empty()) out = "0";
    return out + " = " + row[n].latex();
}

inline std::string matrix_tex(const Matrix& m, bool augmented){
    if (m.empty()) return "";
    const size_t cols = m[0].size();
    std::string spec;
    for (size_t j = 0; j < cols; ++j) {
        if (augmented && j + 1 == cols) spec += "|";
        spec += "c";
    }
    std::string s = "\\left[\\begin{array}{" + spec + "}";
    for (size_t r = 0; r < m.size(); ++r) {
        for (size_t c = 0; c < cols; ++c) {
            if (c > 0) s += " & ";
            s += m[r][c].latex();
        }
        if (r + 1 < m.size()) s += " \\\\ ";
    }
    return s + "\\end{array}\\right]";
}

inline std::string column_tex(const std::vector<Fraction>& v) {
    std::string s = "\\begin{bmatrix} ";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i > 0) s += " \\\\ ";
        s += v[i].latex();
    }
    return s + " \\end{bmatrix}";
}

inline std::string row_op_tex(int target, const Fraction& m, int pivot) {
    const bool neg = m.n < 0;
    const Fraction mag = neg ? -m : m;
    const std::string t = std::to_string(target + 1);
    const std::string p = std::to_string(pivot + 1);
    return "R_{" + t + "} \\leftarrow R_{" + t + "} " + (neg ? "+ " : "- ") +
           (is_one(mag) ? "" : mag.latex()) + "R_{" + p + "}";
}

inline std::string combo_tex(const Fraction& rhs, const std::vector<Fraction>& row,
                             int from, int to, const std::vector<Fraction>& vals) {
    std::string s = rhs.latex();
    for (int j = from; j < to; ++j) {
        if (row[j].is_zero()) continue;
        s += " - (" + row[j].latex() + ")\\cdot(" + vals[j].latex() + ")";
    }
    return s;
}

inline std::string back_line_tex(const std::vector<Fraction>& urow, const Fraction& rhs,
                                 int i, const std::vector<Fraction>& x,
                                 const std::vector<std::string>& vars) {
    const int n = (int)x.size();
    Fraction sum = rhs;
    for (int j = i + 1; j < n; ++j) sum = sum - urow[j] * x[j];
    const std::string combo = combo_tex(rhs, urow, i + 1, n, x);
    std::string s = coef_var_tex(urow[i], vars[i]) + " = " + combo;
    if (combo != sum.latex()) s += " = " + sum.latex();   // only when there was something to compute
    return s + " \\;\\Rightarrow\\; " + var_tex(vars[i]) + " = " + x[i].latex();
}

inline std::string forward_line_tex(const std::vector<Fraction>& lrow, const Fraction& rhs,
                                    int i, const std::vector<Fraction>& c) {
    Fraction sum = rhs;
    for (int j = 0; j < i; ++j) sum = sum - lrow[j] * c[j];
    const std::string combo = combo_tex(rhs, lrow, 0, i, c);
    std::string s = "c_{" + std::to_string(i + 1) + "} = " + combo;
    if (combo != sum.latex()) s += " = " + sum.latex();
    return s;
}

class LatexDoc {
public:
    void heading(const std::string& t) { add("\\textbf{" + t + "}", 16); }
    void text(const std::string& t)    { add("\\text{" + t + "}", 6); }
    void math(const std::string& m)    { add(m, 6); }
    std::string str() const {
        return "\\[\n\\begin{aligned}\n" + body_ +
            "\n\\end{aligned}\n\\]";
    }

private:
    void add(const std::string& line, int gap_pt) {
        if (!body_.empty()) body_ += " \\\\[" + std::to_string(gap_pt) + "pt]\n";
        body_ += "& " + line;
    }
    std::string body_;
};