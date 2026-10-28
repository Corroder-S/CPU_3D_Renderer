#include "TestHelpers.h"
#include "Maths/Vec4.h"
#include <numbers>
#include <type_traits>

using namespace TestHelpers;

namespace
{
    template <typename T>
    void DefaultConstructorIsZero()
    {
        using V = Maths::Vec4<T>;
        const V actual;
        VectorNear(V{0, 0, 0, 0}, actual);
    }

    template <typename T>
    void ComponentsAndCopyAreIndependent()
    {
        using V = Maths::Vec4<T>;
        const V original{1, 2, 3, 4};
        V copy = original;
        copy.x = T{99};
        VectorNear(V{1, 2, 3, 4}, original);
        EXPECT_TRUE(copy.x == T{99});
    }

    template <typename T>
    void ExplicitConversionPreservesValues()
    {
        using V = Maths::Vec4<T>;
        const Maths::Vec4<double> source{1.5, 2.5, 3.5, 4.5};
        const V actual(source);
        VectorNear(V{T{1.5}, T{2.5}, T{3.5}, T{4.5}}, actual);
    }

    template <typename T>
    void AdditionHasKnownResult()
    {
        using V = Maths::Vec4<T>;
        const V a = V{1, 2, 3, 4};
        const V b = V{2, 3, 4, 5};
        const V actual = a + b;
        VectorNear(V{3, 5, 7, 9}, actual);
    }

    template <typename T>
    void SubtractionHasKnownResult()
    {
        using V = Maths::Vec4<T>;
        const V a = V{1, 2, 3, 4};
        const V b = V{2, 3, 4, 5};
        const V actual = a - b;
        VectorNear(V{-1, -1, -1, -1}, actual);
    }

    template <typename T>
    void ComponentProductHasKnownResult()
    {
        using V = Maths::Vec4<T>;
        const V a = V{1, 2, 3, 4};
        const V b = V{2, 3, 4, 5};
        const V actual = a * b;
        VectorNear(V{2, 6, 12, 20}, actual);
    }

    template <typename T>
    void ComponentDivisionHasKnownResult()
    {
        using V = Maths::Vec4<T>;
        const V a = V{2, 6, 12, 20};
        const V b = V{2, 3, 4, 5};
        const V actual = a / b;
        VectorNear(V{1, 2, 3, 4}, actual);
    }

    template <typename T>
    void ScalarOperationsAndNegation()
    {
        using V = Maths::Vec4<T>;
        const V a = V{1, 2, 3, 4};
        VectorNear(V{-1, -2, -3, -4}, -a);
        VectorNear(V{2, 4, 6, 8}, a * T{2});
        VectorNear(V{2, 4, 6, 8}, T{2} * a);
        VectorNear(a, (a * T{2}) / T{2});
    }

    template <typename T>
    void CompoundOperatorsReturnSelfAndSupportAliasing()
    {
        using V = Maths::Vec4<T>;
        V a = V{1, 2, 3, 4};
        EXPECT_TRUE(&(a += a) == &a);
        VectorNear(V{2, 4, 6, 8}, a);
        a -= V{1, 2, 3, 4};
        a *= a;
        VectorNear(V{1, 4, 9, 16}, a);
        a /= V{1, 2, 3, 4};
        a *= T{2};
        a /= T{2};
        VectorNear(V{1, 2, 3, 4}, a);
    }

    template <typename T>
    void DivisionByZeroThrowsWithoutPartialMutation()
    {
        using V = Maths::Vec4<T>;
        const V original = V{1, 2, 3, 4};
        V actual = original;
        EXPECT_THROW(actual /= T{0}, std::domain_error);
        VectorNear(original, actual);
        EXPECT_THROW((actual /= V{0, 1, 1, 1}), std::domain_error);
        VectorNear(original, actual);
        EXPECT_THROW((actual /= V{1, 0, 1, 1}), std::domain_error);
        VectorNear(original, actual);
        EXPECT_THROW((actual /= V{1, 1, 0, 1}), std::domain_error);
        VectorNear(original, actual);
        EXPECT_THROW((actual /= V{1, 1, 1, 0}), std::domain_error);
        VectorNear(original, actual);
    }

    template <typename T>
    void EqualityChecksEveryComponentExactly()
    {
        using V = Maths::Vec4<T>;
        const V a = V{1, 2, 3, 4};
        EXPECT_TRUE(a == a);
        V changedx = a;
        changedx.x += T{1};
        EXPECT_TRUE(changedx != a);
        V changedy = a;
        changedy.y += T{1};
        EXPECT_TRUE(changedy != a);
        V changedz = a;
        changedz.z += T{1};
        EXPECT_TRUE(changedz != a);
        V changedw = a;
        changedw.w += T{1};
        EXPECT_TRUE(changedw != a);
        V nan = a;
        nan.x = std::numeric_limits<T>::quiet_NaN();
        EXPECT_FALSE(nan == nan);
    }

