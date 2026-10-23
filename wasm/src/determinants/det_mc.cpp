#include "determinants/determinants.h"

namespace{
    Fraction det_mc_impl(const Matrix& A){
        const int n = static_cast<int>(A.size());

        if(n == 1) return A[0][0];
        if (n == 2) {
            return A[0][0] * A[1][1] - A[0][1] * A[1][0];
        }

        Fraction total;

        for(int col = 0; col < n; col++){
            Matrix minor(n - 1, std::vector<Fraction>(n-1));

            for (int r = 1; r < n; ++r) {
                int minor_col = 0;
                for (int c = 0; c < n; c++) {
                    if (c == col) continue;
                    minor[r - 1][minor_col] = A[r][c];
                    minor_col++;
                }
            }

            const Fraction term = A[0][col] * det_mc_impl(minor);
            if (col % 2 == 0) total = total + term;  // +, -, +, - ...
            else               total = total - term;
        }
        return total;
    }

    bool is_square_matrix(const Matrix& A){
        if(A.empty()) return false;
        const size_t n = A.size();
        for (const auto& row : A)
            if (row.size() != n) return false;
        return true;
    }
}

Fraction det_mc(const Matrix& A) {
    frac_detail::overflow = false;
    
    if (!is_square_matrix(A)) return Fraction(0);
 
    return det_mc_impl(A);
}