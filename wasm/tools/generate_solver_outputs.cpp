
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "solvers/solvers.h"

struct TestCase {
    const char* input;
    const char* filename;
};

static bool write_case(const TestCase& test) {
    std::filesystem::create_directories("tests/out");

    const std::string path =
        std::string("tests/out/") + test.filename;

    std::ofstream out(path);
    if (!out) {
        std::cerr << "Cannot write " << path << '\n';
        return false;
    }

    out << "\\documentclass{article}\n"
           "\\usepackage{amsmath,amssymb}\n"
           "\\usepackage[margin=1in]{geometry}\n"
           "\\begin{document}\n"
           "\\section*{Linear Equation Solver Test}\n"
           "\\textbf{Input system:}\\par\n"
           "\\begin{verbatim}\n"
        << test.input <<
           "\n\\end{verbatim}\n";

    struct Method {
        const char* name;
        const char* (*solve)(const char*);
        int (*status)();
    };

    const Method methods[] = {
        {"Gaussian Elimination",
         solve_gauss, solve_gauss_status},

        {"LU Factorization",
         solve_lu, solve_lu_status}
    };

    bool success = true;

    for (const Method& method : methods) {
        const char* result = method.solve(test.input);
        const int status = method.status();

        out << "\\section*{" << method.name << "}\n";

        if (status != 0 || result == nullptr ||
            *result == '\0') {
            out << "\\textbf{Solver failed. Status: "
                << status << ".}\\par\n";

            success = false;
        } else {
            out << result << "\n";
        }
    }

    out << "\\end{document}\n";
    out.close();

    if (!out) {
        std::cerr << "Error writing " << path << '\n';
        return false;
    }

    std::cout << (success ? "[PASS] " : "[FAIL] ")
              << path << '\n';

    return success;
}

int main() {
    const TestCase cases[] = {
        {
            "x + 2y = 200\n2x + 3y = 120",
            "linear_equation_1.txt"
        },
        {
            "2x + y = 1\nx + 3y = 1",
            "linear_equation_2.txt"
        },
        {
            "x + y + z = 6\n"
            "2y + 5z = -4\n"
            "2x + 5y - z = 27",
            "linear_equation_3.txt"
        }
    };

    bool all_ok = true;

    for (const TestCase& test : cases) {
        const bool ok = write_case(test);
        all_ok = ok && all_ok;
    }

    return all_ok ? 0 : 1;
}
