#include "lu/lu_core.h"
#include <utility>


LUResult lu_decompose(const Matrix& A){
    const int n = (int)A.size();
    LUResult res;

    Matrix w = A;

    res.perm.resize(n);
    for (int i = 0; i < n; i++) res.perm[i] = i;

    for (int c = 0; c < n; c++){
        int p = -1;
        for(int r = c; r < n; r++)
            if (!w[r][c].is_zero()) { p = r; break; }
        if (p < 0) { res.ok = false; res.bad_col = c; return res; }

        if (p != c) {
            std::swap(w[p], w[c]);
            std::swap(res.perm[p], res.perm[c]);
            res.steps.push_back({true, c, p, Fraction()});
        }

        for (int r = c + 1; r < n; ++r) {
            const Fraction m = w[r][c] / w[c][c];
            w[r][c] = m;                                    // this slot is now a piece of L
            if (m.is_zero()) continue;
            for (int k = c + 1; k < n; ++k) w[r][k] = w[r][k] - m * w[c][k];
            res.steps.push_back({false, r, c, m});
        }
    }

    res.L.assign(n, std::vector<Fraction>(n));
    res.U.assign(n, std::vector<Fraction>(n));
    for(int i = 0; i < n; i++){
        for (int j = 0; j < n; ++j) {
            if (j < i) res.L[i][j] = w[i][j];
            else       res.U[i][j] = w[i][j];
        }
        res.L[i][i] = Fraction(1); 
    }
    res.ok = true;
    return res;
}

std::vector<Fraction> forward_substitute(const Matrix& L, const std::vector<Fraction>& b) {
    const int n = (int)b.size();
    std::vector<Fraction> c(n);
    for (int i = 0; i < n; ++i) {
        Fraction sum = b[i];
        for (int j = 0; j < i; ++j) sum = sum - L[i][j] * c[j];
        c[i] = sum;
    }
    return c;
}

std::vector<Fraction> back_substitute(const Matrix& U, const std::vector<Fraction>& c) {
    const int n = (int)c.size();
    std::vector<Fraction> x(n);
    for (int i = n - 1; i >= 0; --i) {
        Fraction sum = c[i];
        for (int j = i + 1; j < n; ++j) sum = sum - U[i][j] * x[j];
        x[i] = sum / U[i][i];
    }
    return x;
}