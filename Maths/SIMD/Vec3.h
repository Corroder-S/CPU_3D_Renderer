#pragma once
#include "Maths/Vec3.h"


namespace Maths::SIMD
{
    inline Vec3<__m256> Normalize8(const Vec3<__m256>& v)
    {
        __m256 sqX = _mm256_mul_ps(v.x, v.x);
        __m256 sqY = _mm256_mul_ps(v.y, v.y);
        __m256 sqZ = _mm256_mul_ps(v.z, v.z);




        __m256 magSq = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(v.x, v.x), _mm256_mul_ps(v.y, v.y)), _mm256_mul_ps(v.z, v.z));
        __m256 inv = _mm256_rsqrt_ps(magSq);
        return { _mm256_mul_ps(v.x, inv), _mm256_mul_ps(v.y, inv), _mm256_mul_ps(v.z, inv) };
    }



    inline __m256 Dot8(const Vec3<__m256>& a, const Vec3<__m256>& b)
    {
        __m256 dot = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(a.x, b.x), _mm256_mul_ps(a.y, b.y)), _mm256_mul_ps(a.z, b.z));
        return dot;
    }
}
