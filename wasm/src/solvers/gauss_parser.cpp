//   "x + 2y = 200\n2x + 3y = 120"   ->   vars = {x, y}
//                                         aug  = [ 1 2 | 200 ]
//                                                [ 2 3 | 120 ]
//
#include "solvers/gauss_parser.h"

#include <cctype>   // isdigit, isalpha, isalnum, isspace
#include <string>
#include <vector>

namespace {

    bool fail(ParsedSystem& ps, SolveStatus status, const std::string& msg) {
        ps.status = status;
        ps.message = msg;
        return false;
    }

    std::string at_line(int line_no) { return "Line " + std::to_string(line_no) + ": "; }

    bool is_blank(const std::string& s) {
        for (char ch : s)
            if (!std::isspace((unsigned char)ch)) return false;
        return true;
    }

    // "12" or "2.5" -> exact Fraction (2.5 becomes 25/10 = 5/2).
    bool parse_number(const std::string& tok, Fraction& out) {
        const size_t dot = tok.find('.');
        const std::string int_part  = (dot == std::string::npos) ? tok : tok.substr(0, dot);
        const std::string frac_part = (dot == std::string::npos) ? "" : tok.substr(dot + 1);

        if (int_part.empty()) return false;                                  // ".5" or "."
        if (dot != std::string::npos && frac_part.empty()) return false;     // "1."
        if (frac_part.find('.') != std::string::npos) return false;          // "2..5", "1.2.3"
        if (int_part.size() + frac_part.size() > 15) return false;           // too long

        __int128 num = 0, den = 1;
        for (char ch : int_part + frac_part) num = num * 10 + (ch - '0');    // digits as one integer
        for (size_t k = 0; k < frac_part.size(); ++k) den *= 10;             // 10^(digits after the dot)
        out = Fraction::make(num, den);                                      // make() reduces 25/10 -> 5/2
        return true;
    }

    int find_or_add_var(ParsedSystem& ps, const std::string& name) {
        for (size_t i = 0; i < ps.vars.size(); ++i)
            if (ps.vars[i] == name) return (int)i;
        if ((int)ps.vars.size() >= MAX_VARS) return -1;
        ps.vars.push_back(name);
        return (int)ps.vars.size() - 1;
    }

    bool parse_side(const std::string& s, int line_no, bool is_left, ParsedSystem& ps,
                    std::vector<Fraction>& coef, std::vector<bool>& seen, Fraction& rhs) {
        const char* side = is_left ? "left" : "right";
        size_t i = 0;
        const size_t n = s.size();
        auto skip_ws = [&]() { while (i < n && std::isspace((unsigned char)s[i])) ++i; };

        bool first = true;
        for (;;) {
            skip_ws();

            int sign = 1;
            if (!first) {
                if (i >= n) break;
                if (s[i] == '+')      sign = 1;
                else if (s[i] == '-') sign = -1;
                else return fail(ps, SOLVE_SYNTAX, at_line(line_no) + "expected '+' or '-' but found '" +
                                                std::string(1, s[i]) + "' on the " + side + " side");
                ++i;
                skip_ws();
            } else if (i < n && (s[i] == '+' || s[i] == '-')) {
                sign = (s[i] == '-') ? -1 : 1;
                ++i;
                skip_ws();
            }
            if (i >= n)
                return fail(ps, SOLVE_SYNTAX, at_line(line_no) + "expected a number or variable on the " +
                                            side + " side of '='");

            Fraction value(1);
            bool has_number = false;
            if (std::isdigit((unsigned char)s[i]) || s[i] == '.') {
                const size_t start = i;
                while (i < n && (std::isdigit((unsigned char)s[i]) || s[i] == '.')) ++i;
                const std::string tok = s.substr(start, i - start);
                if (!parse_number(tok, value))
                    return fail(ps, SOLVE_BAD_NUMBER, at_line(line_no) + "invalid number '" + tok +
                                                    "' (use digits like 12 or 2.5, max 15 digits)");
                has_number = true;
            }
            skip_ws();

            bool star = false;
            if (i < n && s[i] == '*') { star = true; ++i; skip_ws(); }

            std::string name;
            if (i < n && std::isalpha((unsigned char)s[i])) {
                const size_t start = i;
                while (i < n && (std::isalnum((unsigned char)s[i]) || s[i] == '_')) ++i;
                name = s.substr(start, i - start);
            }
            if (star && name.empty())
                return fail(ps, SOLVE_SYNTAX, at_line(line_no) + "expected a variable after '*'");
            if (!has_number && name.empty())                // nothing recognisable here
                return fail(ps, SOLVE_SYNTAX, at_line(line_no) + "unexpected character '" +
                                            std::string(1, s[i]) + "' on the " + side + " side");

            const Fraction term = (sign < 0) ? -value : value;
            if (name.empty()) {
                rhs = is_left ? rhs - term : rhs + term;
            } else {
                const int v = find_or_add_var(ps, name);
                if (v < 0)
                    return fail(ps, SOLVE_TOO_MANY_VARS, at_line(line_no) + "more than " +
                                                        std::to_string(MAX_VARS) + " variables (the limit is " +
                                                        std::to_string(MAX_VARS) + ")");
                if ((int)coef.size() <= v) { coef.resize(v + 1); seen.resize(v + 1, false); }
                if (seen[v])
                    return fail(ps, SOLVE_DUP_VAR, at_line(line_no) + "variable '" + name +
                                                "' appears more than once in this equation");
                seen[v] = true;
                coef[v] = is_left ? term : -term;
            }
            first = false;
        }
        return true;
    }

