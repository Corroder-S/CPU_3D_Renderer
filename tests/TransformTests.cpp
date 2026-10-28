#include <gtest/gtest.h>

#include "src/Scene/transform.h"

TEST(TransformTests, CombineTRS)
{
    Transform transform;
    transform.position = { 0.0f, 5.0f, 0.0f };
    transform.scale = { 2.0f, 2.0f, 2.0f };

    Maths::Matrix4x4 M = transform.getModelMatrix();
    Maths::Vec4f localPoint{ 1.0f, 1.0f, 0.0f, 1.0f };

    Maths::Vec4f worldPoint = M * localPoint;

    EXPECT_FLOAT_EQ(worldPoint.x, 2.0f);
    EXPECT_FLOAT_EQ(worldPoint.y, 7.0f);
    EXPECT_FLOAT_EQ(worldPoint.z, 0.0f);
    EXPECT_FLOAT_EQ(worldPoint.w, 1.0f);
}