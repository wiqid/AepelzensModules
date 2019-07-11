#pragma once
#include <rack.hpp>

using namespace rack;

/* the original modulo does not deal with negative numbers correctly
   For example -1%12 should be 11, but it is -1*/
inline int modN(int k, int n) {
    return ((k %= n) < 0) ? k+n : k;
}

/* modified version of ceil that works with negative values (example: -2.3 becomes -3) */
inline int ceilN(float x) {
    return (x < 0) ? (int)floor(x) : (int)ceil(x);
}

////////// fast math //////////

template <typename T>
static T tanh_pade(T x) {
    // Padé approximant of tanh
    x = simd::clamp(x, -5.f, 5.f);
    return (T(x * (135135 + simd::pow(x, 2) * (17325 + simd::pow(x, 2) * (378 + simd::pow(x, 2))))))
         / (T(135135 + simd::pow(x, 2) * (62370 + simd::pow(x, 2) * (3150 + 28 * simd::pow(x, 2)))));
}
