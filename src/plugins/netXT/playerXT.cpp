#include "darkstar/console/console.h"
#include "tribes/playerPSC.h"
#include "tribes/worldGlobals.h"
#include "plugins/netXT/netXT.h"
#include "plugins/netXT/playerXT.h"
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

auto PlayerXT::getSnapshot(uint32_t time) const -> const Snapshot*
{
	const auto &snap = xt.snapshots[msToTicks(time) % SnapHistory];
	return snap.time == time ? &snap : nullptr;
}

auto PlayerXT::getSnapshotNext(uint32_t time) const -> const Snapshot*
{
	const auto startTick = msToTicks(time);

	for (auto tick = startTick; tick < startTick + SnapHistory; tick++) {
		const auto &snap = xt.snapshots[tick % SnapHistory];
		if (msToTicks(snap.time) == tick && snap.time >= time)
			return &snap;
	}

	return nullptr;
}

auto PlayerXT::getSnapshotPrev(uint32_t time) const -> const Snapshot*
{
	const auto startTick = msToTicks(time);

	for (auto tick = startTick; tick > startTick - SnapHistory; tick--) {
		const auto &snap = xt.snapshots[tick % SnapHistory];
		if (msToTicks(snap.time) == tick && snap.time <= time)
			return &snap;
	}

	return nullptr;
}

// Corresponds to Player::readPacketData
void PlayerXT::loadSnapshot(const Snapshot &snapshot, bool useMouse)
{
	auto pitch = snapshot.pitch;
	auto yaw = snapshot.yaw;

	if (useMouse && hasFocus && cg.psc != nullptr) {
		// Use accumulated mouse input for local player
		if (const auto *newSnap = getSnapshot(lastProcessTime); newSnap != nullptr) {
			const auto &curMove = cg.psc->curMove;
			pitch = clamp(newSnap->pitch + curMove.pitch, -MaxPitch, MaxPitch);
			yaw = normalize_radians(newSnap->yaw + curMove.turnRot);
		}
	}

	setLinearVelocity(snapshot.velocity);
	setSensorPinged(snapshot.pingStatus);
	setRot({getRot().x, getRot().y, yaw});
	setPos(snapshot.position);

	if (mount != nullptr) {
		const auto mountTransform = mount->getObjectMountTransform(mountPoint);
		setTransform(TMat3F(EulerF(getRot()), {0, 0, 0}) * mountTransform);
	}

	viewPitch = pitch;
	energy = snapshot.energy;
	contact = snapshot.contact;
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
	const auto *a = getSnapshotPrev(time);
	if (a == nullptr)
		return false;

	if (a->time == time) {
		loadSnapshot(*a, true);
		return true;
	}

	const auto *b = getSnapshotNext(time);
	if (b == nullptr)
		return false;

	const auto fraction = (float)(time - a->time) / (b->time - a->time);
	loadSnapshot(Snapshot::interpolate(*a, *b, fraction), true);
	return true;
}

void PlayerXT::setViewAngles(float pitch, float yaw)
{
	viewPitch = pitch;
	setRot({getRot().x, getRot().y, yaw});
	setTransform(TMat3F(EulerF(getRot()), getTransform().p));
}

void PlayerXT::setViewAnglesClamped(float pitch, float yaw)
{
	setViewAngles(clamp(pitch, -MaxPitch, MaxPitch), normalize_radians(yaw));
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

	for (auto index = 0; index < moveCount; index++) {
		if (updateDebt > 5)
			break;

		updateDebt++;

		const auto &move = moves[index];
		const auto &subtickRecord = xt.subtickRecords[index];

		float subtickPitch;
		float subtickYaw;

		// Clients shouldn't send more than SubtickHistory moves, but are allowed to
		if (index < SubtickHistory && subtickRecord.subtick != NoSubtick) {
			// Lets updateMove know not to update image states yet
			xt.applySubtick = true;
			subtickPitch = viewPitch + subtickRecord.pitch;
			subtickYaw = getRot().z + subtickRecord.yaw;
		}

		if (move.useItem != -1) {
			char buf[16];
			sprintf_s(buf, "%d", move.useItem);
			Console->executef(3, "remoteUseItem", scriptThis(), buf);
		}

		updateDamage(0.032f);
		updateMove(&move, true);
		updateAnimation(0.032f);

		if (xt.applySubtick) {
			// Subtract an extra tick to match the client's interpolated view
			const auto tickStart = lastProcessTime - TickMs * 2;
			const auto subtickTime = tickStart + subtickRecord.subtick;
			loadSnapshotInterpolated(subtickTime);
			setViewAnglesClamped(subtickPitch, subtickYaw);

			// Call updateImageState here instead
			for (auto i = 0; i < MaxItemImages; i++)
				updateImageState(i, 0.032f);
		}

		// Update trigger state after move
		if (lastPlayerMove.trigger && !move.trigger)
			setImageTriggerUp(0);
		else if (!lastPlayerMove.trigger && move.trigger)
			setImageTriggerDown(0);

		if (xt.applySubtick) {
			// Restore
			loadSnapshot(lastProcessTime);
			xt.applySubtick = false;
		}

		lastPlayerMove = move;
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

	setLinearVelocity(newVel);
	setRot(rot);
	setPos(newPos);
	contact = newContact;
	viewPitch = newPitch;
	updateSkip = skipCount;

	if (mount != nullptr) {
		const auto mountTransform = mount->getObjectMountTransform(mountPoint);
		setTransform(TMat3F(EulerF(rot), {0, 0, 0}) * mountTransform);
	}

	// Use average time of frame since we don't know when the packet arrived
	const auto avgTime = (cg.currentTime + cg.lastTime + 1) / 2;
	lastProcessTime = avgTime + timeNudge - TickMs;
	invalidatePrediction(lastProcessTime);
	saveSnapshot(lastProcessTime);
	// State sent by server is from before the move, so simulate once
	updateMove(move, false);
	lastPlayerMove = *move;
}