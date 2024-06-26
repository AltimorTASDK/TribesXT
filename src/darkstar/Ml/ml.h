#pragma once

#include "darkstar/Ml/processor.h"
#include "util/matrix.h"
#include "util/vector.h"

struct RMat3F;

struct Point2F : vec2 {
	using vec2::vec2;
	constexpr Point2F(const vec2 &vec) : vec2(vec) {}
};

struct Point3F : vec3 {
	using vec3::vec3;
	constexpr Point3F(const vec3 &vec) : vec3(vec) {}
};

struct Point4F : vec4 {
	using vec4::vec4;
	constexpr Point4F(const vec4 &vec) : vec4(vec) {}
};

struct EulerF : vec3 {
	using vec3::vec3;
	constexpr EulerF(const vec3 &vec) : vec3(vec) {}

	explicit constexpr EulerF(const Point3F &p) : EulerF(p.x, p.y, p.z)
	{
	}

	constexpr RMat3F *makeMatrix(RMat3F *mat) const
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
	uint32_t flags = 0;
};

} // namespace detail

struct RMat3F : detail::RMat3FBase, matrix3x3 {
	using matrix3x3::matrix3x3;
	constexpr RMat3F(const matrix3x3 &matrix) : matrix3x3(matrix) {}

	constexpr RMat3F(const EulerF &e)
	{
		set(e);
	}

	constexpr RMat3F &set(const EulerF &e)
	{
		return *e.makeMatrix(this);
	}
};

struct TMat3F : RMat3F {
	Point3F p;

	constexpr TMat3F() = default;

	constexpr TMat3F(const RMat3F &r, const Point3F &t)
	{
		set(r, t);
	}

	constexpr TMat3F(const EulerF &e, const Point3F &t)
	{
		set(e, t);
	}

	constexpr TMat3F &set(const RMat3F &r, const Point3F &t)
	{
		p = t;
		RMat3F::RMat3F(r);
		flags |= Matrix_HasTranslation;
		return *this;
	}

	constexpr TMat3F &set(const EulerF &e, const Point3F &t)
	{
		p = t;
		RMat3F::set(e);
		flags |= Matrix_HasTranslation;
		return *this;
	}
};

struct RectF {
	Point2F upperL;
	Point2F lowerR;
};

constexpr Point3F operator*(const Point3F &p, const RMat3F &m)
{
	Point3F result;
	Processor::get()->m_Point3F_RMat3F_mul(&p, &m, &result);
	return result;
}

constexpr Point3F operator*(const Point3F &p, const TMat3F &m)
{
	Point3F result;
	Processor::get()->m_Point3F_TMat3F_mul(&p, &m, &result);
	return result;
}

constexpr TMat3F operator*(const TMat3F &a, const TMat3F &b)
{
	TMat3F result;
	Processor::get()->m_TMat3F_TMat3F_mul(&a, &b, &result);
	return result;
}
