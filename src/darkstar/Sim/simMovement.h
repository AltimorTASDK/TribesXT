#pragma once

#include "darkstar/Core/bitset.h"
#include "darkstar/Ml/ml.h"
#include "darkstar/Sim/simContainer.h"
#include "util/struct.h"
#include <cstdint>

class SimMovement : public SimContainer {
protected:
	enum Flags {
		UseFriction         = BIT(0),
		UseAngulerVelocity  = BIT(1),
		UseElasticity       = BIT(2),
		UseCurrent          = BIT(3),
		UseDrag             = BIT(4),
		TrackContacts       = BIT(5),
		RotateBoundingBox   = BIT(6),
		AtRest              = BIT(7),
		AlwaysCalcAVelocity = BIT(8),
	};
private:
	FIELD(0xA0, TMat3F, transform);
	FIELD(0xD4, TMat3F, invTransform);
	FIELD(0x178, BitSet32, flags);
	FIELD(0x19C, Point3F, lPosition);
	FIELD(0x1A8, Point3F, lVelocity);
protected:
	FIELD(0x208, bool, contact);
	FIELD(0x20C, uint32_t, excludedId);

public:
	const Point3F &getLinearVelocity() const { return lVelocity; }
	const Point3F &getLinearPosition() const { return lPosition; }

	const TMat3F &getTransform()    const { return transform; }
	const TMat3F &getInvTransform() const { return invTransform; }
	const RMat3F &getRotation()     const { return (RMat3F&)transform; }
	const RMat3F &getInvRotation()  const { return (RMat3F&)invTransform; }

	void setLinearVelocity(const Point3F &vel)
	{
		lVelocity = vel;
		flags.clear(AtRest);
	}

	void setTransform(const TMat3F &mat)
	{
		using func_t = to_static_function_t<decltype(&SimMovement::setTransform)>;
		((func_t)0x52DB40)(this, mat);
	}
};
