#pragma once
#include <string>

namespace frac_detail {
    inline bool overflow = false;
}

struct Fraction {
    long long n = 0;   // numerator
    long long d = 1;   // denominator, always > 0

    Fraction() {}
    Fraction(long long v) : n(v), d(1) {}

    static Fraction make(__int128 a, __int128 b) {
        if (b < 0) { a = -a; b = -b; }
        __int128 x = a < 0 ? -a : a;
        __int128 y = b;
        while (y != 0) { __int128 t = x % y; x = y; y = t; }   // Euclid's gcd
        if (x > 1) { a /= x; b /= x; }

        const __int128 LIMIT = 4000000000000000000LL;
        Fraction f;
        if (a > LIMIT || a < -LIMIT || b > LIMIT) {
            frac_detail::overflow = true;
            return f;
        }
        f.n = (long long)a;
        f.d = (long long)b;
        return f;
    }

    bool is_zero() const { return n == 0; }

    Fraction operator-() const { return make(-(__int128)n, d); }
    Fraction operator+(const Fraction& o) const {
        return make((__int128)n * o.d + (__int128)o.n * d, (__int128)d * o.d);
    }
    Fraction operator-(const Fraction& o) const {
        return make((__int128)n * o.d - (__int128)o.n * d, (__int128)d * o.d);
    }
    Fraction operator*(const Fraction& o) const {
        return make((__int128)n * o.n, (__int128)d * o.d);
    }
    Fraction operator/(const Fraction& o) const {
        return make((__int128)n * o.d, (__int128)d * o.n);
    }

    std::string latex() const {
        if (d == 1) return std::to_string(n);
        std::string sign = n < 0 ? "-" : "";
        long long m = n < 0 ? -n : n;
        return sign + "\\frac{" + std::to_string(m) + "}{" + std::to_string(d) + "}";
    }
};