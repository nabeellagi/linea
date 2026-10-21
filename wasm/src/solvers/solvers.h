#pragma once
#include "common/common.h"

enum SolveStatus {
    SOLVE_OK = 0,
    SOLVE_EMPTY = 1,
    SOLVE_SYNTAX = 2,
    SOLVE_BAD_NUMBER = 3,
    SOLVE_DUP_VAR = 4,
    SOLVE_TOO_MANY_VARS = 5,
    SOLVE_NOT_SQUARE = 6,
    SOLVE_SINGULAR = 7,
    SOLVE_OVERFLOW = 8,
    SOLVE_NULL = 9
};

MAT_API const char* solve_gauss(const char* input);
MAT_API int solve_gauss_status();

MAT_API const char* solve_lu(const char* input);
MAT_API int solve_lu_status();