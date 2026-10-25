#pragma once
#include <cstdint>

namespace Maths
{
    template< typename T >
    inline float lerp(T a, T b, float t)
    {
        return a + (b - a) * t;
    }

    inline uint32_t lerpColor(uint32_t a, uint32_t b, float t)
    {
        uint8_t a_r = (a >> 16) & 0xFF;
        uint8_t a_g = (a >>  8) & 0xFF;
        uint8_t a_b = (a >>  0) & 0xFF;
        uint8_t a_a = (a >> 24) & 0xFF;

        uint8_t b_r = (b >> 16) & 0xFF;
        uint8_t b_g = (b >>  8) & 0xFF;
        uint8_t b_b = (b >>  0) & 0xFF;
        uint8_t b_a = (b >> 24) & 0xFF;

        uint8_t r = static_cast<uint8_t>(a_r + (b_r - a_r) * t);
        uint8_t g = static_cast<uint8_t>(a_g + (b_g - a_g) * t);
        uint8_t bv = static_cast<uint8_t>(a_b + (b_b - a_b) * t);
        uint8_t av = static_cast<uint8_t>(a_a + (b_a - a_a) * t);

        return (static_cast<uint32_t>(av) << 24) |
               (static_cast<uint32_t>(r)  << 16) |
               (static_cast<uint32_t>(g)  <<  8) |
               (static_cast<uint32_t>(bv));
    }
}
