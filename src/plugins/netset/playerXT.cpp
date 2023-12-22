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
	auto snapshot = Snapshot {
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

	for (auto i = 0; i < MaxItemImages; i++) {
		snapshot.itemImages[i].state = itemImageList[i].state;
		snapshot.itemImages[i].delayTime = itemImageList[i].delayTime;
		snapshot.itemImages[i].fireCount = itemImageList[i].fireCount;
		snapshot.itemImages[i].triggerDown = itemImageList[i].triggerDown;
		snapshot.itemImages[i].ammo = itemImageList[i].ammo;
	}

	return snapshot;
}

// Corresponds to Player::readPacketData
void PlayerXT::loadSnapshot(const Snapshot &snapshot, bool images, bool useMouse)
{
	auto pitch = snapshot.pitch;
	auto yaw = snapshot.yaw;

	if (hasFocus && useMouse) {
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

	if (!images)
		return;

	for (auto i = 0; i < MaxItemImages; i++) {
		itemImageList[i].state = snapshot.itemImages[i].state;
		itemImageList[i].delayTime = snapshot.itemImages[i].delayTime;
		itemImageList[i].fireCount = snapshot.itemImages[i].fireCount;
		itemImageList[i].triggerDown = snapshot.itemImages[i].triggerDown;
		itemImageList[i].ammo = snapshot.itemImages[i].ammo;
	}
}

bool PlayerXT::loadSnapshot(uint32_t time, bool images)
{
	const auto *snap = getSnapshot(time);
	if (snap == nullptr)
		return false;

	loadSnapshot(*snap, images);
	return true;
}

bool PlayerXT::loadSnapshotInterpolated(uint32_t time)
{
	const auto *a = getSnapshot(time);
	if (a == nullptr)
		return false;

	const auto *b = getSnapshot(time + TickMs - 1);
	if (b == nullptr)
		return false;

	if (a == b) {
		loadSnapshot(*a, false, true);
		return true;
	}

	const auto fraction = (float)(time % TickMs) / TickMs;
	loadSnapshot(Snapshot::interpolate(*a, *b, fraction), false, true);
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

		saveSnapshot(lastProcessTime);

		lastPlayerMove = *moves++;
	}

	if (mount == nullptr || mountPoint != 1)
		setMaskBits(OrientationMask);

	updateSkip = 0;
}