    template <typename T>
    void DotHasKnownResult()
    {
        using V = Maths::Vec4<T>;
        const V a = V{1, 2, 3, 4};
        const V b = V{2, 3, 4, 5};
        Near(T{40}, a.Dot(b));
    }

    template <typename T>
    void MagnitudeHasKnownResult()
    {
        using V = Maths::Vec4<T>;
        const V a = V{3, 4, 0, 0};
        Near(T{25}, a.MagnitudeSquared());
        Near(T{5}, a.Magnitude());
        Near(T{0}, V{}.Magnitude());
    }

    template <typename T>
    void NormalizeReturnsNewUnitVector()
    {
        using V = Maths::Vec4<T>;
        const V a = V{3, 4, 0, 0};
        const V actual = a.Normalize();
        VectorNear(V{T{0.6}, T{0.8}, T{0}, T{0}}, actual);
        Near(T{1}, actual.Magnitude());
        VectorNear(V{3, 4, 0, 0}, a);
    }

    template <typename T>
    void NormalizeRejectsZeroAndNonFinite()
    {
        using V = Maths::Vec4<T>;
        EXPECT_THROW((void)V{}.Normalize(), std::domain_error);
        V invalid = V{1, 2, 3, 4};
        invalid.x = std::numeric_limits<T>::infinity();
        EXPECT_THROW((void)invalid.Normalize(), std::domain_error);
        invalid.x = std::numeric_limits<T>::quiet_NaN();
        EXPECT_THROW((void)invalid.Normalize(), std::domain_error);
    }

    template <typename T>
    void NormalizeHandlesVeryLargeAndSmallFiniteValues()
    {
        using V = Maths::Vec4<T>;
        const T large = std::numeric_limits<T>::max();
        const T small = std::numeric_limits<T>::min();
        const V expected = V{T{1}, T{1}, T{1}, T{1}} / std::sqrt(T{4});
        VectorNear(expected, V{large, large, large, large}.Normalize());
        VectorNear(expected, V{small, small, small, small}.Normalize());
    }

    template <typename T>
    void DistanceHasKnownResult()
    {
        using V = Maths::Vec4<T>;
        const V a = V{1, 2, 3, 3};
        const V b = V{4, 6, 3, 3};
        Near(T{25}, a.DistanceSquared(b));
        Near(T{5}, a.Distance(b));
    }

    template <typename T>
    void AngleUsesRadians()
    {
        using V = Maths::Vec4<T>;
        Near(std::numbers::pi_v<T> / T{2}, V::UnitX.Angle(V::UnitY));
        Near(std::numbers::pi_v<T>, V::UnitX.Angle(-V::UnitX));
        Near(T{0}, V::UnitX.Angle(V::UnitX));
        EXPECT_THROW((void)V::Zero.Angle(V::UnitX), std::domain_error);
    }

    template <typename T>
    void LerpSupportsEndpointsMidpointAndExtrapolation()
    {
        using V = Maths::Vec4<T>;
        const V a = V{0, 0, 0, 0};
        const V b = V{2, 4, 6, 8};
        VectorNear(a, V::Lerp(a, b, T{0}));
        VectorNear(b, V::Lerp(a, b, T{1}));
        VectorNear(V{1, 2, 3, 4}, V::Lerp(a, b, T{0.5}));
        VectorNear(V{4, 8, 12, 16}, V::Lerp(a, b, T{2}));
    }

    template <typename T>
    void MinMaxAreComponentWise()
    {
        using V = Maths::Vec4<T>;
        const V a = V{1, -4, 7, 2};
        const V b = V{3, -2, 5, -8};
        VectorNear(V{1, -4, 5, -8}, V::Min(a,b));
        VectorNear(V{3, -2, 7, 2}, V::Max(a,b));
    }

    template <typename T>
    void NamedConstantsHaveExpectedComponents()
    {
        using V = Maths::Vec4<T>;
        VectorNear(V{0, 0, 0, 0}, V::Zero);
        VectorNear(V{1, 1, 1, 1}, V::One);
        VectorNear(V{1, 0, 0, 0}, V::UnitX);
        VectorNear(V{0, 1, 0, 0}, V::UnitY);
        VectorNear(V{0, 0, 1, 0}, V::UnitZ);
        VectorNear(V{0, 0, 0, 1}, V::UnitW);
    }
}

