#pragma once
#include <immintrin.h>


inline __m256 log2_ps(__m256 x)
{
    __m256i bits = _mm256_castps_si256(x);
    __m256i expi = _mm256_and_si256(_mm256_srli_epi32(bits, 23), _mm256_set1_epi32(0xFF));
    expi = _mm256_sub_epi32(expi, _mm256_set1_epi32(127));
    __m256 e = _mm256_cvtepi32_ps(expi);

    __m256i mb = _mm256_and_si256(bits, _mm256_set1_epi32(0x007FFFFF));
    mb = _mm256_or_si256(mb, _mm256_set1_epi32(0x3F800000));
    __m256 m = _mm256_castsi256_ps(mb);

    const __m256 one = _mm256_set1_ps(1.0f);
    __m256 s = _mm256_div_ps(_mm256_sub_ps(m, one), _mm256_add_ps(m, one));
    __m256 s2 = _mm256_mul_ps(s, s);

    __m256 p = _mm256_set1_ps(1.0f / 7.0f);
    p = _mm256_fmadd_ps(p, s2, _mm256_set1_ps(1.0f / 5.0f));
    p = _mm256_fmadd_ps(p, s2, _mm256_set1_ps(1.0f / 3.0f));
    p = _mm256_fmadd_ps(p, s2, one);
    p = _mm256_mul_ps(p, s);
    p = _mm256_mul_ps(p, _mm256_set1_ps(2.8853901f));

    return _mm256_add_ps(e, p);
}

inline __m256 exp2_ps(__m256 x)
{
    x = _mm256_max_ps(x, _mm256_set1_ps(-126.0f));
    x = _mm256_min_ps(x, _mm256_set1_ps(126.0f));

    __m256 n = _mm256_floor_ps(x);
    __m256 f = _mm256_sub_ps(x, n);

    __m256i ni = _mm256_cvtps_epi32(n);
    ni = _mm256_add_epi32(ni, _mm256_set1_epi32(127));
    ni = _mm256_slli_epi32(ni, 23);
    __m256 pow2n = _mm256_castsi256_ps(ni);

    __m256 p = _mm256_set1_ps(0.0555041f);
    p = _mm256_fmadd_ps(p, f, _mm256_set1_ps(0.2402265f));
    p = _mm256_fmadd_ps(p, f, _mm256_set1_ps(0.6931472f));
    p = _mm256_fmadd_ps(p, f, _mm256_set1_ps(1.0f));

    return _mm256_mul_ps(pow2n, p);
}