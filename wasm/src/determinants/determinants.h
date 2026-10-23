#pragma once

#include "common/common.h"
#include "common/matrix.h"
#include "lu/lu_core.h"

enum DetStatus {
    DET_OK = 0,
    DET_EMPTY = 1,
    DET_BAD_NUMBER = 2,
    DET_BAD_ROW = 3,
    DET_NOT_SQUARE = 4,
    DET_TOO_LARGE = 5,
    DET_OVERFLOW = 6,
    DET_NULL = 7,
    DET_METHOD_MISMATCH = 8
};

struct DetLUResult {
    bool valid = false;
    bool singular = false;
    int bad_col = -1;
    int row_swaps = 0;
    Fraction value;
    LUResult lu;
};

DetLUResult det_lu(const Matrix& A);
Fraction det_mc(const Matrix& A);

MAT_API const char* det(const char* input);
MAT_API int det_status();