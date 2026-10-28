#include <gtest/gtest.h>
#include "Renderer/Shader.h"
#include "Asset/Material.h"
#include "Asset/Light.h"

TEST(ShaderTests, NullUniformsReturnWhite)
{
    FragmentInput in;
    ShaderUniforms u;
    Maths::Vec3f L{ 0.0f, 0.0f, 1.0f };

    Color result = evaluateBlinnPhong(in, u, L);
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
    mat.specular = Color{ 0.0f, 0.0f, 0.0f, 0.0f };
    mat.shininess = 32.0f;
    mat.dissolve = 1.0f;

    Light light;
    light.direction = Maths::Vec3f{ 0.0f, 0.0f, 1.0f };
    light.ambient = 0.2f;

    FragmentInput in;
    in.normal = Maths::Vec3f{ 0.0f, 0.0f, -1.0f };
    in.worldPos = Maths::Vec3f{ 0.0f, 0.0f, 0.0f };

    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;

    Color finalColor = evaluateBlinnPhong(in, u, light.direction);

    EXPECT_NEAR(finalColor.r, 0.2f, 1e-4f);
    EXPECT_NEAR(finalColor.g, 0.0f, 1e-4f);
    EXPECT_NEAR(finalColor.b, 0.0f, 1e-4f);
}

TEST(ShaderTests, DiffuseLightingFullyAligned)
{
    Material mat;
    mat.diffuse = Color{ 0.0f, 1.0f, 0.0f, 1.0f };
    mat.ambient = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
    mat.specular = Color{ 0.0f, 0.0f, 0.0f, 0.0f };
    mat.shininess = 32.0f;
    mat.dissolve = 1.0f;

    Light light;
    light.direction = Maths::Vec3f{ 0.0f, 1.0f, 0.0f };
    light.ambient = 0.0f;

    FragmentInput in;
    in.normal = Maths::Vec3f{ 0.0f, 1.0f, 0.0f };
    in.worldPos = Maths::Vec3f{ 0.0f, 0.0f, 0.0f };

    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;
    u.cameraPos = Maths::Vec3f{ 0.0f, 0.0f, 5.0f };

    Color finalColor = evaluateBlinnPhong(in, u, light.direction);

    EXPECT_NEAR(finalColor.r, 0.0f, 1e-4f);
    EXPECT_NEAR(finalColor.g, 1.0f, 1e-4f);
    EXPECT_NEAR(finalColor.b, 0.0f, 1e-4f);
    EXPECT_NEAR(finalColor.a, 1.0f, 1e-4f);
}

TEST(ShaderTests, SpecularHighlight)
{
    Material mat;
    mat.diffuse = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
    mat.ambient = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
    mat.specular = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    mat.shininess = 16.0f;
    mat.dissolve = 1.0f;

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

    Color finalColor = evaluateBlinnPhong(in, u, light.direction);

    EXPECT_GT(finalColor.r, 0.0f);
    EXPECT_GT(finalColor.g, 0.0f);
    EXPECT_GT(finalColor.b, 0.0f);
}

TEST(ShaderTests, DissolveAffectsFinalAlpha)
{
    Material mat;
    mat.diffuse = Color{ 1.0f, 1.0f, 1.0f, 0.8f };
    mat.specular = Color{ 0.0f, 0.0f, 0.0f, 0.0f };
    mat.shininess = 32.0f;
    mat.dissolve = 0.5f;

    Light light;
    light.direction = Maths::Vec3f{ 0.0f, 1.0f, 0.0f };

    FragmentInput in;
    in.normal = Maths::Vec3f{ 0.0f, 1.0f, 0.0f };
    in.worldPos = Maths::Vec3f{ 0.0f, 0.0f, 0.0f };

    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;

    Color finalColor = evaluateBlinnPhong(in, u, light.direction);

    EXPECT_NEAR(finalColor.a, 0.4f, 1e-4f);
}

TEST(ShaderTests, SIMD8_DiffuseLightingFullyAligned)
{
    Material mat;
    mat.diffuse = Color{ 0.0f, 1.0f, 0.0f, 1.0f };
    mat.ambient = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
    mat.specular = Color{ 0.0f, 0.0f, 0.0f, 0.0f };
    mat.shininess = 32.0f;
    mat.dissolve = 1.0f;

    Light light;
    light.direction = Maths::Vec3f{ 0.0f, 1.0f, 0.0f };
    light.ambient = 0.0f;

    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;
    u.cameraPos = Maths::Vec3f{ 0.0f, 0.0f, 5.0f };

    FragmentInput_8 in8{};
    in8.normal.y = _mm256_set1_ps(1.0f);

    Maths::Vec3<__m256> L8 = { _mm256_set1_ps(0.0f), _mm256_set1_ps(1.0f), _mm256_set1_ps(0.0f) };
    Maths::Vec3<__m256> C8 = { _mm256_set1_ps(0.0f), _mm256_set1_ps(0.0f), _mm256_set1_ps(5.0f) };

    Color8 result8 = evaluateBlinnPhong_8(in8, u, L8, C8);

    alignas(32) float r[8], g[8], b[8], a[8];
    _mm256_store_ps(r, result8.r);
    _mm256_store_ps(g, result8.g);
    _mm256_store_ps(b, result8.b);
    _mm256_store_ps(a, result8.a);

    for (int i = 0; i < 8; ++i)
    {
        EXPECT_NEAR(r[i], 0.0f, 1e-3f);
        EXPECT_NEAR(g[i], 1.0f, 1e-3f);
        EXPECT_NEAR(b[i], 0.0f, 1e-3f);
        EXPECT_NEAR(a[i], 1.0f, 1e-3f);
    }
}

