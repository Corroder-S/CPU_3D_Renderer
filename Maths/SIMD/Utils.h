#pragma once
#include <immintrin.h>


inline __m256 log2_ps(__m256 x)
{
    __m256i bits = _mm256_castps_si256(x);
    __m256i exp  = _mm256_srli_epi32(bits, 23);

    exp = _mm256_sub_epi32(exp, _mm256_set1_epi32(127));
    __m256 e     = _mm256_cvtepi32_ps(exp);

    __m256i mantissa_bits = _mm256_and_si256(bits, _mm256_set1_epi32(0x007FFFFF));
    mantissa_bits = _mm256_or_si256(mantissa_bits, _mm256_set1_epi32(0x3F800000));
    __m256 m = _mm256_castsi256_ps(mantissa_bits);

    __m256 t = _mm256_sub_ps(m, _mm256_set1_ps(1.0f));
    __m256 a = _mm256_set1_ps(1.4426950f);
    __m256 b = _mm256_set1_ps(-0.7213475f);
    __m256 c = _mm256_set1_ps(0.4808983f);

    c = _mm256_fmadd_ps(c, t, b);
    c = _mm256_fmadd_ps(c, t, a);
    c = _mm256_mul_ps(c, t);

    return _mm256_add_ps(e, c);
}

inline __m256 exp2_ps(__m256 x)
{
    __m256 n = _mm256_floor_ps(x);
    __m256 f = _mm256_sub_ps(x, n);

    __m256i ni = _mm256_cvtps_epi32(n);
    ni = _mm256_add_epi32(ni, _mm256_set1_epi32(127));
    ni = _mm256_slli_epi32(ni, 23);
    __m256 pow2n = _mm256_castsi256_ps(ni);

    __m256 a = _mm256_set1_ps(1.0f);
    __m256 b = _mm256_set1_ps(0.6931472f);
    __m256 c = _mm256_set1_ps(0.2402265f);
    __m256 pow2f = _mm256_set1_ps(0.0555041f);

    pow2f = _mm256_fmadd_ps(pow2f, f, c);
    pow2f = _mm256_fmadd_ps(pow2f, f, b);
    pow2f = _mm256_fmadd_ps(pow2f, f, a);

    return _mm256_mul_ps(pow2n, pow2f);

}