#pragma once

#include "Vec3.h"
#include <array>
#include <cassert>
#include <limits>
#include <utility>

namespace Maths
{
// ============================================================================
// MATRICE 3×3
// ============================================================================

/// @brief Matrice 3×3 générique.
/// Stockage ligne-major, vecteurs colonnes : résultat = matrice * vecteur.
/// A * B applique B en premier. Les angles sont en radians, rotations main droite.
/// @tparam T Type des éléments (float par défaut).
template <typename T = float>
class Matrix3x3
{
public:
    T values[3][3];

    Matrix3x3(); // Identité.
    /// @brief Construit depuis un tableau de 9 éléments en ordre ligne-major.
    explicit Matrix3x3(const std::array<T, 9>& elements);
    static Matrix3x3 Identity();
    static Matrix3x3 Zero();

    /// @brief Accès en écriture à l'élément (row, column). Lève std::out_of_range si hors limites.
    T& operator()(std::size_t row, std::size_t column);
    /// @brief Accès en lecture à l'élément (row, column). Lève std::out_of_range si hors limites.
    const T& operator()(std::size_t row, std::size_t column) const;

    Matrix3x3 operator+(const Matrix3x3& rhs) const;
    Matrix3x3 operator-(const Matrix3x3& rhs) const;
    /// @brief Multiplication matricielle standard.
    Matrix3x3 operator*(const Matrix3x3& rhs) const;
    /// @brief Transformation d'un vecteur colonne.
    Vec3<T> operator*(const Vec3<T>& rhs) const;
    Matrix3x3 operator*(T scalar) const;
    Matrix3x3& operator*=(const Matrix3x3& rhs);
    bool operator==(const Matrix3x3& rhs) const;
    bool operator!=(const Matrix3x3& rhs) const;

    /// @brief Retourne la transposée.
    Matrix3x3 Transpose() const;
    /// @brief Calcule le déterminant par élimination gaussienne avec pivot partiel.
    T Determinant() const;
    /// @brief Inverse par Gauss-Jordan avec pivot partiel pondéré.
    /// Lève std::domain_error si la matrice est singulière ou mal conditionnée.
    /// @param relativeTolerance Seuil relatif de singularité (défaut : 64 * epsilon).
    Matrix3x3 Inverse(T relativeTolerance = T{64} * std::numeric_limits<T>::epsilon()) const;

    /// @brief Matrice de mise à l'échelle non uniforme.
    static Matrix3x3 Scale(const Vec3<T>& scale);
    /// @brief Rotation autour de l'axe X.
    /// @param radians Angle en radians.
    static Matrix3x3 RotationX(T radians);
    /// @brief Rotation autour de l'axe Y.
    /// @param radians Angle en radians.
    static Matrix3x3 RotationY(T radians);
    /// @brief Rotation autour de l'axe Z.
    /// @param radians Angle en radians.
    static Matrix3x3 RotationZ(T radians);
};

// ============================================================================
// CONSTRUCTEURS
// ============================================================================

template <typename T>
Matrix3x3<T>::Matrix3x3() : values{}
{
    // 1. Initialise la diagonale à 1 (identité)
    for (std::size_t i = 0; i < 3; ++i)
    {
        values[i][i] = T{1};
    }
}

template <typename T>
Matrix3x3<T>::Matrix3x3(const std::array<T, 9>& elements) : values{}
{
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            values[row][column] = elements[row * 3 + column];
        }
    }
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::Identity()
{
    return Matrix3x3();
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::Zero()
{
    return Matrix3x3(std::array<T, 9>{});
}

// ============================================================================
// OPÉRATEURS D'ACCÈS
// ============================================================================

template <typename T>
T& Matrix3x3<T>::operator()(std::size_t row, std::size_t column)
{
    if (row >= 3 || column >= 3)
        throw std::out_of_range("Matrix index is out of range");
    return values[row][column];
}

template <typename T>
const T& Matrix3x3<T>::operator()(std::size_t row, std::size_t column) const
{
    if (row >= 3 || column >= 3)
        throw std::out_of_range("Matrix index is out of range");
    return values[row][column];
}

// ============================================================================
// OPÉRATEURS ARITHMÉTIQUES
// ============================================================================

template <typename T>
Matrix3x3<T> Matrix3x3<T>::operator+(const Matrix3x3& rhs) const
{
    Matrix3x3 result = Zero();
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            result.values[row][column] = values[row][column] + rhs.values[row][column];
        }
    }
    return result;
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::operator-(const Matrix3x3& rhs) const
{
    Matrix3x3 result = Zero();
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            result.values[row][column] = values[row][column] - rhs.values[row][column];
        }
    }
    return result;
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::operator*(const Matrix3x3& rhs) const
{
    Matrix3x3 result = Zero();
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            for (std::size_t k = 0; k < 3; ++k)
            {
                result.values[row][column] += values[row][k] * rhs.values[k][column];
            }
        }
    }
    return result;
}

