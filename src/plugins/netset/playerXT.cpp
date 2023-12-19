#include "plugins/netset/playerXT.h"
#include <cstdint>

bool PlayerXT::loadSnapshot(uint32_t time)
{
	const auto *snap = getSnapshot(time);

	if (snap == nullptr)
		return false;

	setSensorPinged(snap->pingStatus);
	const auto rot = Point3F(getRot().x, getRot().y, snap->yaw);

	if (mount == nullptr) {
		setLinearVelocity(snap->velocity);
		TMat3F transform;
		transform.set(EulerF(rot.x, rot.y, rot.z), snap->position);
		setTransform(transform);
	}

	viewPitch = snap->pitch;
	energy = snap->energy;
	contact = snap->contact;
	setRot(rot);
	jetting = snap->jetting;
	traction = snap->traction;

	const auto oldCrouch = crouching;
	crouching = snap->crouching;

	if (crouching && !oldCrouch)
		setAnimation(ANIM_CROUCH);
	else if (oldCrouch && !crouching)
		setAnimation(ANIM_STAND);

	jumpSurfaceLastContact = snap->jumpSurfaceLastContact;
	interpDoneTime = 0;

	return true;
}