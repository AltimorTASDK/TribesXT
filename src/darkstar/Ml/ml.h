#pragma once

#include "util/matrix.h"
#include "util/vector.h"

using Point2F = vec2;
using Point3F = vec3;

struct RMat3F;

struct EulerF : vec3 {
	using vec3::vec3;

	RMat3F *makeMatrix(RMat3F *mat) const
	{
		using func_t = to_static_function_t<decltype(&EulerF::makeMatrix)>;
		return ((func_t)0x50BE90)(this, mat);
	}
};

namespace detail {

struct RMat3FBase {
	enum {
		Matrix_HasRotation    = 1,
		Matrix_HasTranslation = 2,
		Matrix_HasScale       = 4,
	};
	uint32_t flags;
};

} // namespace detail

struct RMat3F : detail::RMat3FBase, matrix3x3 {
	using matrix3x3::matrix3x3;

	RMat3F &set(const EulerF &e)
	{
		return *e.makeMatrix(this);
	}
};

struct TMat3F : RMat3F {
	Point3F p;

	TMat3F &set(const EulerF &e, const Point3F &t)
	{
		p = t;
		RMat3F::set(e);
		flags |= Matrix_HasTranslation;
		return *this;
	}
};
