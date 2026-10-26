#include "TestHelpers.h"
#include "Maths/Vec3.h"
#include "Maths/Vec3Batch.h"
#include <limits>

namespace
{
    void BuildKnownData(std::vector<Maths::Vec3f>& a, std::vector<Maths::Vec3f>& b, size_t count)
    {
        a.resize(count);
        b.resize(count);
        for (size_t i = 0; i < count; ++i)
        {
            a[i] = Maths::Vec3f{ static_cast<float>(i + 1), static_cast<float>(i + 2), static_cast<float>(i + 3) };
            b[i] = Maths::Vec3f{ static_cast<float>(i + 2), static_cast<float>(i + 3), static_cast<float>(i + 4) };
        }
    }

    void BuildNormalizeInput(std::vector<Maths::Vec3f>& input, size_t count)
    {
        input.resize(count);
        for (size_t i = 0; i < count; ++i)
        {
            input[i] = Maths::Vec3f{ static_cast<float>(i + 1), static_cast<float>(i + 2), static_cast<float>(i + 3) };
        }
    }

    void Ref_DotProductMatchesKnownValues()
    {
        const Maths::Vec3f a[] = { {1,2,3}, {2,3,4} };
        const Maths::Vec3f b[] = { {2,3,4}, {1,0,0} };
        float out[2] = {};
        Maths::Ref::DotProductBatch_AOS(a, b, out, 2);
        TestHelpers::Near(20.0f, out[0]);
        TestHelpers::Near(2.0f, out[1]);
    }

    void Ref_NormalizeMatchesKnownValues()
    {
        const Maths::Vec3f input[] = { {3, 4, 0} };
        Maths::Vec3f output[1] = {};

        Maths::Ref::NormalizeBatch_AOS(input, output, 1);

        TestHelpers::VectorNear(Maths::Vec3f{ 0.6f, 0.8f, 0.0f }, output[0]);
        TestHelpers::Near(1.0f, output[0].Magnitude());
    }

    void Normalize_ZeroVectorGivesZero()
    {
        const Maths::Vec3f input[] = { Maths::Vec3f::Zero };
        Maths::Vec3f outputRef[1] = {};
        Maths::Vec3f outputSse[1] = {};

        Maths::Ref::NormalizeBatch_AOS(input, outputRef, 1);
        Maths::SSE::NormalizeBatch_AOS(input, outputSse, 1);

        TestHelpers::VectorNear(Maths::Vec3f::Zero, outputRef[0]);
        TestHelpers::VectorNear(Maths::Vec3f::Zero, outputSse[0]);
    }

    void Normalize_NonFiniteGivesZero()
    {
        const float nan = std::numeric_limits<float>::quiet_NaN();
        const float inf = std::numeric_limits<float>::infinity();
        const Maths::Vec3f input[] = { {nan, 1.0f, 1.0f}, {inf, 1.0f, 1.0f} };
        Maths::Vec3f outputRef[2] = {};
        Maths::Vec3f outputSse[2] = {};

        Maths::Ref::NormalizeBatch_AOS(input, outputRef, 2);
        Maths::SSE::NormalizeBatch_AOS(input, outputSse, 2);

        TestHelpers::VectorNear(Maths::Vec3f::Zero, outputRef[0]);
        TestHelpers::VectorNear(Maths::Vec3f::Zero, outputRef[1]);
        TestHelpers::VectorNear(Maths::Vec3f::Zero, outputSse[0]);
        TestHelpers::VectorNear(Maths::Vec3f::Zero, outputSse[1]);
    }

    void Normalize_HandlesEmptyBatch()
    {
        std::vector<Maths::Vec3f> input, output;

        Maths::Ref::NormalizeBatch_AOS(input.data(), output.data(), 0);
        Maths::SSE::NormalizeBatch_AOS(input.data(), output.data(), 0);
    }

    void RefAndSSE_Normalize_GiveSameResult(size_t count)
    {
        std::vector<Maths::Vec3f> input;
        BuildNormalizeInput(input, count);

        std::vector<Maths::Vec3f> outputRef(count);
        std::vector<Maths::Vec3f> outputSse(count);

        Maths::Ref::NormalizeBatch_AOS(input.data(), outputRef.data(), count);
        Maths::SSE::NormalizeBatch_AOS(input.data(), outputSse.data(), count);

        for (size_t i = 0; i < count; ++i)
        {
            TestHelpers::VectorNear(outputRef[i], outputSse[i]);
        }
    }

    void Normalize_OutputHasUnitMagnitude(size_t count)
    {
        std::vector<Maths::Vec3f> input;
        BuildNormalizeInput(input, count);

        std::vector<Maths::Vec3f> output(count);
        Maths::SSE::NormalizeBatch_AOS(input.data(), output.data(), count);

        for (size_t i = 0; i < count; ++i)
        {
            TestHelpers::Near(1.0f, output[i].Magnitude());
        }
    }

    void DotProduct_HandlesEmptyBatch()
    {
        std::vector<Maths::Vec3f> a, b;
        std::vector<float> out;
        Maths::Ref::DotProductBatch_AOS(a.data(), b.data(), out.data(), 0);
        Maths::SSE::DotProductBatch_AOS(a.data(), b.data(), out.data(), 0);
    }

