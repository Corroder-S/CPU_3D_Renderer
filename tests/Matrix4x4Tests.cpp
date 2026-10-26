#include "TestHelpers.h"
#include "Maths/Matrix4x4.h"
#include <numbers>
#include <type_traits>

using namespace TestHelpers;

namespace
{
    template <typename T>
    void DefaultAndFactoryAreIdentity()
    {
        using M = Maths::Matrix4x4<T>;
        const M expected = M(std::array<T, 16>{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1});
        MatrixNear(expected, M{});
        MatrixNear(expected, M::Identity());
        MatrixNear(M(std::array<T, 16>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), M::Zero());
    }

    template <typename T>
    void ConstructorAndIndexUseRowMajorOrder()
    {
        using M = Maths::Matrix4x4<T>;
        M matrix = M(std::array<T, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
        Near(T{2}, matrix(0, 1));
        Near(T{5}, matrix(1, 0));
        matrix(1, 0) = T{99};
        const M& view = matrix;
        Near(T{99}, view(1, 0));
        EXPECT_THROW((void)matrix(4, 0), std::out_of_range);
        EXPECT_THROW((void)view(0, 4), std::out_of_range);
    }

    template <typename T>
    void ArithmeticHasIndependentExpectedValues()
    {
        using M = Maths::Matrix4x4<T>;
        const M a = M(std::array<T, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
        const M b = M(std::array<T, 16>{-1, 2, -2, 1, -3, 0, 3, -1, 2, -2, 1, -3, 0, 3, -1, 2});
        MatrixNear(M(std::array<T, 16>{0, 4, 1, 5, 2, 6, 10, 7, 11, 8, 12, 9, 13, 17, 14, 18}), a + b);
        MatrixNear(M(std::array<T, 16>{2, 0, 5, 3, 8, 6, 4, 9, 7, 12, 10, 15, 13, 11, 16, 14}), a - b);
        MatrixNear(M(std::array<T, 16>{2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32}), a * T{2});
    }

    template <typename T>
    void MatrixProductHasIndependentExpectedValues()
    {
        using M = Maths::Matrix4x4<T>;
        const M a = M(std::array<T, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
        const M b = M(std::array<T, 16>{-1, 2, -2, 1, -3, 0, 3, -1, 2, -2, 1, -3, 0, 3, -1, 2});
        const M expected = M(std::array<T, 16>{-1, 8, 3, -2, -9, 20, 7, -6, -17, 32, 11, -10, -25, 44, 15, -14});
        MatrixNear(expected, a * b);
        EXPECT_TRUE(a * b != b * a);
    }

    template <typename T>
    void MultiplyAssignSupportsSelfAliasing()
    {
        using M = Maths::Matrix4x4<T>;
        M matrix = M(std::array<T, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
        EXPECT_TRUE(&(matrix *= matrix) == &matrix);
        MatrixNear(M(std::array<T, 16>{90, 100, 110, 120, 202, 228, 254, 280, 314, 356, 398, 440, 426, 484, 542, 600}), matrix);
    }

    template <typename T>
    void MatrixVectorUsesColumnVectorConvention()
    {
        using M = Maths::Matrix4x4<T>;
        using V = Maths::Vec4<T>;
        const M matrix = M(std::array<T, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
        const V vector{1, 2, 3, 4};
        VectorNear(V{30, 70, 110, 150}, matrix * vector);
    }

    template <typename T>
    void TransposeMovesEveryElement()
    {
        using M = Maths::Matrix4x4<T>;
        const M matrix = M(std::array<T, 16>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
        MatrixNear(M(std::array<T, 16>{1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16}), matrix.Transpose());
    }

    template <typename T>
    void EqualityChecksEveryElement()
    {
        using M = Maths::Matrix4x4<T>;
        const M a;
        for (std::size_t r=0; r<4; ++r)
            for (std::size_t c=0; c<4; ++c)
            {
                M b = a;
                b(r,c) += T{1};
                EXPECT_TRUE(a != b);
            }
        EXPECT_TRUE(a == a);
    }

    template <typename T>
    void ScaleHasKnownResult()
    {
        using M = Maths::Matrix4x4<T>;
        const M scale = M::Scale(Maths::Vec3<T>{2,3,4});
        MatrixNear(M(std::array<T, 16>{2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 4, 0, 0, 0, 0, 1}), scale);
        Near(T{24}, scale.Determinant());
    }

    template <typename T>
    void RotationXIsRightHanded()
    {
        using M = Maths::Matrix4x4<T>;
        using V = Maths::Vec4<T>;
        const M rotation = M::RotationX(std::numbers::pi_v<T> / T{2});
        VectorNear(V{0, 0, 1, 0}, rotation * V{0, 1, 0, 0});
        Near(T{1}, rotation.Determinant());
    }

    template <typename T>
    void RotationYIsRightHanded()
    {
        using M = Maths::Matrix4x4<T>;
        using V = Maths::Vec4<T>;
        const M rotation = M::RotationY(std::numbers::pi_v<T> / T{2});
        VectorNear(V{1, 0, 0, 0}, rotation * V{0, 0, 1, 0});
        Near(T{1}, rotation.Determinant());
    }

    template <typename T>
    void RotationZIsRightHanded()
    {
        using M = Maths::Matrix4x4<T>;
        using V = Maths::Vec4<T>;
        const M rotation = M::RotationZ(std::numbers::pi_v<T> / T{2});
        VectorNear(V{0, 1, 0, 0}, rotation * V{1, 0, 0, 0});
        Near(T{1}, rotation.Determinant());
    }

    template <typename T>
    void InverseUsesPivotingAndKnownExpectedValues()
    {
        using M = Maths::Matrix4x4<T>;
        const M matrix = M(std::array<T, 16>{0, 2, 0, 0, 1, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1});
        const M expected = M(std::array<T, 16>{T{-1.5}, 1, 0, 0, T{0.5}, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1});
        const M actual = matrix.Inverse();
        MatrixNear(expected, actual);
        MatrixNear(M::Identity(), matrix * actual);
        MatrixNear(M::Identity(), actual * matrix);
        Near(T{-2}, matrix.Determinant());
    }

    template <typename T>
    void DenseInverseMatchesRationalFixture()
    {
        using M = Maths::Matrix4x4<T>;
        const M matrix = M(std::array<T, 16>{4, 1, -1, 1, 2, 5, 2, 0, 1, -1, 6, -1, 0, 2, 0, 7});
        const M expected = M(std::array<T, 16>{T{44}/T{175}, T{-1}/T{35}, T{9}/T{175}, T{-1}/T{35}, T{-2}/T{25}, T{1}/T{5}, T{-2}/T{25}, T{0}/T{1}, T{-9}/T{175}, T{1}/T{35}, T{26}/T{175}, T{1}/T{35}, T{4}/T{175}, T{-2}/T{35}, T{4}/T{175}, T{1}/T{7}});
        MatrixNear(expected, matrix.Inverse());
    }

    template <typename T>
    void InverseRejectsSingularAndNonFinite()
    {
        using M = Maths::Matrix4x4<T>;
        EXPECT_THROW((void)M::Zero().Inverse(), std::domain_error);
        M duplicate;
        for (std::size_t c=0; c<4; ++c)
            duplicate(1,c) = duplicate(0,c);
        Near(T{0}, duplicate.Determinant());
        EXPECT_THROW((void)duplicate.Inverse(), std::domain_error);
        M invalid;
        invalid(0,0) = std::numeric_limits<T>::infinity();
        EXPECT_THROW((void)invalid.Inverse(), std::domain_error);
        invalid(0,0) = std::numeric_limits<T>::quiet_NaN();
        EXPECT_THROW((void)invalid.Inverse(), std::domain_error);
    }

    template <typename T>
    void InverseValidatesTolerance()
    {
        using M = Maths::Matrix4x4<T>;
        EXPECT_THROW((void)M{}.Inverse(T{-1}), std::invalid_argument);
        EXPECT_THROW((void)M{}.Inverse(T{1}), std::invalid_argument);
        EXPECT_THROW((void)M{}.Inverse(std::numeric_limits<T>::quiet_NaN()), std::invalid_argument);
    }

    template <typename T>
    void InverseToleranceCanRejectNearDependentRows()
    {
        using M = Maths::Matrix4x4<T>;
        M matrix;
        matrix(0,0) = T{1}; matrix(0,1) = T{1};
        matrix(1,0) = T{1}; matrix(1,1) = T{1} + std::numeric_limits<T>::epsilon();
        EXPECT_THROW((void)matrix.Inverse(), std::domain_error);
    }

    template <typename T>
    void InverseHandlesDifferentRowScales()
    {
        using M = Maths::Matrix4x4<T>;
        M matrix;
        matrix(0,0) = T{0.0001};
        matrix(1,1) = T{10000};
        M expected;
        expected(0,0) = T{10000};
        expected(1,1) = T{0.0001};
        MatrixNear(expected, matrix.Inverse());
    }

    template <typename T>
    void TranslationAffectsPointsButNotDirections()
    {
        using M = Maths::Matrix4x4<T>;
        const M matrix = M::Translation(Maths::Vec3<T>{10,20,30});
        const Maths::Vec3<T> input{1,2,3};
        VectorNear(Maths::Vec3<T>{11,22,33}, matrix.TransformPoint(input));
        VectorNear(input, matrix.TransformDirection(input));
    }

    template <typename T>
    void CompositionAppliesRightMatrixFirst()
    {
        using M = Maths::Matrix4x4<T>;
        const M translate = M::Translation(Maths::Vec3<T>{10,20,30});
        const M scale = M::Scale(Maths::Vec3<T>{2,3,4});
        const Maths::Vec3<T> point{1,2,3};
        VectorNear(Maths::Vec3<T>{12,26,42}, (translate * scale).TransformPoint(point));
        VectorNear(Maths::Vec3<T>{22,66,132}, (scale * translate).TransformPoint(point));
    }

    template <typename T>
    void AffineHelpersRejectPerspective()
    {
        using M = Maths::Matrix4x4<T>;
        M matrix;
        matrix(3,2) = T{1};
        EXPECT_THROW((void)matrix.TransformPoint(Maths::Vec3<T>{1,2,3}), std::domain_error);
        EXPECT_THROW((void)matrix.TransformDirection(Maths::Vec3<T>{1,2,3}), std::domain_error);
    }
}

TEST(Matrix4x4Tests, DefaultAndFactoryAreIdentity_float)            { DefaultAndFactoryAreIdentity<float>(); }
TEST(Matrix4x4Tests, DefaultAndFactoryAreIdentity_double)           { DefaultAndFactoryAreIdentity<double>(); }
TEST(Matrix4x4Tests, ConstructorAndIndexUseRowMajorOrder_float)     { ConstructorAndIndexUseRowMajorOrder<float>(); }
TEST(Matrix4x4Tests, ConstructorAndIndexUseRowMajorOrder_double)    { ConstructorAndIndexUseRowMajorOrder<double>(); }
TEST(Matrix4x4Tests, ArithmeticHasIndependentExpectedValues_float)  { ArithmeticHasIndependentExpectedValues<float>(); }
TEST(Matrix4x4Tests, ArithmeticHasIndependentExpectedValues_double) { ArithmeticHasIndependentExpectedValues<double>(); }
TEST(Matrix4x4Tests, MatrixProductHasIndependentExpectedValues_float)  { MatrixProductHasIndependentExpectedValues<float>(); }
TEST(Matrix4x4Tests, MatrixProductHasIndependentExpectedValues_double) { MatrixProductHasIndependentExpectedValues<double>(); }
TEST(Matrix4x4Tests, MultiplyAssignSupportsSelfAliasing_float)      { MultiplyAssignSupportsSelfAliasing<float>(); }
TEST(Matrix4x4Tests, MultiplyAssignSupportsSelfAliasing_double)     { MultiplyAssignSupportsSelfAliasing<double>(); }
TEST(Matrix4x4Tests, MatrixVectorUsesColumnVectorConvention_float)  { MatrixVectorUsesColumnVectorConvention<float>(); }
TEST(Matrix4x4Tests, MatrixVectorUsesColumnVectorConvention_double) { MatrixVectorUsesColumnVectorConvention<double>(); }
TEST(Matrix4x4Tests, TransposeMovesEveryElement_float)              { TransposeMovesEveryElement<float>(); }
TEST(Matrix4x4Tests, TransposeMovesEveryElement_double)             { TransposeMovesEveryElement<double>(); }
TEST(Matrix4x4Tests, EqualityChecksEveryElement_float)              { EqualityChecksEveryElement<float>(); }
TEST(Matrix4x4Tests, EqualityChecksEveryElement_double)             { EqualityChecksEveryElement<double>(); }
TEST(Matrix4x4Tests, ScaleHasKnownResult_float)                     { ScaleHasKnownResult<float>(); }
TEST(Matrix4x4Tests, ScaleHasKnownResult_double)                    { ScaleHasKnownResult<double>(); }
TEST(Matrix4x4Tests, RotationXIsRightHanded_float)                  { RotationXIsRightHanded<float>(); }
TEST(Matrix4x4Tests, RotationXIsRightHanded_double)                 { RotationXIsRightHanded<double>(); }
TEST(Matrix4x4Tests, RotationYIsRightHanded_float)                  { RotationYIsRightHanded<float>(); }
TEST(Matrix4x4Tests, RotationYIsRightHanded_double)                 { RotationYIsRightHanded<double>(); }
TEST(Matrix4x4Tests, RotationZIsRightHanded_float)                  { RotationZIsRightHanded<float>(); }
TEST(Matrix4x4Tests, RotationZIsRightHanded_double)                 { RotationZIsRightHanded<double>(); }
TEST(Matrix4x4Tests, InverseUsesPivotingAndKnownExpectedValues_float)  { InverseUsesPivotingAndKnownExpectedValues<float>(); }
TEST(Matrix4x4Tests, InverseUsesPivotingAndKnownExpectedValues_double) { InverseUsesPivotingAndKnownExpectedValues<double>(); }
TEST(Matrix4x4Tests, DenseInverseMatchesRationalFixture_float)      { DenseInverseMatchesRationalFixture<float>(); }
TEST(Matrix4x4Tests, DenseInverseMatchesRationalFixture_double)     { DenseInverseMatchesRationalFixture<double>(); }
TEST(Matrix4x4Tests, InverseRejectsSingularAndNonFinite_float)      { InverseRejectsSingularAndNonFinite<float>(); }
TEST(Matrix4x4Tests, InverseRejectsSingularAndNonFinite_double)     { InverseRejectsSingularAndNonFinite<double>(); }
TEST(Matrix4x4Tests, InverseValidatesTolerance_float)               { InverseValidatesTolerance<float>(); }
TEST(Matrix4x4Tests, InverseValidatesTolerance_double)              { InverseValidatesTolerance<double>(); }
TEST(Matrix4x4Tests, InverseToleranceCanRejectNearDependentRows_float)  { InverseToleranceCanRejectNearDependentRows<float>(); }
TEST(Matrix4x4Tests, InverseToleranceCanRejectNearDependentRows_double) { InverseToleranceCanRejectNearDependentRows<double>(); }
TEST(Matrix4x4Tests, InverseHandlesDifferentRowScales_float)        { InverseHandlesDifferentRowScales<float>(); }
TEST(Matrix4x4Tests, InverseHandlesDifferentRowScales_double)       { InverseHandlesDifferentRowScales<double>(); }
TEST(Matrix4x4Tests, TranslationAffectsPointsButNotDirections_float)  { TranslationAffectsPointsButNotDirections<float>(); }
TEST(Matrix4x4Tests, TranslationAffectsPointsButNotDirections_double) { TranslationAffectsPointsButNotDirections<double>(); }
TEST(Matrix4x4Tests, CompositionAppliesRightMatrixFirst_float)      { CompositionAppliesRightMatrixFirst<float>(); }
TEST(Matrix4x4Tests, CompositionAppliesRightMatrixFirst_double)     { CompositionAppliesRightMatrixFirst<double>(); }
TEST(Matrix4x4Tests, AffineHelpersRejectPerspective_float)          { AffineHelpersRejectPerspective<float>(); }
TEST(Matrix4x4Tests, AffineHelpersRejectPerspective_double)         { AffineHelpersRejectPerspective<double>(); }
