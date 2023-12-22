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
	auto snapshot = b;
	snapshot.yaw = lerp_radians(a.yaw, b.yaw, t);
	snapshot.position = lerp(a.position, b.position, t);
	snapshot.velocity = lerp(a.velocity, b.velocity, t);
	snapshot.pitch = lerp(a.pitch, b.pitch, t);
	snapshot.energy = lerp(a.energy, b.energy, t);
	return snapshot;
}

auto PlayerXT::createSnapshot(uint32_t time) const -> Snapshot
{
	return {
		.tick = msToTicks(time),
		.time = time,
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
void PlayerXT::loadSnapshot(const Snapshot &snapshot, bool useMouse)
{
	auto pitch = snapshot.pitch;
	auto yaw = snapshot.yaw;

	if (hasFocus && useMouse) {
		// Use accumulated mouse input for local player
		if (const auto *newSnap = getSnapshot(lastProcessTime); newSnap != nullptr) {
			const auto &curMove = cg.psc->curMove;
			pitch = clamp(newSnap->pitch + curMove.pitch, -MaxPitch, MaxPitch);
			yaw = normalize_radians(newSnap->yaw + curMove.turnRot);
		}
	}

	setSensorPinged(snapshot.pingStatus);
	const auto rot = Point3F(getRot().x, getRot().y, yaw);

	if (mount == nullptr) {
		setLinearVelocity(snapshot.velocity);
		setTransform({EulerF(rot), snapshot.position});
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

	if (time == a->time) {
		loadSnapshot(*a, true);
		return true;
	}

	const auto *b = getSnapshot(time + TickMs - 1);
	if (b == nullptr)
		return false;

	const auto fraction = (float)(time - a->time) / (b->time - a->time);
	loadSnapshot(Snapshot::interpolate(*a, *b, fraction), true);
	return true;
}

// clientProcess partial reimplementation
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

void PlayerXT::serverUpdateMove(PlayerMove *moves, int moveCount)
{
	if (dead)
		return;

	while (moveCount--) {
		if (updateDebt > 5)
			break;

		updateDebt++;

		if (moves->useItem != -1) {
			char buf[16];
			sprintf_s(buf, "%d", moves->useItem);
			Console->executef(3, "remoteUseItem", scriptThis(), buf);
		}

		updateDamage(0.032f);
		updateMove(moves, true);
		updateAnimation(0.032f);

		// Update trigger state after move
		if (lastPlayerMove.trigger && !moves->trigger)
			setImageTriggerUp(0);
		else if (!lastPlayerMove.trigger && moves->trigger)
			setImageTriggerDown(0);

		lastPlayerMove = *moves++;
	}

	if (mount == nullptr || mountPoint != 1)
		setMaskBits(OrientationMask);

	updateSkip = 0;
}

void PlayerXT::ghostSetMove(
	PlayerMove *move, const Point3F &newPos, const Point3F &newVel,
	bool newContact, float newRot, float newPitch, int skipCount, bool noInterp,
	int timeNudge)
{
	const auto rot = Point3F(getRot().x, getRot().y, newRot);

	lastPlayerMove = *move;
	setLinearVelocity(newVel);
	setPos(newPos);
	contact = newContact;
	viewPitch = newPitch;
	setRot(rot);
	updateSkip = skipCount;

	if (mount != nullptr) {
		const auto mountTransform = mount->getObjectMountTransform(mountPoint);
		setTransform(TMat3F(EulerF(rot), {0, 0, 0}) * mountTransform);
	}

	// State sent by server is from before the move, so simulate once
	lastProcessTime = roundMsUpToTick(cg.currentTime) + timeNudge - TickMs;
	invalidatePrediction(lastProcessTime);
	saveSnapshot(lastProcessTime);
	updateMove(move, false);

	Console->printf("currentTime tick %d ms %d", msToTicks(cg.currentTime), cg.currentTime % TickMs);
}