#include "TestHelpers.h"
#include "Maths/Matrix4x4.h"
#include "Maths/Mat4Batch.h"

namespace
{
    void Ref_TranslationAffectsPoints()
    {
        using M = Maths::Matrix4x4<>;
        const M matrix = M::Translation(Maths::Vec3<>{10.f, 20.f, 30.f});
        const Maths::Vec3<> input[] = {{1,2,3}, {4,5,6}, {7,8,9}};
        Maths::Vec3<> output[3] = {};
        Maths::Ref::TransformPointBatch(matrix, input, output, 3);
        TestHelpers::VectorNear({11, 22, 33}, output[0]);
        TestHelpers::VectorNear({14, 25, 36}, output[1]);
        TestHelpers::VectorNear({17, 28, 39}, output[2]);
    }

    void SSE_TranslationAffectsPoints()
    {
        using M = Maths::Matrix4x4<>;
        const M matrix = M::Translation(Maths::Vec3<>{10.f, 20.f, 30.f});
        const Maths::Vec3<> input[] = {{1,2,3}, {4,5,6}, {7,8,9}};
        Maths::Vec3<> output[3] = {};
        Maths::SSE::TransformPointBatch(matrix, input, output, 3);
        TestHelpers::VectorNear({11, 22, 33}, output[0]);
        TestHelpers::VectorNear({14, 25, 36}, output[1]);
        TestHelpers::VectorNear({17, 28, 39}, output[2]);
    }

    void RefAndSSEGiveSameResult()
    {
        using M = Maths::Matrix4x4<>;
        const M matrix = M::Translation(Maths::Vec3<>{10.f, 20.f, 30.f});
        const Maths::Vec3<> input[] = {{1,2,3}, {4,5,6}, {7,8,9}};
        Maths::Vec3<> outputRef[3] = {};
        Maths::Vec3<> outputSSE[3] = {};
        Maths::Ref::TransformPointBatch(matrix, input, outputRef, 3);
        Maths::SSE::TransformPointBatch(matrix, input, outputSSE, 3);
        for (int i = 0; i < 3; ++i)
            TestHelpers::VectorNear(outputRef[i], outputSSE[i]);
    }
}

TEST(MatBatchTests, Ref_TranslationAffectsPoints) { Ref_TranslationAffectsPoints(); }
TEST(MatBatchTests, SSE_TranslationAffectsPoints) { SSE_TranslationAffectsPoints(); }
TEST(MatBatchTests, RefAndSSEGiveSameResult)       { RefAndSSEGiveSameResult(); }
