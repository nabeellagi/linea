#include <vector>
#include "common/matrix.h"

struct LUStep{
    bool is_swap;

    int row;
    int other;
    Fraction mult;
};

struct LUResult{
    bool ok = false;
    int bad_col = -1;
    Matrix L, U;
    std::vector<int> perm;
    std::vector<LUStep> steps;
};

LUResult lu_decompose(const Matrix& A);
std::vector<Fraction> forward_substitute(const Matrix& L, const std::vector<Fraction>& b);
std::vector<Fraction> back_substitute(const Matrix& U, const std::vector<Fraction>& c);