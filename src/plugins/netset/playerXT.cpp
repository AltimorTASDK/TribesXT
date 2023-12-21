#include "plugins/netset/netset.h"
#include "plugins/netset/playerXT.h"
#include <cstdint>
#include <new>

PlayerXT *NetSetPlugin::hook_Player_ctor(PlayerXT *player)
{
	// Initialize new fields
	new (&player->xt) PlayerXT::DataXT;
	return get()->hooks.Player.ctor.callOriginal(player);
}

auto PlayerXT::createSnapshot(uint32_t time) const -> Snapshot
{
	return {
		.tick = msToTicks(time),
		.yaw = getRot().z,
		.position = getLinearPosition(),
		.velocity = getLinearVelocity(),
		.pitch = viewPitch,
		.energy = energy,
		.traction = traction,
		.jumpSurfaceLastContact = jumpSurfaceLastContact,
		.pingStatus = getSensorPinged(),
		.contact = contact,
		.jetting = jetting,
		.crouching = crouching
	};
}

bool PlayerXT::loadSnapshot(uint32_t time)
{
	const auto *snap = getSnapshot(time);

	if (snap == nullptr)
		return false;

	// Corresponds to Player::readPacketData

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