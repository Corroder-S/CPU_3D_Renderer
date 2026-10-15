#include "Maths/Mat4Batch.h"
#include <xmmintrin.h>
#include <stdexcept>


namespace Maths::SSE
{
    void TransformPointBatch(const Matrix4x4<>& transform, const Vec3<>* points, Vec3<>* outPoints, std::size_t count)
    {
        if (count > 0 &&
        (transform.values[3][0] != 0.0f || transform.values[3][1] != 0.0f ||
        transform.values[3][2] != 0.0f || transform.values[3][3] != 1.0f))
        {
            throw std::domain_error("Transform requires an affine matrix");
        }

        const Matrix4x4<> trans = transform.Transpose();
        __m128 pX = _mm_loadu_ps(trans.values[0]);
        __m128 pY = _mm_loadu_ps(trans.values[1]);
        __m128 pZ = _mm_loadu_ps(trans.values[2]);
        __m128 pW = _mm_loadu_ps(trans.values[3]);
        for (std::size_t i = 0; i < count; ++i)
        {
            __m128 bx = _mm_mul_ps(pX, _mm_set1_ps(points[i].x));
            __m128 by = _mm_mul_ps(pY, _mm_set1_ps(points[i].y));
            __m128 bz = _mm_mul_ps(pZ, _mm_set1_ps(points[i].z));

            bx = _mm_add_ps(bx, by);
            bx = _mm_add_ps(bx, bz);
            bx = _mm_add_ps(bx, pW);

            float tmp[4];
            _mm_storeu_ps(tmp, bx);
            outPoints[i] = {tmp[0], tmp[1], tmp[2]};
        }
    }

    void MultiplyMatrixBatch(const Matrix4x4<>* a, const Matrix4x4<>* b, Matrix4x4<>* out, size_t count)
    {
        for (size_t i = 0; i < count; ++i)
        {
            const __m128 b0 = _mm_loadu_ps(b[i].values[0]);
            const __m128 b1 = _mm_loadu_ps(b[i].values[1]);
            const __m128 b2 = _mm_loadu_ps(b[i].values[2]);
            const __m128 b3 = _mm_loadu_ps(b[i].values[3]);

            for (int r = 0; r < 4; ++r)
            {
                __m128 row = _mm_mul_ps(_mm_set1_ps(a[i].values[r][0]), b0);
                row = _mm_add_ps(row, _mm_mul_ps(_mm_set1_ps(a[i].values[r][1]), b1));
                row = _mm_add_ps(row, _mm_mul_ps(_mm_set1_ps(a[i].values[r][2]), b2));
                row = _mm_add_ps(row, _mm_mul_ps(_mm_set1_ps(a[i].values[r][3]), b3));
                _mm_storeu_ps(out[i].values[r], row);
            }
        }
    }
}

