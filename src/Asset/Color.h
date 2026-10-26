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

    Color clamp() const
    {
        return Color{
            std::clamp(r, 0.0f, 1.0f),
            std::clamp(g, 0.0f, 1.0f),
            std::clamp(b, 0.0f, 1.0f),
            std::clamp(a, 0.0f, 1.0f)
        };
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