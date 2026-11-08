#pragma once

#include "Vec4.h"
#include "Vec3.h"
#include <array>
#include <cassert>
#include <limits>
#include <utility>

namespace Maths
{
// ============================================================================
// MATRICE 4×4
// ============================================================================

/// @brief Matrice 4×4 générique (transformations affines et projectives).
/// Stockage ligne-major, vecteurs colonnes : résultat = matrice * vecteur.
/// A * B applique B en premier. Les angles sont en radians, rotations main droite.
/// @tparam T Type des éléments (float par défaut).
template <typename T = float>
class Matrix4x4
{
public:
    T values[4][4];

    Matrix4x4(); // Identité.
    /// @brief Construit depuis un tableau de 16 éléments en ordre ligne-major.
    explicit Matrix4x4(const std::array<T, 16>& elements);
    static Matrix4x4 Identity();
    static Matrix4x4 Zero();

    /// @brief Accès en écriture à l'élément (row, column). Lève std::out_of_range si hors limites.
    T& operator()(std::size_t row, std::size_t column);
    /// @brief Accès en lecture à l'élément (row, column). Lève std::out_of_range si hors limites.
    const T& operator()(std::size_t row, std::size_t column) const;

    Matrix4x4 operator+(const Matrix4x4& rhs) const;
    Matrix4x4 operator-(const Matrix4x4& rhs) const;
    /// @brief Multiplication matricielle standard.
    Matrix4x4 operator*(const Matrix4x4& rhs) const;
    /// @brief Transformation d'un vecteur colonne homogène.
    Vec4<T> operator*(const Vec4<T>& rhs) const;
    Matrix4x4 operator*(T scalar) const;
    Matrix4x4& operator*=(const Matrix4x4& rhs);
    bool operator==(const Matrix4x4& rhs) const;
    bool operator!=(const Matrix4x4& rhs) const;

    /// @brief Retourne la transposée.
    Matrix4x4 Transpose() const;
    /// @brief Calcule le déterminant par élimination gaussienne avec pivot partiel.
    T Determinant() const;
    /// @brief Inverse par Gauss-Jordan avec pivot partiel pondéré.
    /// Lève std::domain_error si la matrice est singulière ou mal conditionnée.
    /// @param relativeTolerance Seuil relatif de singularité (défaut : 64 * epsilon).
    Matrix4x4 Inverse(T relativeTolerance = T{64} * std::numeric_limits<T>::epsilon()) const;

    /// @brief Matrice de mise à l'échelle non uniforme.
    static Matrix4x4 Scale(const Vec3<T>& scale);
    /// @brief Rotation autour de l'axe X.
    /// @param radians Angle en radians.
    static Matrix4x4 RotationX(T radians);
    /// @brief Rotation autour de l'axe Y.
    /// @param radians Angle en radians.
    static Matrix4x4 RotationY(T radians);
    /// @brief Rotation autour de l'axe Z.
    /// @param radians Angle en radians.
    static Matrix4x4 RotationZ(T radians);
    /// @brief Matrice de translation.
    static Matrix4x4 Translation(const Vec3<T>& offset);

