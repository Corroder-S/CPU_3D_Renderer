#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>

namespace Maths
{
    template <typename T = float>
    class Vec2
    {
    public:
        T x;
        T y;

        Vec2();
        Vec2(T _x, T _y);
        template <typename U>
        explicit Vec2(const Vec2<U>& other);
        // Conversion from any type with x, y (e.g. Vec3 — drops z)
        template <typename U>
        explicit Vec2(const U& v);

        Vec2 operator+(const Vec2& rhs) const;
        Vec2 operator-(const Vec2& rhs) const;
        Vec2 operator*(const Vec2& rhs) const;
        Vec2 operator/(const Vec2& rhs) const;
        Vec2 operator-() const;
        Vec2 operator*(T scalar) const;
        Vec2 operator/(T scalar) const;
        Vec2& operator+=(const Vec2& rhs);
        Vec2& operator-=(const Vec2& rhs);
        Vec2& operator*=(const Vec2& rhs);
        Vec2& operator/=(const Vec2& rhs);
        Vec2& operator*=(T scalar);
        Vec2& operator/=(T scalar);
        bool operator==(const Vec2& rhs) const;
        bool operator!=(const Vec2& rhs) const;

        T MagnitudeSquared() const;
        T Magnitude() const;

        static const Vec2 Zero;
        static const Vec2 One;
        static const Vec2 UnitX;
        static const Vec2 UnitY;
    };

    // --- Constructors ---

    template <typename T>
    Vec2<T>::Vec2() : x(0), y(0) {}

    template <typename T>
    Vec2<T>::Vec2(T _x, T _y) : x(_x), y(_y) {}

    template <typename T>
    template <typename U>
    Vec2<T>::Vec2(const Vec2<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

    template <typename T>
    template <typename U>
    Vec2<T>::Vec2(const U& v)
        : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) {}

    // --- Operators ---

    template <typename T>
    Vec2<T> Vec2<T>::operator+(const Vec2& rhs) const { return {x + rhs.x, y + rhs.y}; }

    template <typename T>
    Vec2<T> Vec2<T>::operator-(const Vec2& rhs) const { return {x - rhs.x, y - rhs.y}; }

    template <typename T>
    Vec2<T> Vec2<T>::operator*(const Vec2& rhs) const { return {x * rhs.x, y * rhs.y}; }

    template <typename T>
    Vec2<T> Vec2<T>::operator/(const Vec2& rhs) const
    {
        assert(rhs.x != T{0} && rhs.y != T{0} && "Cannot divide by a zero component");
        return {x / rhs.x, y / rhs.y};
    }

    template <typename T>
    Vec2<T> Vec2<T>::operator-() const { return {-x, -y}; }

    template <typename T>
    Vec2<T> Vec2<T>::operator*(T scalar) const { return {x * scalar, y * scalar}; }

    template <typename T>
    Vec2<T> Vec2<T>::operator/(T scalar) const
    {
        assert(scalar != T{0} && "Cannot divide by zero");
        return {x / scalar, y / scalar};
    }

    template <typename T>
    Vec2<T>& Vec2<T>::operator+=(const Vec2& rhs) { *this = *this + rhs; return *this; }

    template <typename T>
    Vec2<T>& Vec2<T>::operator-=(const Vec2& rhs) { *this = *this - rhs; return *this; }

    template <typename T>
    Vec2<T>& Vec2<T>::operator*=(const Vec2& rhs) { *this = *this * rhs; return *this; }

    template <typename T>
    Vec2<T>& Vec2<T>::operator/=(const Vec2& rhs) { *this = *this / rhs; return *this; }

    template <typename T>
    Vec2<T>& Vec2<T>::operator*=(T scalar) { *this = *this * scalar; return *this; }

    template <typename T>
    Vec2<T>& Vec2<T>::operator/=(T scalar) { *this = *this / scalar; return *this; }

    template <typename T>
    bool Vec2<T>::operator==(const Vec2& rhs) const { return x == rhs.x && y == rhs.y; }

    template <typename T>
    bool Vec2<T>::operator!=(const Vec2& rhs) const { return !(*this == rhs); }

    template <typename T>
    Vec2<T> operator*(T scalar, const Vec2<T>& v) { return v * scalar; }

    // --- Instance methods (single-object operations) ---

    template <typename T>
    T Vec2<T>::MagnitudeSquared() const { return x*x + y*y; }

    template <typename T>
    T Vec2<T>::Magnitude() const { return std::hypot(x, y); }

    // --- Static constants ---

    template <typename T> const Vec2<T> Vec2<T>::Zero  {0, 0};
    template <typename T> const Vec2<T> Vec2<T>::One   {1, 1};
    template <typename T> const Vec2<T> Vec2<T>::UnitX {1, 0};
    template <typename T> const Vec2<T> Vec2<T>::UnitY {0, 1};

    // --- Free functions (arithmetic) ---

    template <typename T>
    T Dot(const Vec2<T>& a, const Vec2<T>& b)
    {
        return a.x*b.x + a.y*b.y;
    }

    // 2D cross product — returns the scalar z of the equivalent 3D cross
    // Useful for edge functions, winding order, sign of rotation
    template <typename T>
    T Cross(const Vec2<T>& a, const Vec2<T>& b)
    {
        return a.x*b.y - a.y*b.x;
    }

    template <typename T>
    T DistanceSquared(const Vec2<T>& a, const Vec2<T>& b) { return (a - b).MagnitudeSquared(); }

    template <typename T>
    Vec2<T> Min(const Vec2<T>& a, const Vec2<T>& b)
    {
        return {std::min(a.x, b.x), std::min(a.y, b.y)};
    }

    template <typename T>
    Vec2<T> Max(const Vec2<T>& a, const Vec2<T>& b)
    {
        return {std::max(a.x, b.x), std::max(a.y, b.y)};
    }

    // --- Free functions (floating point only) ---

    template <std::floating_point T>
    T Distance(const Vec2<T>& a, const Vec2<T>& b) { return (a - b).Magnitude(); }

    template <std::floating_point T>
    Vec2<T> Normalize(const Vec2<T>& v)
    {
        assert(std::isfinite(v.x) && std::isfinite(v.y) && "Cannot normalize non-finite components");
        const T scale = std::max(std::abs(v.x), std::abs(v.y));
        assert(scale != T{0} && "Cannot normalize the zero vector");
        const Vec2<T> scaled = v / scale;
        return scaled / scaled.Magnitude();
    }

    template <std::floating_point T>
    Vec2<T> Lerp(const Vec2<T>& a, const Vec2<T>& b, T t)
    {
        return a * (T{1} - t) + b * t;
    }

    template <std::floating_point T>
    T Angle(const Vec2<T>& a, const Vec2<T>& b)
    {
        return std::acos(std::clamp(Dot(Normalize(a), Normalize(b)), T{-1}, T{1}));
    }

    using Vec2f = Vec2<float>;
    using Vec2d = Vec2<double>;
    using Vec2i = Vec2<int>;
}