TEST(ShaderTests, SIMD8_NullUniformsReturnWhite)
{
    FragmentInput_8 in8{};
    ShaderUniforms u;
    Maths::Vec3<__m256> L8 = { _mm256_setzero_ps(), _mm256_setzero_ps(), _mm256_set1_ps(1.0f) };
    Maths::Vec3<__m256> C8 = { _mm256_setzero_ps(), _mm256_setzero_ps(), _mm256_setzero_ps() };

    Color8 result8 = evaluateBlinnPhong_8(in8, u, L8, C8);

    alignas(32) float r[8], g[8], b[8], a[8];
    _mm256_store_ps(r, result8.r);
    _mm256_store_ps(g, result8.g);
    _mm256_store_ps(b, result8.b);
    _mm256_store_ps(a, result8.a);

    for (int i = 0; i < 8; ++i)
    {
        EXPECT_FLOAT_EQ(r[i], 1.0f);
        EXPECT_FLOAT_EQ(g[i], 1.0f);
        EXPECT_FLOAT_EQ(b[i], 1.0f);
        EXPECT_FLOAT_EQ(a[i], 1.0f);
    }
}

TEST(ShaderTests, SIMD8_AmbientLightingWhenSurfaceFacesAway)
{
    Material mat;
    mat.diffuse = Color{ 1.0f, 0.0f, 0.0f, 1.0f };
    mat.ambient = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    mat.specular = Color{ 0.0f, 0.0f, 0.0f, 0.0f };
    mat.shininess = 32.0f;
    mat.dissolve = 1.0f;

    Light light;
    light.direction = Maths::Vec3f{ 0.0f, 0.0f, 1.0f };
    light.ambient = 0.2f;

    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;

    FragmentInput_8 in8{};
    in8.normal.z = _mm256_set1_ps(-1.0f);

    Maths::Vec3<__m256> L8 = { _mm256_setzero_ps(), _mm256_setzero_ps(), _mm256_set1_ps(1.0f) };
    Maths::Vec3<__m256> C8 = { _mm256_setzero_ps(), _mm256_setzero_ps(), _mm256_setzero_ps() };

    Color8 result8 = evaluateBlinnPhong_8(in8, u, L8, C8);

    alignas(32) float r[8], g[8], b[8], a[8];
    _mm256_store_ps(r, result8.r);
    _mm256_store_ps(g, result8.g);
    _mm256_store_ps(b, result8.b);
    _mm256_store_ps(a, result8.a);

    for (int i = 0; i < 8; ++i)
    {
        EXPECT_NEAR(r[i], 0.2f, 1e-4f);
        EXPECT_NEAR(g[i], 0.0f, 1e-4f);
        EXPECT_NEAR(b[i], 0.0f, 1e-4f);
        EXPECT_NEAR(a[i], 1.0f, 1e-4f);
    }
}

TEST(ShaderTests, SIMD8_SpecularHighlightAndShininess)
{
    Material mat;
    mat.diffuse = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
    mat.ambient = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
    mat.specular = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    mat.shininess = 16.0f;
    mat.dissolve = 1.0f;

    Light light;
    light.ambient = 0.0f;

    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;

    FragmentInput_8 in8{};
    in8.normal.y = _mm256_set1_ps(1.0f);

    float invLen = 1.0f / std::sqrt(2.0f);
    Maths::Vec3<__m256> L8 = { _mm256_set1_ps(0.0f), _mm256_set1_ps(invLen), _mm256_set1_ps(invLen) };
    Maths::Vec3<__m256> C8 = { _mm256_set1_ps(0.0f), _mm256_set1_ps(1.0f), _mm256_set1_ps(1.0f) };

    Color8 result8 = evaluateBlinnPhong_8(in8, u, L8, C8);

    alignas(32) float r[8], g[8], b[8];
    _mm256_store_ps(r, result8.r);
    _mm256_store_ps(g, result8.g);
    _mm256_store_ps(b, result8.b);

    for (int i = 0; i < 8; ++i)
    {
        EXPECT_GT(r[i], 0.0f);
        EXPECT_GT(g[i], 0.0f);
        EXPECT_GT(b[i], 0.0f);
    }
}

TEST(ShaderTests, SIMD8_DissolveAffectsFinalAlpha)
{
    Material mat;
    mat.diffuse = Color{ 1.0f, 1.0f, 1.0f, 0.8f };
    mat.specular = Color{ 0.0f, 0.0f, 0.0f, 0.0f };
    mat.shininess = 32.0f;
    mat.dissolve = 0.5f;

    Light light;
    ShaderUniforms u;
    u.material = &mat;
    u.light = &light;

    FragmentInput_8 in8{};
    in8.normal.y = _mm256_set1_ps(1.0f);

    Maths::Vec3<__m256> L8 = { _mm256_setzero_ps(), _mm256_set1_ps(1.0f), _mm256_setzero_ps() };
    Maths::Vec3<__m256> C8 = { _mm256_setzero_ps(), _mm256_setzero_ps(), _mm256_setzero_ps() };

    Color8 result8 = evaluateBlinnPhong_8(in8, u, L8, C8);

    alignas(32) float a[8];
    _mm256_store_ps(a, result8.a);

    for (int i = 0; i < 8; ++i)
    {
        EXPECT_NEAR(a[i], 0.4f, 1e-4f);
    }
}