    /// @brief Transforme un point (w = 1) par la matrice affine.
    /// Lève std::domain_error si la matrice n'est pas affine.
    Vec3<T> TransformPoint(const Vec3<T>& point) const;
    /// @brief Transforme une direction (w = 0) par la matrice affine, sans translation.
    /// Lève std::domain_error si la matrice n'est pas affine.
    Vec3<T> TransformDirection(const Vec3<T>& direction) const;
};

// ============================================================================
// CONSTRUCTEURS
// ============================================================================

template <typename T>
Matrix4x4<T>::Matrix4x4() : values{}
{
    // 1. Initialise la diagonale à 1 (identité)
    for (std::size_t i = 0; i < 4; ++i)
    {
        values[i][i] = T{1};
    }
}

template <typename T>
Matrix4x4<T>::Matrix4x4(const std::array<T, 16>& elements) : values{}
{
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            values[row][column] = elements[row * 4 + column];
        }
    }
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::Identity()
{
    return Matrix4x4();
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::Zero()
{
    return Matrix4x4(std::array<T, 16>{});
}

// ============================================================================
// OPÉRATEURS D'ACCÈS
// ============================================================================

template <typename T>
T& Matrix4x4<T>::operator()(std::size_t row, std::size_t column)
{
    if (row >= 4 || column >= 4)
        throw std::out_of_range("Matrix index is out of range");
    return values[row][column];
}

template <typename T>
const T& Matrix4x4<T>::operator()(std::size_t row, std::size_t column) const
{
    if (row >= 4 || column >= 4)
        throw std::out_of_range("Matrix index is out of range");
    return values[row][column];
}

// ============================================================================
// OPÉRATEURS ARITHMÉTIQUES
// ============================================================================

template <typename T>
Matrix4x4<T> Matrix4x4<T>::operator+(const Matrix4x4& rhs) const
{
    Matrix4x4 result = Zero();
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            result.values[row][column] = values[row][column] + rhs.values[row][column];
        }
    }
    return result;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::operator-(const Matrix4x4& rhs) const
{
    Matrix4x4 result = Zero();
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            result.values[row][column] = values[row][column] - rhs.values[row][column];
        }
    }
    return result;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::operator*(const Matrix4x4& rhs) const
{
    Matrix4x4 result = Zero();
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            for (std::size_t k = 0; k < 4; ++k)
            {
                result.values[row][column] += values[row][k] * rhs.values[k][column];
            }
        }
    }
    return result;
}

template <typename T>
Vec4<T> Matrix4x4<T>::operator*(const Vec4<T>& rhs) const
{
    return {
        values[0][0] * rhs.x + values[0][1] * rhs.y + values[0][2] * rhs.z + values[0][3] * rhs.w,
        values[1][0] * rhs.x + values[1][1] * rhs.y + values[1][2] * rhs.z + values[1][3] * rhs.w,
        values[2][0] * rhs.x + values[2][1] * rhs.y + values[2][2] * rhs.z + values[2][3] * rhs.w,
        values[3][0] * rhs.x + values[3][1] * rhs.y + values[3][2] * rhs.z + values[3][3] * rhs.w
    };
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::operator*(T scalar) const
{
    Matrix4x4 result = Zero();
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            result.values[row][column] = values[row][column] * scalar;
        }
    }
    return result;
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::operator*=(const Matrix4x4& rhs)
{
    *this = *this * rhs;
    return *this;
}

template <typename T>
bool Matrix4x4<T>::operator==(const Matrix4x4& rhs) const
{
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            if (values[row][column] != rhs.values[row][column])
            {
                return false;
            }
        }
    }
    return true;
}

template <typename T>
bool Matrix4x4<T>::operator!=(const Matrix4x4& rhs) const
{
    return !(*this == rhs);
}

// ============================================================================
// OPÉRATIONS MATRICIELLES
// ============================================================================

template <typename T>
Matrix4x4<T> Matrix4x4<T>::Transpose() const
{
    Matrix4x4 result = Zero();
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            result.values[column][row] = values[row][column];
        }
    }
    return result;
}

