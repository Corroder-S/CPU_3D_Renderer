#include <gtest/gtest.h>
#include "Renderer/clip.h"

TEST(ClippingTests, FullyInsideProducesOneTriangle)
{
    ClipVertex v0{ { 0.0f,  1.0f, -1.0f, 5.0f}, {0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };
    ClipVertex v1{ {-1.0f, -1.0f, -1.0f, 5.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f} };
    ClipVertex v2{ { 1.0f, -1.0f, -1.0f, 5.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f} };

    ClipVertex out[6];
    int triCount = clipTriangleNear(v0, v1, v2, out);

    EXPECT_EQ(triCount, 1);
    EXPECT_NEAR(out[0].clip.z, -1.0f, 1e-4f);
    EXPECT_NEAR(out[1].clip.z, -1.0f, 1e-4f);
    EXPECT_NEAR(out[2].clip.z, -1.0f, 1e-4f);
}

TEST(ClippingTests, FullyOutsideIsCulled)
{
    ClipVertex v0{ { 0.0f,  1.0f, -6.0f, 5.0f}, {0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, -6.0f} };
    ClipVertex v1{ {-1.0f, -1.0f, -6.0f, 5.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {-1.0f, -1.0f, -6.0f} };
    ClipVertex v2{ { 1.0f, -1.0f, -6.0f, 5.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, -1.0f, -6.0f} };

    ClipVertex out[6];
    int triCount = clipTriangleNear(v0, v1, v2, out);

    EXPECT_EQ(triCount, 0);
}

TEST(ClippingTests, OneVertexInsideProducesOneTriangle)
{
    ClipVertex v0{ { 0.0f,  2.0f,  1.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 2.0f, 0.0f} };
    ClipVertex v1{ {-2.0f, -2.0f, -3.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {-2.0f, -2.0f, 0.0f} };
    ClipVertex v2{ { 2.0f, -2.0f, -3.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, -2.0f, 0.0f} };

    ClipVertex out[6];
    int triCount = clipTriangleNear(v0, v1, v2, out);

    ASSERT_EQ(triCount, 1);

    EXPECT_NEAR(out[0].nearDistance(), 0.0f, 1e-4f);
    EXPECT_NEAR(out[1].nearDistance(), 0.0f, 1e-4f);
}

TEST(ClippingTests, TwoVerticesInsideProducesTwoTriangles)
{
    ClipVertex v0{ {-1.0f,  1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f} };
    ClipVertex v1{ { 1.0f,  1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, { 1.0f, 1.0f, 0.0f} };
    ClipVertex v2{ { 0.0f, -2.0f,-3.0f, 1.0f}, {0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, { 0.0f,-2.0f, 0.0f} };

    ClipVertex out[6];
    int triCount = clipTriangleNear(v0, v1, v2, out);

    EXPECT_EQ(triCount, 2);
}

TEST(ClippingTests, LerpInterpolatesAllAttributes)
{
    ClipVertex a{
        {0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {10.0f, 0.0f, 0.0f}
    };
    ClipVertex b{
        {10.0f, 10.0f, 10.0f, 3.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f, 0.0f},
        {20.0f, 10.0f, 0.0f}
    };

    ClipVertex mid = ClipVertex::lerp(a, b, 0.5f);

    EXPECT_FLOAT_EQ(mid.clip.x, 5.0f);
    EXPECT_FLOAT_EQ(mid.clip.w, 2.0f);
    EXPECT_FLOAT_EQ(mid.uv.x, 0.5f);
    EXPECT_FLOAT_EQ(mid.uv.y, 0.5f);
    EXPECT_FLOAT_EQ(mid.normal.x, 0.5f);
    EXPECT_FLOAT_EQ(mid.normal.y, 0.5f);
    EXPECT_FLOAT_EQ(mid.worldPos.x, 15.0f);
}