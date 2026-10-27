#pragma once

#include <algorithm>
#include <cstdint>

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    constexpr Color() = default;
    constexpr Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    Color operator+(const Color& other) const {
        return { r + other.r, g + other.g, b + other.b, a + other.a };
    }

    Color& operator+=(const Color& other) {
        r += other.r; g += other.g; b += other.b; a += other.a;
        return *this;
    }

    Color operator*(float factor) const {
        return { r * factor, g * factor, b * factor, a };
    }

    Color operator*(const Color& other) const {
        return { r * other.r, g * other.g, b * other.b, a * other.a };
    }

    friend Color operator*(float factor, const Color& color) {
        return color * factor;
    }
    uint32_t toRGBA32() const {
        uint8_t ur = static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f);
        uint8_t ug = static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f);
        uint8_t ub = static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f);
        uint8_t ua = static_cast<uint8_t>(std::clamp(a, 0.0f, 1.0f) * 255.0f);

        return (static_cast<uint32_t>(ua) << 24) |
            (static_cast<uint32_t>(ur) << 16) |
            (static_cast<uint32_t>(ug) << 8) |
            static_cast<uint32_t>(ub);
    }

    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color White;
    static const Color Black;
    static const Color Gray;
};

inline constexpr Color Color::Red{ 1.0f, 0.0f, 0.0f, 1.0f };
inline constexpr Color Color::Green{ 0.0f, 1.0f, 0.0f, 1.0f };
inline constexpr Color Color::Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
inline constexpr Color Color::White{ 1.0f, 1.0f, 1.0f, 1.0f };
inline constexpr Color Color::Black{ 0.0f, 0.0f, 0.0f, 1.0f };
inline constexpr Color Color::Gray{ 0.5f, 0.5f, 0.5f, 1.0f };




struct Color8
{
    __m256 r, g, b, a;

    constexpr Color8() = default;
    constexpr Color8(__m256 r, __m256 g, __m256 b, __m256 a) : r(r), g(g), b(b), a(a) {}

    Color8 operator+(const Color8& other) const
    {
        return { _mm256_add_ps(r, other.r), _mm256_add_ps(g, other.g), _mm256_add_ps(b, other.b), _mm256_add_ps(a, other.a) };
    }

    Color8& operator+=(const Color8& other)
    {
        r = _mm256_add_ps(r, other.r);
        g = _mm256_add_ps(g, other.g);
        b = _mm256_add_ps(b, other.b);
        a = _mm256_add_ps(a, other.a);
        return *this;
    }

    Color8 operator*(__m256 factor) const {
        return { _mm256_mul_ps(r, factor), _mm256_mul_ps(g, factor), _mm256_mul_ps(b, factor),a };
    }

    Color8 operator*(const Color8& other) const {
        return { _mm256_mul_ps(r, other.r), _mm256_mul_ps(g, other.g), _mm256_mul_ps(b, other.b), _mm256_mul_ps(a, other.a) };
    }

    friend Color8 operator*(__m256 factor, const Color8& color) {
        return color * factor;
    }


    __m256i toRGBA32_8() const
    {
        __m256i clampedR = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_min_ps(_mm256_max_ps(r, _mm256_setzero_ps()), _mm256_set1_ps(1.0f)), _mm256_set1_ps(255.0f)));
        __m256i clampedG = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_min_ps(_mm256_max_ps(g, _mm256_setzero_ps()), _mm256_set1_ps(1.0f)), _mm256_set1_ps(255.0f)));
        __m256i clampedB = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_min_ps(_mm256_max_ps(b, _mm256_setzero_ps()), _mm256_set1_ps(1.0f)), _mm256_set1_ps(255.0f)));
        __m256i clampedA = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_min_ps(_mm256_max_ps(a, _mm256_setzero_ps()), _mm256_set1_ps(1.0f)), _mm256_set1_ps(255.0f)));

        __m256i aInt = _mm256_slli_epi32( clampedA,24);
        __m256i rInt = _mm256_slli_epi32( clampedR,16);
        __m256i gInt = _mm256_slli_epi32( clampedG,8);


        __m256i result = _mm256_or_si256(_mm256_or_si256(aInt, rInt), _mm256_or_si256(gInt, clampedB));

        return result;
    }

};