template <typename T>
T Matrix4x4<T>::Determinant() const
{
    // 1. Copie de travail pour l'élimination gaussienne
    Matrix4x4 working = *this;
    T determinant = T{1};
    for (std::size_t column = 0; column < 4; ++column)
    {
        // 2. Recherche du pivot maximal en valeur absolue
        std::size_t pivot = column;
        for (std::size_t row = column + 1; row < 4; ++row)
        {
            if (std::abs(working.values[row][column]) > std::abs(working.values[pivot][column]))
            {
                pivot = row;
            }
        }
        if (working.values[pivot][column] == T{0})
        {
            return T{0};
        }
        // 3. Échange de lignes — inverse le signe du déterminant
        if (pivot != column)
        {
            for (std::size_t j = 0; j < 4; ++j)
            {
                std::swap(working.values[pivot][j], working.values[column][j]);
            }
            determinant = -determinant;
        }
        // 4. Accumulation du produit diagonal et élimination
        const T diagonal = working.values[column][column];
        determinant *= diagonal;
        for (std::size_t row = column + 1; row < 4; ++row)
        {
            const T factor = working.values[row][column] / diagonal;
            for (std::size_t j = column + 1; j < 4; ++j)
            {
                working.values[row][j] -= factor * working.values[column][j];
            }
        }
    }
    return determinant;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::Inverse(T relativeTolerance) const
{
    if (!std::isfinite(relativeTolerance) || relativeTolerance < T{ 0 } || relativeTolerance >= T{ 1 })
        throw std::invalid_argument("Inverse tolerance must be finite and in [0, 1)");

    // 1. Augmentation [A | I] pour Gauss-Jordan
    Matrix4x4 left = *this;
    Matrix4x4 right;
    T scales[4]{};
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            if (!std::isfinite(left.values[row][column]))
                throw std::domain_error("Cannot invert a non-finite matrix");
            scales[row] = std::max(scales[row], std::abs(left.values[row][column]));
        }
        if (scales[row] == T{ 0 })
            throw std::domain_error("Cannot invert a singular matrix");
    }
    for (std::size_t column = 0; column < 4; ++column)
    {
        // 2. Pivot partiel pondéré par la magnitude de chaque ligne
        std::size_t pivot = column;
        for (std::size_t row = column + 1; row < 4; ++row)
        {
            if (std::abs(left.values[row][column]) / scales[row] >
                std::abs(left.values[pivot][column]) / scales[pivot])
            {
                pivot = row;
            }
        }
        if (std::abs(left.values[pivot][column]) / scales[pivot] <= relativeTolerance)
            throw std::domain_error("Matrix is singular or too ill-conditioned");

        // 3. Échange simultané dans les deux moitiés de l'augmentation
        for (std::size_t j = 0; j < 4; ++j)
        {
            std::swap(left.values[column][j], left.values[pivot][j]);
            std::swap(right.values[column][j], right.values[pivot][j]);
        }
        std::swap(scales[column], scales[pivot]);
        // 4. Normalisation de la ligne pivot
        const T diagonal = left.values[column][column];
        for (std::size_t j = 0; j < 4; ++j)
        {
            left.values[column][j] /= diagonal;
            right.values[column][j] /= diagonal;
        }
        // 5. Élimination de la colonne courante dans toutes les autres lignes
        for (std::size_t row = 0; row < 4; ++row)
        {
            if (row == column) continue;
            const T factor = left.values[row][column];
            for (std::size_t j = 0; j < 4; ++j)
            {
                left.values[row][j] -= factor * left.values[column][j];
                right.values[row][j] -= factor * right.values[column][j];
            }
        }
    }
    return right;
}

// ============================================================================
// TRANSFORMATIONS GÉOMÉTRIQUES
// ============================================================================

