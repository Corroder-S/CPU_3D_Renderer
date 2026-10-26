#pragma once
#include "Maths/Vec3.h"


namespace Maths::SIMD
{
    inline Vec3f Normalize(const Vec3f& v)
    {
        __m128 vec = _mm_set_ps(0.0f, v.z, v.y, v.x);
        __m128 sq  = _mm_mul_ps(vec, vec);
        __m128 shuf = _mm_shuffle_ps(sq, sq, _MM_SHUFFLE(2, 3, 0, 1));
        __m128 sums = _mm_add_ps(sq, shuf);
        shuf        = _mm_shuffle_ps(sums, sums, _MM_SHUFFLE(0, 1, 2, 3));
        __m128 magSq = _mm_add_ps(sums, shuf); // même valeur dans les 4 lanes
        __m128 inv        = _mm_rsqrt_ps(magSq);
        __m128 normalized = _mm_mul_ps(vec, inv);
        alignas(16) float out[4];
        _mm_store_ps(out, normalized);
        return Vec3f(out[0], out[1], out[2]);
    }


    inline float Dot(const Vec3f& a, const Vec3f& b)
    {
        __m128 vecA = _mm_set_ps(0.0f, a.z, a.y, a.x);
        __m128 vecB = _mm_set_ps(0.0f, b.z, b.y, b.x);
        __m128 prod = _mm_mul_ps(vecA, vecB);
        __m128 shuf = _mm_shuffle_ps(prod, prod, _MM_SHUFFLE(2, 3, 0, 1));
        __m128 sums = _mm_add_ps(prod, shuf);
        shuf        = _mm_shuffle_ps(sums, sums, _MM_SHUFFLE(0, 1, 2, 3));
        __m128 dot = _mm_add_ps(sums, shuf); // même valeur dans les 4 lanes
        alignas(16) float out[4];
        _mm_store_ps(out, dot);
        return out[0];
    }


}
