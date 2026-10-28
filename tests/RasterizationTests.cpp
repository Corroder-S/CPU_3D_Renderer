#include <gtest/gtest.h>
#include <vector>
#include "Renderer/Draw.h"
#include "Asset/Material.h"
#include "Asset/Light.h"

class RasterizationTests : public ::testing::Test
{
protected:
    const int width = 1280;
    const int height = 800;

    std::vector<uint32_t> framebuffer;
    std::vector<float> zbuffer;

    void SetUp() override
    {
        framebuffer.assign(width * height, 0x00000000);
        zbuffer.assign(width * height, 1.0f);
    }
};

TEST_F(RasterizationTests, DrawLineHorizontalAndVertical)
{
    uint32_t color = 0xFF0000FF; // Rouge

    drawline(10, 20, 30, 20, color, framebuffer);

    for (int x = 10; x <= 30; ++x)
    {
        EXPECT_EQ(framebuffer[20 * width + x], color);
    }

    EXPECT_EQ(framebuffer[21 * width + 20], 0x00000000);
}

TEST_F(RasterizationTests, TriangleRasterizationFillsInsidePixels)
{
    Maths::Vec3f a{ 20.0f, 10.0f, 0.5f };
    Maths::Vec3f b{ 40.0f, 40.0f, 0.5f };
    Maths::Vec3f c{ 10.0f, 40.0f, 0.5f };

    Material mat;
    mat.diffuse = Color{ 1.0f, 0.0f, 0.0f, 1.0f }; // Rouge

    Light light;
    light.direction = Maths::Vec3f{ 0.0f, 0.0f, 1.0f };

    ShaderUniforms uniforms;
    uniforms.material = &mat;
    uniforms.light = &light;

    Maths::Vec2i renderMin{ 0, 0 };
    Maths::Vec2i renderMax{ width, height };

    drawTriangle(
        a, b, c,
        { 0,0 }, { 1,0 }, { 0,1 },               // UVs
        { 1.0f, 1.0f, 1.0f },                 // invW
        { 0,0,1 }, { 0,0,1 }, { 0,0,1 },          // world normals
        a, b, c,                            // world pos
        framebuffer, zbuffer,
        uniforms, light,
        renderMin, renderMax
    );

    EXPECT_NE(framebuffer[30 * width + 23], 0x00000000);

    EXPECT_EQ(framebuffer[0 * width + 0], 0x00000000);
}

TEST_F(RasterizationTests, TriangleZBufferRejection)
{
    Material mat;
    mat.diffuse = Color{ 1.0f, 1.0f, 1.0f, 1.0f };

    Light light;
    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;

    Maths::Vec3f a{ 10.0f, 10.0f, 0.2f };
    Maths::Vec3f b{ 30.0f, 10.0f, 0.2f };
    Maths::Vec3f c{ 20.0f, 30.0f, 0.2f };

    drawTriangle(
        a, b, c, { 0,0 }, { 1,0 }, { 0,1 }, { 1,1,1 },
        { 0,0,1 }, { 0,0,1 }, { 0,0,1 }, a, b, c,
        framebuffer, zbuffer, u, light,
        { 0,0 }, { width, height }
    );

    int testIdx = 15 * width + 20;
    EXPECT_FLOAT_EQ(zbuffer[testIdx], 0.2f);

    Maths::Vec3f a2{ 10.0f, 10.0f, 0.8f };
    Maths::Vec3f b2{ 30.0f, 10.0f, 0.8f };
    Maths::Vec3f c2{ 20.0f, 30.0f, 0.8f };

    drawTriangle(
        a2, b2, c2, { 0,0 }, { 1,0 }, { 0,1 }, { 1,1,1 },
        { 0,0,1 }, { 0,0,1 }, { 0,0,1 }, a2, b2, c2,
        framebuffer, zbuffer, u, light,
        { 0,0 }, { width, height }
    );

    EXPECT_FLOAT_EQ(zbuffer[testIdx], 0.2f);
}

TEST_F(RasterizationTests, RespectsRenderMinAndMaxBounds)
{
    Material mat;
    Light light;
    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;

    Maths::Vec3f a{ 0.0f, 0.0f, 0.5f };
    Maths::Vec3f b{ 50.0f, 0.0f, 0.5f };
    Maths::Vec3f c{ 25.0f, 50.0f, 0.5f };

    Maths::Vec2i renderMin{ 10, 10 };
    Maths::Vec2i renderMax{ 20, 20 };

    drawTriangle(
        a, b, c, { 0,0 }, { 1,0 }, { 0,1 }, { 1,1,1 },
        { 0,0,1 }, { 0,0,1 }, { 0,0,1 }, a, b, c,
        framebuffer, zbuffer, u, light,
        renderMin, renderMax
    );

    EXPECT_NE(framebuffer[15 * width + 15], 0x00000000);

    EXPECT_EQ(framebuffer[5 * width + 5], 0x00000000);
}