template <typename T>
Vec3<T> Matrix3x3<T>::operator*(const Vec3<T>& rhs) const
{
    return {
        values[0][0] * rhs.x + values[0][1] * rhs.y + values[0][2] * rhs.z,
        values[1][0] * rhs.x + values[1][1] * rhs.y + values[1][2] * rhs.z,
        values[2][0] * rhs.x + values[2][1] * rhs.y + values[2][2] * rhs.z
    };
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::operator*(T scalar) const
{
    Matrix3x3 result = Zero();
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            result.values[row][column] = values[row][column] * scalar;
        }
    }
    return result;
}

template <typename T>
Matrix3x3<T>& Matrix3x3<T>::operator*=(const Matrix3x3& rhs)
{
    *this = *this * rhs;
    return *this;
}

template <typename T>
bool Matrix3x3<T>::operator==(const Matrix3x3& rhs) const
{
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
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
bool Matrix3x3<T>::operator!=(const Matrix3x3& rhs) const
{
    return !(*this == rhs);
}

// ============================================================================
// OPÉRATIONS MATRICIELLES
// ============================================================================

template <typename T>
Matrix3x3<T> Matrix3x3<T>::Transpose() const
{
    Matrix3x3 result = Zero();
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            result.values[column][row] = values[row][column];
        }
    }
    return result;
}

template <typename T>
T Matrix3x3<T>::Determinant() const
{
    // 1. Copie de travail pour l'élimination gaussienne
    Matrix3x3 working = *this;
    T determinant = T{1};
    for (std::size_t column = 0; column < 3; ++column)
    {
        // 2. Recherche du pivot maximal en valeur absolue
        std::size_t pivot = column;
        for (std::size_t row = column + 1; row < 3; ++row)
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
            for (std::size_t j = 0; j < 3; ++j)
            {
                std::swap(working.values[pivot][j], working.values[column][j]);
            }
            determinant = -determinant;
        }
        // 4. Accumulation du produit diagonal et élimination
        const T diagonal = working.values[column][column];
        determinant *= diagonal;
        for (std::size_t row = column + 1; row < 3; ++row)
        {
            const T factor = working.values[row][column] / diagonal;
            for (std::size_t j = column + 1; j < 3; ++j)
            {
                working.values[row][j] -= factor * working.values[column][j];
            }
        }
    }
    return determinant;
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::Inverse(T relativeTolerance) const
{
    if (!std::isfinite(relativeTolerance) || relativeTolerance < T{ 0 } || relativeTolerance >= T{ 1 })
        throw std::invalid_argument("Inverse tolerance must be finite and in [0, 1)");

    // 1. Augmentation [A | I] pour Gauss-Jordan
    Matrix3x3 left = *this;
    Matrix3x3 right;
    T scales[3]{};
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            if (!std::isfinite(left.values[row][column]))
                throw std::domain_error("Cannot invert a non-finite matrix");
            scales[row] = std::max(scales[row], std::abs(left.values[row][column]));
        }
        if (scales[row] == T{ 0 })
            throw std::domain_error("Cannot invert a singular matrix");
    }
    for (std::size_t column = 0; column < 3; ++column)
    {
        // 2. Pivot partiel pondéré par la magnitude de chaque ligne
        std::size_t pivot = column;
        for (std::size_t row = column + 1; row < 3; ++row)
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
        for (std::size_t j = 0; j < 3; ++j)
        {
            std::swap(left.values[column][j], left.values[pivot][j]);
            std::swap(right.values[column][j], right.values[pivot][j]);
        }
        std::swap(scales[column], scales[pivot]);
        // 4. Normalisation de la ligne pivot
        const T diagonal = left.values[column][column];
        for (std::size_t j = 0; j < 3; ++j)
        {
            left.values[column][j] /= diagonal;
            right.values[column][j] /= diagonal;
        }
        // 5. Élimination de la colonne courante dans toutes les autres lignes
        for (std::size_t row = 0; row < 3; ++row)
        {
            if (row == column) continue;
            const T factor = left.values[row][column];
            for (std::size_t j = 0; j < 3; ++j)
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
Matrix3x3<T> Matrix3x3<T>::Scale(const Vec3<T>& scale)
{
    Matrix3x3 result;
    result.values[0][0] = scale.x;
    result.values[1][1] = scale.y;
    result.values[2][2] = scale.z;
    return result;
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::RotationX(T radians)
{
    Matrix3x3 result;
    const T cosine = std::cos(radians);
    const T sine = std::sin(radians);
    result.values[1][1] = cosine;
    result.values[2][2] = cosine;
    result.values[1][2] = -sine;
    result.values[2][1] = sine;
    return result;
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::RotationY(T radians)
{
    Matrix3x3 result;
    const T cosine = std::cos(radians);
    const T sine = std::sin(radians);
    result.values[2][2] = cosine;
    result.values[0][0] = cosine;
    result.values[2][0] = -sine;
    result.values[0][2] = sine;
    return result;
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::RotationZ(T radians)
{
    Matrix3x3 result;
    const T cosine = std::cos(radians);
    const T sine = std::sin(radians);
    result.values[0][0] = cosine;
    result.values[1][1] = cosine;
    result.values[0][1] = -sine;
    result.values[1][0] = sine;
    return result;
}

}
