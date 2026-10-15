#pragma once
#include "Maths/Vec3.h"
#include <vector>


namespace Maths
{
    struct Vec3SoA
    {
        std::vector<float> x;
        std::vector<float> y;
        std::vector<float> z;
    };

    Vec3SoA ConvertToSOA(const Vec3f* input, size_t count);

    namespace Ref
    {
        void DotProductBatch_AOS(const Vec3f* a, const Vec3f* b, float* result, size_t count);
        void DotProductBatch_SOA(const Vec3SoA& a, const Vec3SoA& b, float* result, size_t count);
        void NormalizeBatch_AOS(const Vec3f* input, Vec3f* output, size_t count);
        void CrossBatch(const Vec3SoA& a, const Vec3SoA& b, Vec3f* result, size_t count);
    }

    namespace SSE
    {
        void DotProductBatch_AOS(const Vec3f* a, const Vec3f* b, float* result, size_t count);
        void DotProductBatch_SOA(const Vec3SoA& a, const Vec3SoA& b, float* result, size_t count);
        void NormalizeBatch_AOS(const Vec3f* input, Vec3f* output, size_t count);
        void CrossProductBatch(const Vec3SoA& a, const Vec3SoA& b, Vec3f* result, size_t count);
    }

}
