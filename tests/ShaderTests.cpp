#include <gtest/gtest.h>
#include "src/Renderer/Shader.h"
#include "src/Asset/Material.h"
#include "src/Asset/Light.h"

TEST(ShaderTests, NullUniformsReturnWhite)
{
    FragmentInput in;
    ShaderUniforms u;

    Color result = evaluateBlinnPhong(in, u);
    EXPECT_FLOAT_EQ(result.r, Color::White.r);
    EXPECT_FLOAT_EQ(result.g, Color::White.g);
    EXPECT_FLOAT_EQ(result.b, Color::White.b);
    EXPECT_FLOAT_EQ(result.a, Color::White.a);
}

TEST(ShaderTests, AmbientLightingWhenSurfaceFacesAway)
{
    Material mat;
    mat.diffuse = Color{ 1.0f, 0.0f, 0.0f, 1.0f };
    mat.ambient = Color{ 1.0f, 1.0f, 1.0f, 1.0f };

    Light light;
    light.direction = Maths::Vec3f{ 0.0f, 0.0f, 1.0f };
    light.ambient = 0.2f;

    FragmentInput in;
    in.normal = Maths::Vec3f{ 0.0f, 0.0f, -1.0f };
    in.worldPos = Maths::Vec3f{ 0.0f, 0.0f, 0.0f };

    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;

    Color finalColor = evaluateBlinnPhong(in, u);

    EXPECT_NEAR(finalColor.r, 0.2f, 1e-4f);
    EXPECT_NEAR(finalColor.g, 0.0f, 1e-4f);
    EXPECT_NEAR(finalColor.b, 0.0f, 1e-4f);
}

TEST(ShaderTests, DiffuseLightingFullyAligned)
{
    Material mat;
    mat.diffuse = Color{ 0.0f, 1.0f, 0.0f, 1.0f };
    mat.ambient = Color{ 0.0f, 0.0f, 0.0f, 1.0f };

    Light light;
    light.direction = Maths::Vec3f{ 0.0f, 1.0f, 0.0f };
    light.ambient = 0.0f;

    FragmentInput in;
    in.normal = Maths::Vec3f{ 0.0f, 1.0f, 0.0f };
    in.worldPos = Maths::Vec3f{ 0.0f, 0.0f, 0.0f };

    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;

    Color finalColor = evaluateBlinnPhong(in, u);

    EXPECT_NEAR(finalColor.r, 0.0f, 1e-4f);
    EXPECT_NEAR(finalColor.g, 1.0f, 1e-4f);
    EXPECT_NEAR(finalColor.b, 0.0f, 1e-4f);
}

TEST(ShaderTests, SpecularHighlight)
{
    Material mat;
    mat.diffuse = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
    mat.ambient = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
    mat.specular = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    mat.shininess = 16.0f;

    Light light;
    light.direction = Maths::Normalize(Maths::Vec3f{ 0.0f, 1.0f, 1.0f });
    light.ambient = 0.0f;

    FragmentInput in;
    in.normal = Maths::Vec3f{ 0.0f, 1.0f, 0.0f };
    in.worldPos = Maths::Vec3f{ 0.0f, 0.0f, 0.0f };

    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;
    u.cameraPos = Maths::Vec3f{ 0.0f, 1.0f, 1.0f };

    Color finalColor = evaluateBlinnPhong(in, u);

    EXPECT_GT(finalColor.r, 0.0f);
    EXPECT_GT(finalColor.g, 0.0f);
    EXPECT_GT(finalColor.b, 0.0f);
}

TEST(ShaderTests, DissolveAffectsFinalAlpha)
{
    Material mat;
    mat.diffuse = Color{ 1.0f, 1.0f, 1.0f, 0.8f };
    mat.dissolve = 0.5f;

    Light light;
    FragmentInput in;
    in.normal = Maths::Vec3f{ 0.0f, 1.0f, 0.0f };

    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;

    Color finalColor = evaluateBlinnPhong(in, u);

    EXPECT_NEAR(finalColor.a, 0.4f, 1e-4f);
}