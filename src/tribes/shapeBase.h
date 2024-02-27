#pragma once

#include "darkstar/Ml/ml.h"
#include "tribes/gameBase.h"
#include "util/memory.h"
#include "util/struct.h"
#include <cstddef>

class ShapeBase : public GameBase {
protected:
	enum MaskBits {
		RotationMask    = BIT(4),
		OrientationMask = BIT(13),
	};

public:
	struct ShapeBaseData : GameBase::GameBaseData {
		std::byte padding34[0x68 - 0x34];
	};

protected:
	FIELD(0x258, Point3F, rotation);
public:
	FIELD(0x740, float, energy);

	Point3F getRot() const
	{
		return rotation;
	}

	void setPos(const Point3F &pos)
	{
		call_virtual<103, decltype(&ShapeBase::setPos)>(this, pos);
	}

	void setRot(const Point3F &rot)
	{
		call_virtual<104, decltype(&ShapeBase::setRot)>(this, rot);
	}

	bool getAimedMuzzleTransform(int slot, TMat3F *mat) const
	{
		using func_t = to_static_function_t<bool(ShapeBase::*)(int, TMat3F*) const>;
		return ((func_t)0x4D1DA0)(this, slot, mat);
	}

	TMat3F getAimedMuzzleTransform(int slot) const
	{
		TMat3F mat;
		getAimedMuzzleTransform(slot, &mat);
		return mat;
	}
};
