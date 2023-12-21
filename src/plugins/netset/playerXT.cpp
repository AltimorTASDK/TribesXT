#include "darkstar/console/console.h"
#include "tribes/playerPSC.h"
#include "tribes/worldGlobals.h"
#include "plugins/netset/netset.h"
#include "plugins/netset/playerXT.h"
#include "util/math.h"
#include <cstdint>
#include <new>

auto PlayerXT::Snapshot::interpolate(const Snapshot &a, const Snapshot &b, float t) -> Snapshot
{
	return {
		.tick = b.tick,
		.yaw = lerp_radians(a.yaw, b.yaw, t),
		.position = lerp(a.position, b.position, t),
		.velocity = lerp(a.velocity, b.velocity, t),
		.pitch = lerp(a.pitch, b.pitch, t),
		.energy = lerp(a.energy, b.energy, t),
		.traction = b.traction,
		.jumpSurfaceLastContact = b.jumpSurfaceLastContact,
		.pingStatus = b.pingStatus,
		.contact = b.contact,
		.jetting = b.jetting,
		.crouching = b.crouching
	};
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

// Corresponds to Player::readPacketData
void PlayerXT::loadSnapshot(const Snapshot &snapshot, bool useMouseInput)
{
	auto pitch = snapshot.pitch;
	auto yaw = snapshot.yaw;

	if (hasFocus && useMouseInput) {
		// Use accumulated mouse input for local player
		if (const auto *newSnap = getSnapshot(lastProcessTime); newSnap != nullptr) {
			constexpr auto MaxPitch = deg_to_rad(88);
			pitch = clamp(newSnap->pitch + cg.psc->curMove.pitch, -MaxPitch, MaxPitch);
			yaw = newSnap->yaw + cg.psc->curMove.turnRot;
		}
	}

	setSensorPinged(snapshot.pingStatus);
	const auto rot = Point3F(getRot().x, getRot().y, yaw);

	if (mount == nullptr) {
		setLinearVelocity(snapshot.velocity);
		TMat3F transform;
		transform.set(EulerF(rot.x, rot.y, rot.z), snapshot.position);
		setTransform(transform);
	}

	viewPitch = pitch;
	energy = snapshot.energy;
	contact = snapshot.contact;
	setRot(rot);
	jetting = snapshot.jetting;
	traction = snapshot.traction;

	const auto oldCrouch = crouching;
	crouching = snapshot.crouching;

	if (crouching && !oldCrouch)
		setAnimation(ANIM_CROUCH);
	else if (oldCrouch && !crouching)
		setAnimation(ANIM_STAND);

	jumpSurfaceLastContact = snapshot.jumpSurfaceLastContact;
	interpDoneTime = 0;
}

bool PlayerXT::loadSnapshot(uint32_t time)
{
	const auto *snap = getSnapshot(time);
	if (snap == nullptr)
		return false;

	loadSnapshot(*snap);
	return true;
}

bool PlayerXT::loadSnapshotInterpolated(uint32_t time)
{
	const auto *a = getSnapshot(time);
	if (a == nullptr)
		return false;

	const auto *b = getSnapshot(time + TICK_MS - 1);
	if (b == nullptr)
		return false;

	if (a == b) {
		loadSnapshot(*a, true);
		return true;
	}

	const auto fraction = (float)(time % TICK_MS) / TICK_MS;
	loadSnapshot(Snapshot::interpolate(*a, *b, fraction), true);
	return true;
}

void PlayerXT::clientMove(uint32_t curTime)
{
	if (lastProcessTime < curTime) {
		loadSnapshot(lastProcessTime);
		do {
			if (hasFocus) {
				auto *move = cg.psc->getClientMove(lastProcessTime);
				if (move == nullptr)
					break;
				lastPlayerMove = *move;
				updateMove(move, false);
			} else {
				updateMove(&lastPlayerMove, false);
			}
		} while (lastProcessTime < curTime);
	}

	loadSnapshotInterpolated(curTime);
}