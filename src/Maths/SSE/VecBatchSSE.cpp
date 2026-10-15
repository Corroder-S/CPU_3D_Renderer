#include "Maths/Vec3Batch.h"

namespace Maths
{
    namespace SSE
    {
        void DotProductBatch_AOS(const Vec3f* a, const Vec3f* b, float* result, size_t count)
        {
            for (size_t i = 0; i < count; ++i)
            {
                __m128 vA = _mm_loadu_ps(&a[i].x);
                __m128 vB = _mm_loadu_ps(&b[i].x);

                __m128 vMul = _mm_mul_ps(vA, vB);

                __m128 vY = _mm_shuffle_ps(vMul, vMul, _MM_SHUFFLE(1, 1, 1, 1));
                __m128 vZ = _mm_shuffle_ps(vMul, vMul, _MM_SHUFFLE(2, 2, 2, 2));

                __m128 vSum = _mm_add_ps(vMul, _mm_add_ps(vY, vZ));

                result[i] = _mm_cvtss_f32(vSum);
            }
        }

        void DotProductBatch_SOA(const Vec3SoA& a, const Vec3SoA& b, float* result, size_t count)
        {
            size_t i = 0;
            const size_t simdWidth = 4;

            for (; i + simdWidth <= count; i += simdWidth)
            {
                __m128 ax = _mm_loadu_ps(&a.x[i]);
                __m128 ay = _mm_loadu_ps(&a.y[i]);
                __m128 az = _mm_loadu_ps(&a.z[i]);

                __m128 bx = _mm_loadu_ps(&b.x[i]);
                __m128 by = _mm_loadu_ps(&b.y[i]);
                __m128 bz = _mm_loadu_ps(&b.z[i]);

                ax = _mm_mul_ps(ax, bx);
                ay = _mm_mul_ps(ay, by);
                az = _mm_mul_ps(az, bz);

                __m128 dot = _mm_add_ps(_mm_add_ps(ax, ay), az);

                _mm_storeu_ps(&result[i], dot);
            }

            for (; i < count; ++i)
            {
                result[i] = a.x[i] * b.x[i] + a.y[i] * b.y[i] + a.z[i] * b.z[i];
            }
        }

        void NormalizeBatch_AOS(const Vec3f* input, Vec3f* output, size_t count)
        {
            size_t i = 0;
            const size_t width = 4;

            for (; i + width <= count; i += width)
            {
                __m128 vx = _mm_set_ps(input[i + 3].x, input[i + 2].x, input[i + 1].x, input[i + 0].x);
                __m128 vy = _mm_set_ps(input[i + 3].y, input[i + 2].y, input[i + 1].y, input[i + 0].y);
                __m128 vz = _mm_set_ps(input[i + 3].z, input[i + 2].z, input[i + 1].z, input[i + 0].z);

                __m128 magSq = _mm_add_ps(_mm_add_ps(_mm_mul_ps(vx, vx), _mm_mul_ps(vy, vy)), _mm_mul_ps(vz, vz));

                __m128 zeroMask = _mm_cmpeq_ps(magSq, _mm_setzero_ps());

                __m128 safeMagSq = _mm_or_ps(_mm_andnot_ps(zeroMask, magSq), _mm_and_ps(zeroMask, _mm_set1_ps(1.0f)));

                __m128 invMag = _mm_div_ps(_mm_set1_ps(1.0f), _mm_sqrt_ps(safeMagSq));

                __m128 rx = _mm_mul_ps(vx, invMag);
                __m128 ry = _mm_mul_ps(vy, invMag);
                __m128 rz = _mm_mul_ps(vz, invMag);

                rx = _mm_andnot_ps(zeroMask, rx);
                ry = _mm_andnot_ps(zeroMask, ry);
                rz = _mm_andnot_ps(zeroMask, rz);

                alignas(16) float xs[4], ys[4], zs[4];
                _mm_store_ps(xs, rx);
                _mm_store_ps(ys, ry);
                _mm_store_ps(zs, rz);

                for (size_t lane = 0; lane < width; ++lane)
                {
                    output[i + lane] = { xs[lane], ys[lane], zs[lane] };
                }
            }

            for (; i < count; ++i)
            {
                const Vec3f& v = input[i];
                const bool finite = std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
                const float magSq = v.x * v.x + v.y * v.y + v.z * v.z;

                if (!finite || magSq == 0.0f)
                {
                    output[i] = Vec3f::Zero;
                }
                else
                {
                    const float invMag = 1.0f / std::sqrt(magSq);
                    output[i] = { v.x * invMag, v.y * invMag, v.z * invMag };
                }
            }
        }
        void CrossProductBatch(const Vec3SoA& a, const Vec3SoA& b, Vec3f* result, size_t count)
        {
            size_t i = 0;
            for (; i + 4 <= count; i += 4)
            {
                __m128 ax = _mm_loadu_ps(&a.x[i]);
                __m128 ay = _mm_loadu_ps(&a.y[i]);
                __m128 az = _mm_loadu_ps(&a.z[i]);

                __m128 bx = _mm_loadu_ps(&b.x[i]);
                __m128 by = _mm_loadu_ps(&b.y[i]);
                __m128 bz = _mm_loadu_ps(&b.z[i]);

                __m128 cx = _mm_sub_ps(_mm_mul_ps(ay, bz), _mm_mul_ps(az, by));
                __m128 cy = _mm_sub_ps(_mm_mul_ps(az, bx), _mm_mul_ps(ax, bz));
                __m128 cz = _mm_sub_ps(_mm_mul_ps(ax, by), _mm_mul_ps(ay, bx));

                alignas(16) float tmp[4];
                _mm_store_ps(tmp, cx);
                for (size_t j = 0; j < 4; ++j) result[i+j].x = tmp[j];
                _mm_store_ps(tmp, cy);
                for (size_t j = 0; j < 4; ++j) result[i+j].y = tmp[j];
                _mm_store_ps(tmp, cz);
                for (size_t j = 0; j < 4; ++j) result[i+j].z = tmp[j];
            }
            for (; i < count; ++i)
            {
                result[i] = {
                    a.y[i] * b.z[i] - a.z[i] * b.y[i],
                    a.z[i] * b.x[i] - a.x[i] * b.z[i],
                    a.x[i] * b.y[i] - a.y[i] * b.x[i]
                };
            }
        }

    }
}
