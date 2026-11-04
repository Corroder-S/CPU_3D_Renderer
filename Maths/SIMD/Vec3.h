#pragma once
#include "Maths/Vec3.h"


namespace Maths::SIMD
{


    inline __m256 Dot8(const Vec3<__m256>& a, const Vec3<__m256>& b)
    {
        __m256 dot = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(a.x, b.x), _mm256_mul_ps(a.y, b.y)), _mm256_mul_ps(a.z, b.z));
        return dot;
    }


    inline Vec3<__m256> Normalize8(const Vec3<__m256>& v)
    {
        __m256 magSq = Dot8(v, v);
        __m256 isZeroMask = _mm256_cmp_ps(magSq, _mm256_set1_ps(1e-8f), _CMP_LT_OS);
        __m256 safeMagSq = _mm256_blendv_ps(magSq, _mm256_set1_ps(1.0f), isZeroMask);

        __m256 inv = _mm256_rsqrt_ps(safeMagSq);
        __m256 half = _mm256_set1_ps(0.5f), threeHalf = _mm256_set1_ps(1.5f);
        inv = _mm256_mul_ps(inv, _mm256_sub_ps(threeHalf,
            _mm256_mul_ps(_mm256_mul_ps(half, safeMagSq), _mm256_mul_ps(inv, inv))));

        __m256 zero = _mm256_setzero_ps();
        return {
            _mm256_blendv_ps(_mm256_mul_ps(v.x, inv), zero, isZeroMask),
            _mm256_blendv_ps(_mm256_mul_ps(v.y, inv), zero, isZeroMask),
            _mm256_blendv_ps(_mm256_mul_ps(v.z, inv), zero, isZeroMask)
        };
    }
}
