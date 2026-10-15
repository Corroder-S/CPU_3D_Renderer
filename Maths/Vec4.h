#pragma once

#include "Vec3.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>

namespace Maths
{
    template <std::floating_point T = float>
    class Vec4
    {
    public:
        T x;
        T y;
        T z;
        T w;

        Vec4();
        Vec4(T _x, T _y, T _z, T _w);
        Vec4(const Vec3<T>& v, T _w);
        template <std::floating_point U>
        explicit Vec4(const Vec4<U>& other);

        Vec4 operator+(const Vec4& rhs) const;
        Vec4 operator-(const Vec4& rhs) const;
        Vec4 operator*(const Vec4& rhs) const;
        Vec4 operator/(const Vec4& rhs) const;
        Vec4 operator-() const;
        Vec4 operator*(T scalar) const;
        Vec4 operator/(T scalar) const;
        Vec4& operator+=(const Vec4& rhs);
        Vec4& operator-=(const Vec4& rhs);
        Vec4& operator*=(const Vec4& rhs);
        Vec4& operator/=(const Vec4& rhs);
        Vec4& operator*=(T scalar);
        Vec4& operator/=(T scalar);
        bool operator==(const Vec4& rhs) const;
        bool operator!=(const Vec4& rhs) const;

        T MagnitudeSquared() const;
        T Magnitude() const;
        Vec3<T> xyz() const;

        static const Vec4 Zero;
        static const Vec4 One;
        static const Vec4 UnitX;
        static const Vec4 UnitY;
        static const Vec4 UnitZ;
        static const Vec4 UnitW;
    };

    // --- Constructors ---

    template <std::floating_point T>
    Vec4<T>::Vec4() : x(0), y(0), z(0), w(0) {}

    template <std::floating_point T>
    Vec4<T>::Vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}

    template <std::floating_point T>
    Vec4<T>::Vec4(const Vec3<T>& v, T _w) : x(v.x), y(v.y), z(v.z), w(_w) {}

    template <std::floating_point T>
    template <std::floating_point U>
    Vec4<T>::Vec4(const Vec4<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)),
          z(static_cast<T>(other.z)), w(static_cast<T>(other.w)) {}

    // --- Operators ---

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator+(const Vec4& rhs) const
    {
        return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator-(const Vec4& rhs) const
    {
        return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator*(const Vec4& rhs) const
    {
        return {x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator/(const Vec4& rhs) const
    {
        assert(rhs.x != T{0} && rhs.y != T{0} && rhs.z != T{0} && rhs.w != T{0} && "Cannot divide by a zero component");
        return {x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator-() const { return {-x, -y, -z, -w}; }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator*(T scalar) const
    {
        return {x * scalar, y * scalar, z * scalar, w * scalar};
    }

    template <std::floating_point T>
    Vec4<T> Vec4<T>::operator/(T scalar) const
    {
        assert(scalar != T{0} && "Cannot divide by zero");
        return {x / scalar, y / scalar, z / scalar, w / scalar};
    }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator+=(const Vec4& rhs) { *this = *this + rhs; return *this; }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator-=(const Vec4& rhs) { *this = *this - rhs; return *this; }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator*=(const Vec4& rhs) { *this = *this * rhs; return *this; }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator/=(const Vec4& rhs) { *this = *this / rhs; return *this; }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator*=(T scalar) { *this = *this * scalar; return *this; }

    template <std::floating_point T>
    Vec4<T>& Vec4<T>::operator/=(T scalar) { *this = *this / scalar; return *this; }

    template <std::floating_point T>
    bool Vec4<T>::operator==(const Vec4& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    template <std::floating_point T>
    bool Vec4<T>::operator!=(const Vec4& rhs) const { return !(*this == rhs); }

    template <std::floating_point T>
    Vec4<T> operator*(T scalar, const Vec4<T>& v) { return v * scalar; }

    // --- Instance methods (single-object operations) ---

    template <std::floating_point T>
    T Vec4<T>::MagnitudeSquared() const { return x*x + y*y + z*z + w*w; }

    template <std::floating_point T>
    T Vec4<T>::Magnitude() const { return std::hypot(std::hypot(x, y), std::hypot(z, w)); }

    template <std::floating_point T>
    Vec3<T> Vec4<T>::xyz() const { return {x, y, z}; }

    // --- Static constants ---

    template <std::floating_point T> const Vec4<T> Vec4<T>::Zero  {0, 0, 0, 0};
    template <std::floating_point T> const Vec4<T> Vec4<T>::One   {1, 1, 1, 1};
    template <std::floating_point T> const Vec4<T> Vec4<T>::UnitX {1, 0, 0, 0};
    template <std::floating_point T> const Vec4<T> Vec4<T>::UnitY {0, 1, 0, 0};
    template <std::floating_point T> const Vec4<T> Vec4<T>::UnitZ {0, 0, 1, 0};
    template <std::floating_point T> const Vec4<T> Vec4<T>::UnitW {0, 0, 0, 1};

    // --- Free functions ---

    template <std::floating_point T>
    T Dot(const Vec4<T>& a, const Vec4<T>& b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    }

    template <std::floating_point T>
    Vec4<T> Normalize(const Vec4<T>& v)
    {
        assert(std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) && std::isfinite(v.w) && "Cannot normalize non-finite components");
        const T scale = std::max({std::abs(v.x), std::abs(v.y), std::abs(v.z), std::abs(v.w)});
        assert(scale != T{0} && "Cannot normalize the zero vector");
        const Vec4<T> scaled = v / scale;
        return scaled / scaled.Magnitude();
    }

    template <std::floating_point T>
    T DistanceSquared(const Vec4<T>& a, const Vec4<T>& b) { return (a - b).MagnitudeSquared(); }

    template <std::floating_point T>
    T Distance(const Vec4<T>& a, const Vec4<T>& b) { return (a - b).Magnitude(); }

    template <std::floating_point T>
    T Angle(const Vec4<T>& a, const Vec4<T>& b)
    {
        return std::acos(std::clamp(Dot(Normalize(a), Normalize(b)), T{-1}, T{1}));
    }

    template <std::floating_point T>
    Vec4<T> Lerp(const Vec4<T>& a, const Vec4<T>& b, T t)
    {
        return a * (T{1} - t) + b * t;
    }

    template <std::floating_point T>
    Vec4<T> Min(const Vec4<T>& a, const Vec4<T>& b)
    {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w)};
    }

    template <std::floating_point T>
    Vec4<T> Max(const Vec4<T>& a, const Vec4<T>& b)
    {
        return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w)};
    }

    using Vec4f = Vec4<float>;
    using Vec4d = Vec4<double>;
}
