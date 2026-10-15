#include "Maths/Vec3Batch.h"

namespace Maths
{
    Vec3SoA ConvertToSOA(const Vec3f* input, size_t count)
    {
        Vec3SoA result;
        result.x.resize(count);
        result.y.resize(count);
        result.z.resize(count);

        for (size_t i = 0; i < count; ++i)
        {
            result.x[i] = input[i].x;
            result.y[i] = input[i].y;
            result.z[i] = input[i].z;
        }

        return result;
    }


    namespace Ref
    {
        void DotProductBatch_AOS(const Vec3f* a, const Vec3f* b, float* result, size_t count)
        {
            for (size_t i = 0; i < count; ++i)
            {
                result[i] = a[i].Dot(b[i]);
            }
        }


        void DotProductBatch_SOA(const Vec3SoA& a, const Vec3SoA& b, float* result, size_t count)
        {
            for (size_t i = 0; i < count; ++i)
            {
                result[i] = a.x[i] * b.x[i] + a.y[i] * b.y[i] + a.z[i] * b.z[i];
            }
        }

        void NormalizeBatch_AOS(const Vec3f* input, Vec3f* output, size_t count)
        {
            for (size_t i = 0; i < count; ++i)
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
        void CrossBatch(const Vec3SoA& a, const Vec3SoA& b, Vec3f* result, size_t count)
        {
            for (size_t i = 0; i < count; ++i)
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
