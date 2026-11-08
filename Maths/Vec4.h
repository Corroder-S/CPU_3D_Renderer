#pragma once

#include "Vec3.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <stdexcept>

namespace Maths
{
// ============================================================================
// VECTEUR 4D
// ============================================================================

/// @brief Vecteur 4D générique (position homogène ou RGBA).
/// @tparam T Type des composantes (float par défaut).
template <typename T = float>
class Vec4
{
public:
    T x;
    T y;
    T z;
    T w;

    Vec4();
    Vec4(T _x, T _y, T _z, T _w);
    /// @brief Construit depuis un Vec3 en ajoutant la composante w.
    Vec4(const Vec3<T>& v, T _w);
    /// @brief Conversion explicite depuis un Vec4 d'un autre type.
    template <typename U>
    explicit Vec4(const Vec4<U>& other);

    Vec4 operator+(const Vec4& rhs) const;
    Vec4 operator-(const Vec4& rhs) const;
    Vec4 operator*(const Vec4& rhs) const;
    /// @brief Division composante par composante. Lève std::domain_error si un dénominateur est nul.
    Vec4 operator/(const Vec4& rhs) const;
    Vec4 operator-() const;
    Vec4 operator*(T scalar) const;
    /// @brief Division par scalaire. Lève std::domain_error si le scalaire est nul.
    Vec4 operator/(T scalar) const;
    Vec4& operator+=(const Vec4& rhs);
    Vec4& operator-=(const Vec4& rhs);
    Vec4& operator*=(const Vec4& rhs);
    Vec4& operator/=(const Vec4& rhs);
    Vec4& operator*=(T scalar);
    Vec4& operator/=(T scalar);
    bool operator==(const Vec4& rhs) const;
    bool operator!=(const Vec4& rhs) const;

    /// @brief Carré de la magnitude (évite une racine carrée).
    T MagnitudeSquared() const;
    /// @brief Magnitude (norme euclidienne).
    T Magnitude() const;

    /// @brief Extrait les trois premières composantes.
    Vec3<T> xyz() const;

    /// @brief Produit scalaire avec rhs.
    T Dot(const Vec4& rhs) const;
    /// @brief Vecteur normalisé. No-op pour les types entiers.
    Vec4 Normalize() const;
    /// @brief Carré de la distance avec rhs.
    T DistanceSquared(const Vec4& rhs) const;
    /// @brief Distance euclidienne avec rhs. Retourne 0 pour les types entiers.
    T Distance(const Vec4& rhs) const;
    /// @brief Angle en radians avec rhs. Retourne 0 pour les types entiers.
    T Angle(const Vec4& rhs) const;

    static Vec4 Min(const Vec4& a, const Vec4& b);
    static Vec4 Max(const Vec4& a, const Vec4& b);
    /// @brief Interpolation linéaire entre a et b. No-op (retourne a) pour les types entiers.
    static Vec4 Lerp(const Vec4& a, const Vec4& b, T t);

    static const Vec4 Zero;
    static const Vec4 One;
    static const Vec4 UnitX;
    static const Vec4 UnitY;
    static const Vec4 UnitZ;
    static const Vec4 UnitW;
};

// ============================================================================
// CONSTRUCTEURS
// ============================================================================

template <typename T>
Vec4<T>::Vec4() : x(0), y(0), z(0), w(0) {}

template <typename T>
Vec4<T>::Vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}

template <typename T>
Vec4<T>::Vec4(const Vec3<T>& v, T _w) : x(v.x), y(v.y), z(v.z), w(_w) {}

template <typename T>
template <typename U>
Vec4<T>::Vec4(const Vec4<U>& other)
    : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)),
    z(static_cast<T>(other.z)), w(static_cast<T>(other.w)) {
}

// ============================================================================
// OPÉRATEURS
// ============================================================================

template <typename T>
Vec4<T> Vec4<T>::operator+(const Vec4& rhs) const
{
    return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
}

template <typename T>
Vec4<T> Vec4<T>::operator-(const Vec4& rhs) const
{
    return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
}

template <typename T>
Vec4<T> Vec4<T>::operator*(const Vec4& rhs) const
{
    return { x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w };
}

template <typename T>
Vec4<T> Vec4<T>::operator/(const Vec4& rhs) const
{
    if (rhs.x == T{ 0 } || rhs.y == T{ 0 } || rhs.z == T{ 0 } || rhs.w == T{ 0 })
        throw std::domain_error("Cannot divide by a zero component");
    return { x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w };
}

template <typename T>
Vec4<T> Vec4<T>::operator-() const { return { -x, -y, -z, -w }; }

template <typename T>
Vec4<T> Vec4<T>::operator*(T scalar) const
{
    return { x * scalar, y * scalar, z * scalar, w * scalar };
}

template <typename T>
Vec4<T> Vec4<T>::operator/(T scalar) const
{
    if (scalar == T{ 0 })
        throw std::domain_error("Cannot divide by zero");
    return { x / scalar, y / scalar, z / scalar, w / scalar };
}

template <typename T>
Vec4<T>& Vec4<T>::operator+=(const Vec4& rhs) { *this = *this + rhs; return *this; }

template <typename T>
Vec4<T>& Vec4<T>::operator-=(const Vec4& rhs) { *this = *this - rhs; return *this; }

template <typename T>
Vec4<T>& Vec4<T>::operator*=(const Vec4& rhs) { *this = *this * rhs; return *this; }

template <typename T>
Vec4<T>& Vec4<T>::operator/=(const Vec4& rhs) { *this = *this / rhs; return *this; }

template <typename T>
Vec4<T>& Vec4<T>::operator*=(T scalar) { *this = *this * scalar; return *this; }

