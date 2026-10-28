#include "Maths/ASM/Vec3.h"
#include "TestHelpers.h"
#include <gtest/gtest.h>

namespace
{
    using V = Maths::Vec3<float>;
}

TEST(ASM_Vec3DotProduct, BasicResult)
{
    const V a{1.f, 2.f, 3.f};
    const V b{2.f, 3.f, 4.f};
    float result = -1.f;
    Maths::ASM::DotProductBatch_AOS(&a, &b, &result, 1);
    // 1*2 + 2*3 + 3*4 = 2 + 6 + 12 = 20
    TestHelpers::Near(20.f, result);
}

TEST(ASM_Vec3DotProduct, BatchMultipleElements)
{
    const V a[3] = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};
    const V b[3] = {{1.f, 2.f, 3.f}, {1.f, 2.f, 3.f}, {1.f, 2.f, 3.f}};
    float result[3] = {-1.f, -1.f, -1.f};
    Maths::ASM::DotProductBatch_AOS(a, b, result, 3);
    TestHelpers::Near(1.f, result[0]);
    TestHelpers::Near(2.f, result[1]);
    TestHelpers::Near(3.f, result[2]);
}

TEST(ASM_Vec3DotProduct, ZeroVector)
{
    const V a{0.f, 0.f, 0.f};
    const V b{1.f, 2.f, 3.f};
    float result = -1.f;
    Maths::ASM::DotProductBatch_AOS(&a, &b, &result, 1);
    TestHelpers::Near(0.f, result);
}

TEST(ASM_Vec3DotProduct, CountZero)
{
    const V a{1.f, 2.f, 3.f};
    const V b{4.f, 5.f, 6.f};
    float result = 42.f;
    Maths::ASM::DotProductBatch_AOS(&a, &b, &result, 0);
    // count=0 : rien ne doit etre ecrit
    EXPECT_FLOAT_EQ(42.f, result);
}

TEST(ASM_Vec3DotProduct, Orthogonal)
{
    const V a{1.f, 0.f, 0.f};
    const V b{0.f, 1.f, 0.f};
    float result = -1.f;
    Maths::ASM::DotProductBatch_AOS(&a, &b, &result, 1);
    TestHelpers::Near(0.f, result);
}

TEST(ASM_Vec3DotProduct, NegativeComponents)
{
    const V a{-1.f, 2.f, -3.f};
    const V b{4.f, -5.f, 6.f};
    float result = 0.f;
    Maths::ASM::DotProductBatch_AOS(&a, &b, &result, 1);
    // -4 - 10 - 18 = -32
    TestHelpers::Near(-32.f, result);
}
