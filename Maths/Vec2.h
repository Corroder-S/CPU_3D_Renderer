#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>

namespace Maths
{
// ============================================================================
// VECTEUR 2D
// ============================================================================

/// @brief Vecteur 2D générique.
/// @tparam T Type des composantes (float par défaut).
template <typename T = float>
class Vec2
{
public:
    T x;
    T y;

    Vec2();
    Vec2(T _x, T _y);
    /// @brief Conversion explicite depuis un Vec2 d'un autre type.
    template <typename U>
    explicit Vec2(const Vec2<U>& other);
    /// @brief Conversion depuis tout type possédant x et y (ex. Vec3 — ignore z).
    template <typename U>
    explicit Vec2(const U& v);

    Vec2 operator+(const Vec2& rhs) const;
    Vec2 operator-(const Vec2& rhs) const;
    Vec2 operator*(const Vec2& rhs) const;
    /// @brief Division composante par composante. Assert si un dénominateur est nul.
    Vec2 operator/(const Vec2& rhs) const;
    Vec2 operator-() const;
    Vec2 operator*(T scalar) const;
    /// @brief Division par scalaire. Assert si le scalaire est nul.
    Vec2 operator/(T scalar) const;
    Vec2& operator+=(const Vec2& rhs);
    Vec2& operator-=(const Vec2& rhs);
    Vec2& operator*=(const Vec2& rhs);
    Vec2& operator/=(const Vec2& rhs);
    Vec2& operator*=(T scalar);
    Vec2& operator/=(T scalar);
    bool operator==(const Vec2& rhs) const;
    bool operator!=(const Vec2& rhs) const;

    /// @brief Carré de la magnitude (évite une racine carrée).
    T MagnitudeSquared() const;
    /// @brief Magnitude (norme euclidienne).
    T Magnitude() const;

    static const Vec2 Zero;
    static const Vec2 One;
    static const Vec2 UnitX;
    static const Vec2 UnitY;
};

// ============================================================================
// CONSTRUCTEURS
// ============================================================================

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

// ============================================================================
// OPÉRATEURS
// ============================================================================

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

/// @brief Multiplication scalaire commutative (scalar * vec).
template <typename T>
Vec2<T> operator*(T scalar, const Vec2<T>& v) { return v * scalar; }

// ============================================================================
// MÉTHODES D'INSTANCE
// ============================================================================

template <typename T>
T Vec2<T>::MagnitudeSquared() const { return x*x + y*y; }

template <typename T>
T Vec2<T>::Magnitude() const { return std::hypot(x, y); }

// ============================================================================
// CONSTANTES STATIQUES
// ============================================================================

template <typename T> const Vec2<T> Vec2<T>::Zero  {0, 0};
template <typename T> const Vec2<T> Vec2<T>::One   {1, 1};
template <typename T> const Vec2<T> Vec2<T>::UnitX {1, 0};
template <typename T> const Vec2<T> Vec2<T>::UnitY {0, 1};

// ============================================================================
// FONCTIONS LIBRES — ARITHMÉTIQUE
// ============================================================================

/// @brief Produit scalaire.
template <typename T>
T Dot(const Vec2<T>& a, const Vec2<T>& b)
{
    return a.x*b.x + a.y*b.y;
}

/// @brief Produit vectoriel 2D — retourne la composante z de l'équivalent 3D.
/// Utile pour les fonctions d'arête, l'ordre d'enroulement et le signe de rotation.
template <typename T>
T Cross(const Vec2<T>& a, const Vec2<T>& b)
{
    return a.x*b.y - a.y*b.x;
}

/// @brief Carré de la distance entre deux points.
template <typename T>
T DistanceSquared(const Vec2<T>& a, const Vec2<T>& b) { return (a - b).MagnitudeSquared(); }

/// @brief Minimum composante par composante.
template <typename T>
Vec2<T> Min(const Vec2<T>& a, const Vec2<T>& b)
{
    return {std::min(a.x, b.x), std::min(a.y, b.y)};
}

/// @brief Maximum composante par composante.
template <typename T>
Vec2<T> Max(const Vec2<T>& a, const Vec2<T>& b)
{
    return {std::max(a.x, b.x), std::max(a.y, b.y)};
}

// ============================================================================
// FONCTIONS LIBRES — VIRGULE FLOTTANTE UNIQUEMENT
// ============================================================================

/// @brief Distance euclidienne entre deux points.
template <std::floating_point T>
T Distance(const Vec2<T>& a, const Vec2<T>& b) { return (a - b).Magnitude(); }

/// @brief Normalise le vecteur (norme = 1). Utilise un pré-scaling pour éviter
///        les dépassements. Assert si le vecteur est nul ou non fini.
template <std::floating_point T>
Vec2<T> Normalize(const Vec2<T>& v)
{
    assert(std::isfinite(v.x) && std::isfinite(v.y) && "Cannot normalize non-finite components");
    const T scale = std::max(std::abs(v.x), std::abs(v.y));
    assert(scale != T{0} && "Cannot normalize the zero vector");
    const Vec2<T> scaled = v / scale;
    return scaled / scaled.Magnitude();
}

/// @brief Interpolation linéaire entre a et b selon t ∈ [0, 1].
template <std::floating_point T>
Vec2<T> Lerp(const Vec2<T>& a, const Vec2<T>& b, T t)
{
    return a * (T{1} - t) + b * t;
}

/// @brief Angle en radians entre deux vecteurs (non nécessairement normalisés).
template <std::floating_point T>
T Angle(const Vec2<T>& a, const Vec2<T>& b)
{
    return std::acos(std::clamp(Dot(Normalize(a), Normalize(b)), T{-1}, T{1}));
}

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
using Vec2i = Vec2<int>;
}
