#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "regression/linear_regression.h"

struct TestCase {
    const char* input;
    const char* filename;
};

static bool write_case(const TestCase& test) {

    std::filesystem::create_directories("tests/out");

    const std::string path = std::string("tests/out/") + test.filename;

    std::ofstream out(path);

    if (!out) {
        std::cerr << "Cannot write " << path << '\n';
        return false;
    }

    out << "\\documentclass{article}\n"
           "\\usepackage{amsmath,amssymb}\n"
           "\\usepackage[margin=1in]{geometry}\n"
           "\\begin{document}\n"
           "\\section*{Linear Regression Test}\n"
           "\\textbf{Input coordinate pairs:}\\par\n"
           "\\begin{verbatim}\n"
        << test.input <<
           "\n\\end{verbatim}\n"
           "\\section*{Step-by-step least-squares solution}\n";

    // linear_regression() returns the same aligned LaTeX body used by KaTeX/MathJax.
    // The wrapper here makes a complete .tex document for manual review or PDF export.

    const char* result = linear_regression(test.input);
    const int status = linear_regression_status();

    if (status != REG_OK || result == nullptr || *result == '\0') {
        out << "\\textbf{Calculation failed. Status: " << status << ".}\\par\n\\end{document}\n";
        out.close();
        std::cerr << "[FAIL] " << path << '\n';
        return false;
    }

    out << result << "\n\\end{document}\n";
    out.close();
    if (!out) {
        std::cerr << "Error writing " << path << '\n';
        return false;
    }
    std::cout << "[PASS] " << path << '\n';
    return true;
}

int main() {
    const TestCase cases[] = {
        {
            "(0, 1) (1, 2) (1, 3) (1.5, 4) (2, 5) (3, 6)",
            "linear_regression_example.txt"
        },
        {
            "(0, 2) (1, 2) (2, 2)",
            "linear_regression_constant_y.txt"
        }
    };

    bool all_ok = true;
    for (const TestCase& test : cases) {
        const bool ok = write_case(test);
        all_ok = ok && all_ok;
    }
    return all_ok ? 0 : 1;
}