#pragma once

#include "darkstar/Ml/ml.h"
#include "tribes/gameBase.h"
#include "util/memory.h"
#include "util/struct.h"

class ShapeBase : public GameBase {
protected:
	enum MaskBits {
		RotationMask    = BIT(4),
		OrientationMask = BIT(13),
	};

	FIELD(0x258, Point3F, rotation);
public:
	FIELD(0x740, float, energy);

	Point3F getRot() const
	{
		return rotation;
	}

	void setRot(const Point3F &rot)
	{
		call_virtual<104, decltype(&ShapeBase::setRot)>(this, rot);
	}
};