TEST(Vec4Tests, DefaultConstructorIsZero_float)          { DefaultConstructorIsZero<float>(); }
TEST(Vec4Tests, DefaultConstructorIsZero_double)         { DefaultConstructorIsZero<double>(); }
TEST(Vec4Tests, ComponentsAndCopyAreIndependent_float)   { ComponentsAndCopyAreIndependent<float>(); }
TEST(Vec4Tests, ComponentsAndCopyAreIndependent_double)  { ComponentsAndCopyAreIndependent<double>(); }
TEST(Vec4Tests, ExplicitConversionPreservesValues_float) { ExplicitConversionPreservesValues<float>(); }
TEST(Vec4Tests, ExplicitConversionPreservesValues_double){ ExplicitConversionPreservesValues<double>(); }
TEST(Vec4Tests, AdditionHasKnownResult_float)            { AdditionHasKnownResult<float>(); }
TEST(Vec4Tests, AdditionHasKnownResult_double)           { AdditionHasKnownResult<double>(); }
TEST(Vec4Tests, SubtractionHasKnownResult_float)         { SubtractionHasKnownResult<float>(); }
TEST(Vec4Tests, SubtractionHasKnownResult_double)        { SubtractionHasKnownResult<double>(); }
TEST(Vec4Tests, ComponentProductHasKnownResult_float)    { ComponentProductHasKnownResult<float>(); }
TEST(Vec4Tests, ComponentProductHasKnownResult_double)   { ComponentProductHasKnownResult<double>(); }
TEST(Vec4Tests, ComponentDivisionHasKnownResult_float)   { ComponentDivisionHasKnownResult<float>(); }
TEST(Vec4Tests, ComponentDivisionHasKnownResult_double)  { ComponentDivisionHasKnownResult<double>(); }
TEST(Vec4Tests, ScalarOperationsAndNegation_float)       { ScalarOperationsAndNegation<float>(); }
TEST(Vec4Tests, ScalarOperationsAndNegation_double)      { ScalarOperationsAndNegation<double>(); }
TEST(Vec4Tests, CompoundOperatorsReturnSelfAndSupportAliasing_float)  { CompoundOperatorsReturnSelfAndSupportAliasing<float>(); }
TEST(Vec4Tests, CompoundOperatorsReturnSelfAndSupportAliasing_double) { CompoundOperatorsReturnSelfAndSupportAliasing<double>(); }
TEST(Vec4Tests, DivisionByZeroThrowsWithoutPartialMutation_float)     { DivisionByZeroThrowsWithoutPartialMutation<float>(); }
TEST(Vec4Tests, DivisionByZeroThrowsWithoutPartialMutation_double)    { DivisionByZeroThrowsWithoutPartialMutation<double>(); }
TEST(Vec4Tests, EqualityChecksEveryComponentExactly_float)  { EqualityChecksEveryComponentExactly<float>(); }
TEST(Vec4Tests, EqualityChecksEveryComponentExactly_double) { EqualityChecksEveryComponentExactly<double>(); }
TEST(Vec4Tests, DotHasKnownResult_float)                 { DotHasKnownResult<float>(); }
TEST(Vec4Tests, DotHasKnownResult_double)                { DotHasKnownResult<double>(); }
TEST(Vec4Tests, MagnitudeHasKnownResult_float)           { MagnitudeHasKnownResult<float>(); }
TEST(Vec4Tests, MagnitudeHasKnownResult_double)          { MagnitudeHasKnownResult<double>(); }
TEST(Vec4Tests, NormalizeReturnsNewUnitVector_float)     { NormalizeReturnsNewUnitVector<float>(); }
TEST(Vec4Tests, NormalizeReturnsNewUnitVector_double)    { NormalizeReturnsNewUnitVector<double>(); }
TEST(Vec4Tests, NormalizeRejectsZeroAndNonFinite_float)  { NormalizeRejectsZeroAndNonFinite<float>(); }
TEST(Vec4Tests, NormalizeRejectsZeroAndNonFinite_double) { NormalizeRejectsZeroAndNonFinite<double>(); }
TEST(Vec4Tests, NormalizeHandlesVeryLargeAndSmallFiniteValues_float)  { NormalizeHandlesVeryLargeAndSmallFiniteValues<float>(); }
TEST(Vec4Tests, NormalizeHandlesVeryLargeAndSmallFiniteValues_double) { NormalizeHandlesVeryLargeAndSmallFiniteValues<double>(); }
TEST(Vec4Tests, DistanceHasKnownResult_float)            { DistanceHasKnownResult<float>(); }
TEST(Vec4Tests, DistanceHasKnownResult_double)           { DistanceHasKnownResult<double>(); }
TEST(Vec4Tests, AngleUsesRadians_float)                  { AngleUsesRadians<float>(); }
TEST(Vec4Tests, AngleUsesRadians_double)                 { AngleUsesRadians<double>(); }
TEST(Vec4Tests, LerpSupportsEndpointsMidpointAndExtrapolation_float)  { LerpSupportsEndpointsMidpointAndExtrapolation<float>(); }
TEST(Vec4Tests, LerpSupportsEndpointsMidpointAndExtrapolation_double) { LerpSupportsEndpointsMidpointAndExtrapolation<double>(); }
TEST(Vec4Tests, MinMaxAreComponentWise_float)            { MinMaxAreComponentWise<float>(); }
TEST(Vec4Tests, MinMaxAreComponentWise_double)           { MinMaxAreComponentWise<double>(); }
TEST(Vec4Tests, NamedConstantsHaveExpectedComponents_float)  { NamedConstantsHaveExpectedComponents<float>(); }
TEST(Vec4Tests, NamedConstantsHaveExpectedComponents_double) { NamedConstantsHaveExpectedComponents<double>(); }
