#pragma once

#include "common/common.h"

enum RegressionStatus {
    REG_OK = 0,
    REG_EMPTY = 1,
    REG_SYNTAX = 2,
    REG_BAD_NUMBER = 3,
    REG_TOO_FEW_POINTS = 4,
    REG_TOO_MANY_POINTS = 5,
    REG_SINGULAR_X = 6,
    REG_OVERFLOW = 7,
    REG_NULL = 8
};

MAT_API const char* linear_regression(const char* input);
MAT_API int linear_regression_status();