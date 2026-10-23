#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
 
#include "determinants/determinants.h"
 
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
           "\\section*{Determinant Test}\n"
           "\\textbf{Input matrix:}\\par\n"
           "\\begin{verbatim}\n"
        << test.input <<
           "\n\\end{verbatim}\n"
           "\\section*{Determinant calculation}\n";
 
    const char* result = det(test.input);
    const int status = det_status();
    if (status != DET_OK || result == nullptr || *result == '\0') {
        out << "\\textbf{Calculation failed. Status: " << status << ".}\\par\n";
        out.close();
        std::cerr << "[FAIL] " << path << '\n';
        return false;
    }
 
    // det() already returns a LaTeX aligned environment, ready for KaTeX or MathJax.
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
            "0 2 1\n1 1 1\n2 5 -1",
            "determinant_3x3.txt"
        },
        {
            "1 2\n2 4",
            "determinant_singular.txt"
        }
    };
 
    bool all_ok = true;
    for (const TestCase& test : cases) {
        const bool ok = write_case(test);
        all_ok = ok && all_ok;
    }
    return all_ok ? 0 : 1;
}