template <typename T>
Vec4<T>& Vec4<T>::operator/=(T scalar) { *this = *this / scalar; return *this; }

template <typename T>
bool Vec4<T>::operator==(const Vec4& rhs) const
{
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

template <typename T>
bool Vec4<T>::operator!=(const Vec4& rhs) const { return !(*this == rhs); }

/// @brief Multiplication scalaire commutative (scalar * vec).
template <typename T>
Vec4<T> operator*(T scalar, const Vec4<T>& v) { return v * scalar; }

// ============================================================================
// MÉTHODES D'INSTANCE
// ============================================================================

template <typename T>
T Vec4<T>::MagnitudeSquared() const { return x * x + y * y + z * z + w * w; }

/// @brief Calcule la magnitude via deux hypot imbriqués pour limiter le débordement.
template <typename T>
T Vec4<T>::Magnitude() const { return std::hypot(std::hypot(x, y), std::hypot(z, w)); }

template <typename T>
Vec3<T> Vec4<T>::xyz() const { return { x, y, z }; }

// ============================================================================
// CONSTANTES STATIQUES
// ============================================================================

template <typename T> const Vec4<T> Vec4<T>::Zero{ 0, 0, 0, 0 };
template <typename T> const Vec4<T> Vec4<T>::One{ 1, 1, 1, 1 };
template <typename T> const Vec4<T> Vec4<T>::UnitX{ 1, 0, 0, 0 };
template <typename T> const Vec4<T> Vec4<T>::UnitY{ 0, 1, 0, 0 };
template <typename T> const Vec4<T> Vec4<T>::UnitZ{ 0, 0, 1, 0 };
template <typename T> const Vec4<T> Vec4<T>::UnitW{ 0, 0, 0, 1 };

// ============================================================================
// FONCTIONS LIBRES — ARITHMÉTIQUE
// ============================================================================

/// @brief Produit scalaire 4D.
template <typename T>
T Dot(const Vec4<T>& a, const Vec4<T>& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/// @brief Carré de la distance entre deux points.
template <typename T>
T DistanceSquared(const Vec4<T>& a, const Vec4<T>& b) { return (a - b).MagnitudeSquared(); }

/// @brief Minimum composante par composante.
template <typename T>
Vec4<T> Min(const Vec4<T>& a, const Vec4<T>& b)
{
    return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w) };
}

/// @brief Maximum composante par composante.
template <typename T>
Vec4<T> Max(const Vec4<T>& a, const Vec4<T>& b)
{
    return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w) };
}

// ============================================================================
// FONCTIONS LIBRES — VIRGULE FLOTTANTE UNIQUEMENT
// ============================================================================

/// @brief Normalise le vecteur (norme = 1). Utilise un pré-scaling pour éviter
///        les dépassements. Lève std::domain_error si le vecteur est nul ou non fini.
template <std::floating_point T>
Vec4<T> Normalize(const Vec4<T>& v)
{
    if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z) || !std::isfinite(v.w))
        throw std::domain_error("Cannot normalize non-finite components");
    const T scale = std::max({ std::abs(v.x), std::abs(v.y), std::abs(v.z), std::abs(v.w) });
    if (scale == T{ 0 })
        throw std::domain_error("Cannot normalize the zero vector");
    const Vec4<T> scaled = v / scale;
    return scaled / scaled.Magnitude();
}

/// @brief Distance euclidienne entre deux points.
template <std::floating_point T>
T Distance(const Vec4<T>& a, const Vec4<T>& b) { return (a - b).Magnitude(); }

/// @brief Angle en radians entre deux vecteurs (non nécessairement normalisés).
template <std::floating_point T>
T Angle(const Vec4<T>& a, const Vec4<T>& b)
{
    return std::acos(std::clamp(Dot(Normalize(a), Normalize(b)), T{ -1 }, T{ 1 }));
}

/// @brief Interpolation linéaire entre a et b selon t ∈ [0, 1].
template <std::floating_point T>
Vec4<T> Lerp(const Vec4<T>& a, const Vec4<T>& b, T t)
{
    return a * (T{ 1 } - t) + b * t;
}

// ============================================================================
// WRAPPERS MEMBRES ET STATIQUES (délèguent aux fonctions libres)
// ============================================================================

template <typename T>
T Vec4<T>::Dot(const Vec4& rhs) const { return Maths::Dot(*this, rhs); }

template <typename T>
Vec4<T> Vec4<T>::Normalize() const {
    if constexpr (std::floating_point<T>) return Maths::Normalize(*this);
    else return *this;
}

template <typename T>
T Vec4<T>::DistanceSquared(const Vec4& rhs) const { return Maths::DistanceSquared(*this, rhs); }

template <typename T>
T Vec4<T>::Distance(const Vec4& rhs) const {
    if constexpr (std::floating_point<T>) return Maths::Distance(*this, rhs);
    else return T{ 0 };
}

template <typename T>
T Vec4<T>::Angle(const Vec4& rhs) const {
    if constexpr (std::floating_point<T>) return Maths::Angle(*this, rhs);
    else return T{ 0 };
}

template <typename T>
Vec4<T> Vec4<T>::Min(const Vec4& a, const Vec4& b) { return Maths::Min(a, b); }

template <typename T>
Vec4<T> Vec4<T>::Max(const Vec4& a, const Vec4& b) { return Maths::Max(a, b); }

template <typename T>
Vec4<T> Vec4<T>::Lerp(const Vec4& a, const Vec4& b, T t) {
    if constexpr (std::floating_point<T>) return Maths::Lerp(a, b, t);
    else return a;
}

using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;
using Vec4i = Vec4<int>;
}
