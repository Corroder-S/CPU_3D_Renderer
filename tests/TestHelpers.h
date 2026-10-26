#pragma once
#include <gtest/gtest.h>
#include "Maths/Vec3.h"
#include "Maths/Vec4.h"
#include <cmath>
#include <limits>
#include <iterator>
#include <sstream>

namespace TestHelpers
{

    template <typename T>
    void Near(T expected, T actual, const char* context = "value")
    {
        // A failed NaN/Inf result must never silently pass a tolerance check.
        const T absoluteTolerance = T{64} * std::numeric_limits<T>::epsilon();
        const T relativeTolerance = T{128} * std::numeric_limits<T>::epsilon();
        const T tolerance = absoluteTolerance + relativeTolerance * std::abs(expected);
        const bool close = std::isfinite(expected) && std::isfinite(actual) &&
            std::abs(expected - actual) <= tolerance;
        std::ostringstream message;
        message << context << ": expected " << expected << ", actual " << actual
                << ", tolerance " << tolerance;
        EXPECT_TRUE(close) << message.str();

    }

    template <typename T>
    void VectorNear(const Maths::Vec3<T>& expected, const Maths::Vec3<T>& actual)
    {
        Near(expected.x, actual.x, "x");
        Near(expected.y, actual.y, "y");
        Near(expected.z, actual.z, "z");
    }

    template <typename T>
    void VectorNear(const Maths::Vec4<T>& expected, const Maths::Vec4<T>& actual)
    {
        Near(expected.x, actual.x, "x");
        Near(expected.y, actual.y, "y");
        Near(expected.z, actual.z, "z");
        Near(expected.w, actual.w, "w");
    }

    template <typename Matrix>
    void MatrixNear(const Matrix& expected, const Matrix& actual)
    {
        const std::size_t count = std::size(expected.values);
        for (std::size_t row = 0; row < count; ++row)
        {
            for (std::size_t column = 0; column < count; ++column)
            {
                std::ostringstream context;
                context << "matrix[" << row << "][" << column << "]";
                Near(expected.values[row][column], actual.values[row][column], context.str().c_str());
            }
        }
    }
}
