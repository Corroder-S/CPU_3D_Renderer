#include <gtest/gtest.h>
#include "src/Asset/Color.h"
#include "Maths/Vec3.h"

TEST(ColorTests, DefaultIsBlackOrTransparent)
{
    Color c;
    EXPECT_FLOAT_EQ(c.r, 0.0f);
    EXPECT_FLOAT_EQ(c.g, 0.0f);
    EXPECT_FLOAT_EQ(c.b, 0.0f);
}

TEST(ColorTests, PackingToUint32)
{
    Color red{ 1.0f, 0.0f, 0.0f, 1.0f };
    uint32_t packedRed = red.toRGBA32();

    EXPECT_EQ((packedRed >> 16) & 0xFF, 0xFF); // R
    EXPECT_EQ((packedRed >> 8) & 0xFF, 0x00);  // G
    EXPECT_EQ(packedRed & 0xFF, 0x00);         // B
}

TEST(ColorTests, ClampingValues)
{
    Color overexposed{ 1.5f, 2.0f, -0.5f, 1.0f };
    Color clamped = overexposed.clamp();

    EXPECT_FLOAT_EQ(clamped.r, 1.0f);
    EXPECT_FLOAT_EQ(clamped.g, 1.0f);
    EXPECT_FLOAT_EQ(clamped.b, 0.0f);
}