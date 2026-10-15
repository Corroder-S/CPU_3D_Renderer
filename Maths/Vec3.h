#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>

namespace Maths
{
    template <std::floating_point T = float>
    class Vec3
    {
    public:
        T x;
        T y;
        T z;

        Vec3();
        Vec3(T _x, T _y, T _z);
        template <std::floating_point U>
        explicit Vec3(const Vec3<U>& other);
        // Conversion from any type with x, y, z (e.g. Vec4 — drops w)
        template <typename U>
        explicit Vec3(const U& v);

        Vec3 operator+(const Vec3& rhs) const;
        Vec3 operator-(const Vec3& rhs) const;
        Vec3 operator*(const Vec3& rhs) const;
        Vec3 operator/(const Vec3& rhs) const;
        Vec3 operator-() const;
        Vec3 operator*(T scalar) const;
        Vec3 operator/(T scalar) const;
        Vec3& operator+=(const Vec3& rhs);
        Vec3& operator-=(const Vec3& rhs);
        Vec3& operator*=(const Vec3& rhs);
        Vec3& operator/=(const Vec3& rhs);
        Vec3& operator*=(T scalar);
        Vec3& operator/=(T scalar);
        bool operator==(const Vec3& rhs) const;
        bool operator!=(const Vec3& rhs) const;

        T MagnitudeSquared() const;
        T Magnitude() const;

        static const Vec3 Zero;
        static const Vec3 One;
        static const Vec3 UnitX;
        static const Vec3 UnitY;
        static const Vec3 UnitZ;
    };

    // --- Constructors ---

    template <std::floating_point T>
    Vec3<T>::Vec3() : x(0), y(0), z(0) {}

    template <std::floating_point T>
    Vec3<T>::Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

    template <std::floating_point T>
    template <std::floating_point U>
    Vec3<T>::Vec3(const Vec3<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z)) {}

    template <std::floating_point T>
    template <typename U>
    Vec3<T>::Vec3(const U& v)
        : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z)) {}

    // --- Operators ---

    template <std::floating_point T>
    Vec3<T> Vec3<T>::operator+(const Vec3& rhs) const
    {
        return {x + rhs.x, y + rhs.y, z + rhs.z};
    }

    template <std::floating_point T>
    Vec3<T> Vec3<T>::operator-(const Vec3& rhs) const
    {
        return {x - rhs.x, y - rhs.y, z - rhs.z};
    }

    template <std::floating_point T>
    Vec3<T> Vec3<T>::operator*(const Vec3& rhs) const
    {
        return {x * rhs.x, y * rhs.y, z * rhs.z};
    }

    template <std::floating_point T>
    Vec3<T> Vec3<T>::operator/(const Vec3& rhs) const
    {
        assert(rhs.x != T{0} && rhs.y != T{0} && rhs.z != T{0} && "Cannot divide by a zero component");
        return {x / rhs.x, y / rhs.y, z / rhs.z};
    }

    template <std::floating_point T>
    Vec3<T> Vec3<T>::operator-() const
    {
        return {-x, -y, -z};
    }

    template <std::floating_point T>
    Vec3<T> Vec3<T>::operator*(T scalar) const
    {
        return {x * scalar, y * scalar, z * scalar};
    }

    template <std::floating_point T>
    Vec3<T> Vec3<T>::operator/(T scalar) const
    {
        assert(scalar != T{0} && "Cannot divide by zero");
        return {x / scalar, y / scalar, z / scalar};
    }

    template <std::floating_point T>
    Vec3<T>& Vec3<T>::operator+=(const Vec3& rhs) { *this = *this + rhs; return *this; }

    template <std::floating_point T>
    Vec3<T>& Vec3<T>::operator-=(const Vec3& rhs) { *this = *this - rhs; return *this; }

    template <std::floating_point T>
    Vec3<T>& Vec3<T>::operator*=(const Vec3& rhs) { *this = *this * rhs; return *this; }

    template <std::floating_point T>
    Vec3<T>& Vec3<T>::operator/=(const Vec3& rhs) { *this = *this / rhs; return *this; }

    template <std::floating_point T>
    Vec3<T>& Vec3<T>::operator*=(T scalar) { *this = *this * scalar; return *this; }

    template <std::floating_point T>
    Vec3<T>& Vec3<T>::operator/=(T scalar) { *this = *this / scalar; return *this; }

    template <std::floating_point T>
    bool Vec3<T>::operator==(const Vec3& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    template <std::floating_point T>
    bool Vec3<T>::operator!=(const Vec3& rhs) const { return !(*this == rhs); }

    template <std::floating_point T>
    Vec3<T> operator*(T scalar, const Vec3<T>& v) { return v * scalar; }

    // --- Instance methods (single-object operations) ---

    template <std::floating_point T>
    T Vec3<T>::MagnitudeSquared() const { return x*x + y*y + z*z; }

    template <std::floating_point T>
    T Vec3<T>::Magnitude() const { return std::hypot(x, y, z); }

    // --- Static constants ---

    template <std::floating_point T> const Vec3<T> Vec3<T>::Zero  {0, 0, 0};
    template <std::floating_point T> const Vec3<T> Vec3<T>::One   {1, 1, 1};
    template <std::floating_point T> const Vec3<T> Vec3<T>::UnitX {1, 0, 0};
    template <std::floating_point T> const Vec3<T> Vec3<T>::UnitY {0, 1, 0};
    template <std::floating_point T> const Vec3<T> Vec3<T>::UnitZ {0, 0, 1};

    // --- Free functions ---

    template <std::floating_point T>
    T Dot(const Vec3<T>& a, const Vec3<T>& b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }

    template <std::floating_point T>
    Vec3<T> Cross(const Vec3<T>& a, const Vec3<T>& b)
    {
        return {a.y*b.z - a.z*b.y,
                a.z*b.x - a.x*b.z,
                a.x*b.y - a.y*b.x};
    }

    template <std::floating_point T>
    Vec3<T> Normalize(const Vec3<T>& v)
    {
        assert(std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) && "Cannot normalize non-finite components");
        const T scale = std::max({std::abs(v.x), std::abs(v.y), std::abs(v.z)});
        assert(scale != T{0} && "Cannot normalize the zero vector");
        const Vec3<T> scaled = v / scale;
        return scaled / scaled.Magnitude();
    }

    template <std::floating_point T>
    T DistanceSquared(const Vec3<T>& a, const Vec3<T>& b) { return (a - b).MagnitudeSquared(); }

    template <std::floating_point T>
    T Distance(const Vec3<T>& a, const Vec3<T>& b) { return (a - b).Magnitude(); }

    template <std::floating_point T>
    T Angle(const Vec3<T>& a, const Vec3<T>& b)
    {
        return std::acos(std::clamp(Dot(Normalize(a), Normalize(b)), T{-1}, T{1}));
    }

    template <std::floating_point T>
    Vec3<T> Lerp(const Vec3<T>& a, const Vec3<T>& b, T t)
    {
        return a * (T{1} - t) + b * t;
    }

    template <std::floating_point T>
    Vec3<T> Min(const Vec3<T>& a, const Vec3<T>& b)
    {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
    }

    template <std::floating_point T>
    Vec3<T> Max(const Vec3<T>& a, const Vec3<T>& b)
    {
        return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
    }

    using Vec3f = Vec3<float>;
    using Vec3d = Vec3<double>;
}
