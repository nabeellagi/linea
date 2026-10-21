#pragma once
#include <string>
#include <vector>
#include "common/fraction.h"
#include "solvers/solvers.h"

constexpr int MAX_VARS = 100;

struct ParsedSystem {
    SolveStatus status = SOLVE_OK;
    std::string message;
    std::vector<std::string> vars;
    std::vector<std::vector<Fraction>> aug;
};

ParsedSystem parse_system(const std::string& text);