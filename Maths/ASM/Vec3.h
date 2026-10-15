#pragma once
#include "Maths/Vec3.h"
extern "C" void ASM_DotProductBatch_AOS(const Maths::Vec3<float>* a, const Maths::Vec3<float>* b, float* result, size_t count);
namespace Maths::ASM
{
    inline void DotProductBatch_AOS(const Vec3<float>* a, const Vec3<float>* b, float* result, size_t count)
    {
        ::ASM_DotProductBatch_AOS(a, b, result, count);
    }
}
