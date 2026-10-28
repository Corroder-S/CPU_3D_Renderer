#include <gtest/gtest.h>
#include "Maths/Matrix4x4.h"

TEST(MVPTests, ViewMatrixTranslatesWorld)
{
    Maths::Vec3f eye{ 0.0f, 0.0f, 5.0f };
    Maths::Vec3f target{ 0.0f, 0.0f, 0.0f };
    Maths::Vec3f up{ 0.0f, 1.0f, 0.0f };

    Maths::Matrix4x4 V = Maths::LookAt(eye, target, up);
    Maths::Vec4f worldOrigin{ 0.0f, 0.0f, 0.0f, 1.0f };

    Maths::Vec4f viewPoint = V * worldOrigin;
    EXPECT_FLOAT_EQ(viewPoint.z, -5.0f);
}

TEST(MVPTests, FullMVPChainProjectsToClipSpace)
{
    Maths::Matrix4x4 M = Maths::Matrix4x4();

    Maths::Matrix4x4 V = Maths::LookAt(Maths::Vec3f{ 0.0f, 0.0f, 5.0f }, Maths::Vec3f{ 0.0f, 0.0f, 0.0f }, Maths::Vec3f{ 0.0f, 1.0f, 0.0f });

    Maths::Matrix4x4 P = Maths::Perspective(90.0f, 1.0f, 0.1f, 100.0f);

    Maths::Matrix4x4 MVP = P * V * M;

    Maths::Vec4f p{ 0.0f, 0.0f, 0.0f, 1.0f };
    Maths::Vec4f clipPoint = MVP * p;

    EXPECT_FLOAT_EQ(clipPoint.x, 0.0f);
    EXPECT_FLOAT_EQ(clipPoint.y, 0.0f);
    EXPECT_GT(clipPoint.w, 0.0f);
}