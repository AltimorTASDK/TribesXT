#pragma once

#include "util/memory.h"

struct Point3F;
struct RMat3F;
struct TMat3F;

class Processor {
public:
	static Processor *get()
	{
		return *(Processor**)0x6D5BF8;
	}

private:
	virtual void vfunc0() const;
	virtual void vfunc1() const;
	virtual void vfunc2() const;
	virtual void vfunc3() const;
	virtual void vfunc4() const;
	virtual void vfunc5() const;
	virtual void vfunc6() const;
	virtual void vfunc7() const;
public:
	virtual float m_Point3F_lenf(const Point3F*) const;
	virtual void m_Point3F_normalizef(Point3F*) const;
	virtual void m_Point3F_RMat3F_mul(const Point3F*, const RMat3F*, Point3F*) const;
	virtual void m_Point3F_TMat3F_mul(const Point3F*, const TMat3F*, Point3F*) const;
	virtual void m_TMat3F_TMat3F_mul(const TMat3F*, const TMat3F*, TMat3F*) const;
private:
	virtual void vfunc13() const;
	virtual void vfunc14() const;
	virtual void vfunc15() const;
	virtual void vfunc16() const;
};