template <typename T>
Matrix4x4<T> Matrix4x4<T>::Scale(const Vec3<T>& scale)
{
    Matrix4x4 result;
    result.values[0][0] = scale.x;
    result.values[1][1] = scale.y;
    result.values[2][2] = scale.z;
    return result;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::RotationX(T radians)
{
    Matrix4x4 result;
    const T cosine = std::cos(radians);
    const T sine = std::sin(radians);
    result.values[1][1] = cosine;
    result.values[2][2] = cosine;
    result.values[1][2] = -sine;
    result.values[2][1] = sine;
    return result;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::RotationY(T radians)
{
    Matrix4x4 result;
    const T cosine = std::cos(radians);
    const T sine = std::sin(radians);
    result.values[2][2] = cosine;
    result.values[0][0] = cosine;
    result.values[2][0] = -sine;
    result.values[0][2] = sine;
    return result;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::RotationZ(T radians)
{
    Matrix4x4 result;
    const T cosine = std::cos(radians);
    const T sine = std::sin(radians);
    result.values[0][0] = cosine;
    result.values[1][1] = cosine;
    result.values[0][1] = -sine;
    result.values[1][0] = sine;
    return result;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::Translation(const Vec3<T>& offset)
{
    Matrix4x4 result;
    result.values[0][3] = offset.x;
    result.values[1][3] = offset.y;
    result.values[2][3] = offset.z;
    return result;
}

template <typename T>
Vec3<T> Matrix4x4<T>::TransformPoint(const Vec3<T>& point) const
{
    if (values[3][0] != T{ 0 } || values[3][1] != T{ 0 } || values[3][2] != T{ 0 } || values[3][3] != T{ 1 })
        throw std::domain_error("Transform requires an affine matrix");
    const Vec4<T> result = *this * Vec4<T>(point, T{ 1 });
    return Vec3<T>(result);
}

template <typename T>
Vec3<T> Matrix4x4<T>::TransformDirection(const Vec3<T>& direction) const
{
    if (values[3][0] != T{ 0 } || values[3][1] != T{ 0 } || values[3][2] != T{ 0 } || values[3][3] != T{ 1 })
        throw std::domain_error("Transform requires an affine matrix");
    const Vec4<T> result = *this * Vec4<T>(direction, T{ 0 });
    return Vec3<T>(result);
}

// ============================================================================
// FONCTIONS DE CAMÉRA / PROJECTION
// ============================================================================

/// @brief Construit la matrice modèle TRS (Translation × Rotation YXZ × Scale).
/// @param position Translation dans l'espace monde.
/// @param rotation Angles d'Euler (x, y, z) en radians.
/// @param scale Facteurs d'échelle.
template <std::floating_point T>
Matrix4x4<T> Model(Vec3<T> position, Vec3<T> rotation, Vec3<T> scale)
{
    return Matrix4x4<T>::Translation(position)
         * Matrix4x4<T>::RotationY(rotation.y)
         * Matrix4x4<T>::RotationX(rotation.x)
         * Matrix4x4<T>::RotationZ(rotation.z)
         * Matrix4x4<T>::Scale(scale);
}

/// @brief Projection perspective symétrique (profondeur NDC [0, 1]).
/// @param fovY Champ de vision vertical en radians.
/// @param aspect Ratio largeur/hauteur.
/// @param nearPlane Plan proche (positif).
/// @param farPlane Plan lointain (positif, > nearPlane).
template <std::floating_point T>
Matrix4x4<T> Perspective(T fovY, T aspect, T nearPlane, T farPlane)
{
    T ys = 1.0f / std::tan(fovY * 0.5f);
    T xs = ys / aspect;

    T alpha = farPlane / (nearPlane - farPlane);
    T beta  = (farPlane * nearPlane) / (nearPlane - farPlane);

    Matrix4x4<T> result = Matrix4x4<T>::Zero();
    result.values[0][0] = xs;
    result.values[1][1] = ys;
    result.values[2][2] = alpha;
    result.values[2][3] = beta;
    result.values[3][2] = -1.0f;
    return result;
}

/// @brief Construit une matrice vue orientée vers une cible (view matrix).
/// @param eye Position de la caméra.
/// @param target Point regardé.
/// @param up Vecteur haut de référence.
template <std::floating_point T>
Matrix4x4<T> LookAt(Vec3<T> eye, Vec3<T> target, Vec3<T> up)
{
    // 1. Base orthonormée de la caméra
    Vec3<T> forward = Normalize(eye - target);
    Vec3<T> right   = Normalize(Cross(up, forward));
    Vec3<T> camUp = Cross(forward, right);
    // 2. Remplissage de la matrice vue (rotation + translation)
    Matrix4x4<T> result = Matrix4x4<T>::Zero();
    result.values[0][0] = right.x;
    result.values[0][1] = right.y;
    result.values[0][2] = right.z;
    result.values[0][3] = -Dot(right, eye);
    result.values[1][0] = camUp.x;
    result.values[1][1] = camUp.y;
    result.values[1][2] = camUp.z;
    result.values[1][3] = -Dot(camUp, eye);
    result.values[2][0] = forward.x;
    result.values[2][1] = forward.y;
    result.values[2][2] = forward.z;
    result.values[2][3] = -Dot(forward, eye);
    result.values[3][3] = T{1};
    return result;
}


}
