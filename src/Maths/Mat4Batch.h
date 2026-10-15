#pragma once
#include "Vec3.h"
#include "Matrix4x4.h"

namespace Maths::Ref
{
    void TransformPointBatch(const Matrix4x4<>& transform, const Vec3<>* points, Vec3<>* outPoints, std::size_t count);

	void MultiplyMatrixBatch(const Matrix4x4<>* a, const Matrix4x4<>* b, Matrix4x4<>* out, std::size_t count);
}

namespace Maths::SSE
{
    void TransformPointBatch(const Matrix4x4<>& transform, const Vec3<>* points, Vec3<>* outPoints, std::size_t count);

	void MultiplyMatrixBatch(const Matrix4x4<>* a, const Matrix4x4<>* b, Matrix4x4<>* out, std::size_t count);
}