    void RefAndSSE_AoS_GiveSameResult(size_t count)
    {
        std::vector<Maths::Vec3f> a, b;
        BuildKnownData(a, b, count);
        std::vector<float> outRef(count);
        std::vector<float> outSse(count);
        Maths::Ref::DotProductBatch_AOS(a.data(), b.data(), outRef.data(), count);
        Maths::SSE::DotProductBatch_AOS(a.data(), b.data(), outSse.data(), count);
        for (size_t i = 0; i < count; ++i)
            TestHelpers::Near(outRef[i], outSse[i]);
    }

    void RefAndSSE_SoA_GiveSameResult(size_t count)
    {
        std::vector<Maths::Vec3f> a, b;
        BuildKnownData(a, b, count);
        Maths::Vec3SoA soaA = Maths::ConvertToSOA(a.data(), count);
        Maths::Vec3SoA soaB = Maths::ConvertToSOA(b.data(), count);
        std::vector<float> outRef(count);
        std::vector<float> outSoa(count);
        Maths::Ref::DotProductBatch_SOA(soaA, soaB, outRef.data(), count);
        Maths::SSE::DotProductBatch_SOA(soaA, soaB, outSoa.data(), count);
        for (size_t i = 0; i < count; ++i)
            TestHelpers::Near(outRef[i], outSoa[i]);
    }

    void RefAndSSE_Cross_GiveSameResult(size_t count)
    {
        std::vector<Maths::Vec3f> a, b;
        BuildKnownData(a, b, count);
        Maths::Vec3SoA soaA = Maths::ConvertToSOA(a.data(), count);
        Maths::Vec3SoA soaB = Maths::ConvertToSOA(b.data(), count);
        std::vector<Maths::Vec3f> outRef(count);
        std::vector<Maths::Vec3f> outSse(count);
        Maths::Ref::CrossBatch(soaA, soaB, outRef.data(), count);
        Maths::SSE::CrossProductBatch(soaA, soaB, outSse.data(), count);
        for (size_t i = 0; i < count; ++i)
        {
            TestHelpers::Near(outRef[i].x, outSse[i].x);
            TestHelpers::Near(outRef[i].y, outSse[i].y);
            TestHelpers::Near(outRef[i].z, outSse[i].z);
        }
    }
}

TEST(VecBatchTests, Ref_DotProductMatchesKnownValues)   { Ref_DotProductMatchesKnownValues(); }
TEST(VecBatchTests, DotProduct_HandlesEmptyBatch)        { DotProduct_HandlesEmptyBatch(); }
TEST(VecBatchTests, RefAndSSE_AoS_GiveSameResult1)       { RefAndSSE_AoS_GiveSameResult(1); }
TEST(VecBatchTests, RefAndSSE_AoS_GiveSameResult4)       { RefAndSSE_AoS_GiveSameResult(4); }
TEST(VecBatchTests, RefAndSSE_SoA_GiveSameResult4)       { RefAndSSE_SoA_GiveSameResult(4); }
TEST(VecBatchTests, RefAndSSE_AoS_GiveSameResult7)       { RefAndSSE_AoS_GiveSameResult(7); }
TEST(VecBatchTests, RefAndSSE_SoA_GiveSameResult100)     { RefAndSSE_SoA_GiveSameResult(1000); }
TEST(VecBatchTests, Ref_NormalizeMatchesKnownValues)        { Ref_NormalizeMatchesKnownValues(); }
TEST(VecBatchTests, Normalize_ZeroVectorGivesZero)          { Normalize_ZeroVectorGivesZero(); }
TEST(VecBatchTests, Normalize_NonFiniteGivesZero)           { Normalize_NonFiniteGivesZero(); }
TEST(VecBatchTests, Normalize_HandlesEmptyBatch)            { Normalize_HandlesEmptyBatch(); }
TEST(VecBatchTests, RefAndSSE_Normalize_GiveSameResult1)    { RefAndSSE_Normalize_GiveSameResult(1); }
TEST(VecBatchTests, RefAndSSE_Normalize_GiveSameResult4)    { RefAndSSE_Normalize_GiveSameResult(4); }
TEST(VecBatchTests, RefAndSSE_Normalize_GiveSameResult7)    { RefAndSSE_Normalize_GiveSameResult(7); }
TEST(VecBatchTests, Normalize_OutputHasUnitMagnitude1)      { Normalize_OutputHasUnitMagnitude(1); }
TEST(VecBatchTests, Normalize_OutputHasUnitMagnitude4)      { Normalize_OutputHasUnitMagnitude(4); }
TEST(VecBatchTests, Normalize_OutputHasUnitMagnitude7)      { Normalize_OutputHasUnitMagnitude(7); }
TEST(VecBatchTests, RefAndSSE_Cross_GiveSameResult1)        { RefAndSSE_Cross_GiveSameResult(1); }
TEST(VecBatchTests, RefAndSSE_Cross_GiveSameResult4)        { RefAndSSE_Cross_GiveSameResult(4); }
TEST(VecBatchTests, RefAndSSE_Cross_GiveSameResult7)        { RefAndSSE_Cross_GiveSameResult(7); }