    bool parse_equation(const std::string& line, int line_no, ParsedSystem& ps,
                        std::vector<std::vector<Fraction>>& rows, std::vector<Fraction>& rhs) {
        const size_t eq = line.find('=');
        if (eq == std::string::npos)
            return fail(ps, SOLVE_SYNTAX, at_line(line_no) + "missing '=' (every line must be an equation)");
        if (line.find('=', eq + 1) != std::string::npos)
            return fail(ps, SOLVE_SYNTAX, at_line(line_no) + "more than one '=' found");

        std::vector<Fraction> coef;
        std::vector<bool> seen;
        Fraction b;

        if (!parse_side(line.substr(0, eq),  line_no, true,  ps, coef, seen, b)) return false;
        if (!parse_side(line.substr(eq + 1), line_no, false, ps, coef, seen, b)) return false;

        rows.push_back(coef);
        rhs.push_back(b);
        return true;
    }

}  // namespace

ParsedSystem parse_system(const std::string& text) {
    ParsedSystem ps;                                    // status defaults to SOLVE_OK
    frac_detail::overflow = false;                      // start every solve with a clean flag

    std::vector<std::vector<Fraction>> rows;            // coefficients
    std::vector<Fraction> rhs;

    // ---- split the text into lines at '\n' (a '\r' left over from Windows is just whitespace) ----
    size_t pos = 0;
    int line_no = 0;
    while (pos <= text.size()) {
        size_t end = text.find('\n', pos);
        if (end == std::string::npos) end = text.size();
        const std::string line = text.substr(pos, end - pos);
        pos = end + 1;
        ++line_no;
        if (is_blank(line)) continue;
        if (!parse_equation(line, line_no, ps, rows, rhs)) return ps;
    }

    if (rows.empty()) {
        ps.status = SOLVE_EMPTY;
        ps.message = "No equations found. Type one equation per line, e.g. x + 2y = 200";
        return ps;
    }

    const int num_eq = (int)rows.size();
    const int num_vars = (int)ps.vars.size();
    if (num_eq != num_vars) {
        ps.status = SOLVE_NOT_SQUARE;
        ps.message = "Found " + std::to_string(num_eq) + " equation(s) but " + std::to_string(num_vars) +
                     " variable(s). A unique solution needs the same number of each.";
        return ps;
    }

    // ---- build the augmented matrix: pad short rows with 0, then append the right-hand side ----
    for (int r = 0; r < num_eq; ++r) {
        rows[r].resize(num_vars);                       // missing variables get coefficient 0
        rows[r].push_back(rhs[r]);
        ps.aug.push_back(rows[r]);
    }

    if (frac_detail::overflow) {
        ps.status = SOLVE_OVERFLOW;
        ps.message = "A number in the input is too large.";
        return ps;
    }
    ps.status = SOLVE_OK;
    return ps;
}