#include "darkstar/console/console.h"
#include "tribes/playerPSC.h"
#include "tribes/projectile.h"
#include "tribes/worldGlobals.h"
#include "plugins/netXT/netXT.h"
#include "plugins/netXT/playerXT.h"
#include "util/math.h"
#include <algorithm>
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
	return Snapshot {
		.time = time,
		.yaw = getRot().z,
		.position = getLinearPosition(),
		.velocity = getLinearVelocity(),
		.pitch = viewPitch,
		.energy = energy,
		.traction = traction,
		.lastContactCount = lastContactCount,
		.jumpSurfaceLastContact = jumpSurfaceLastContact,
		.pingStatus = getSensorPinged(),
		.contact = contact,
		.jetting = jetting,
		.crouching = crouching
	};
}

auto PlayerXT::SnapshotBuffer::get(uint32_t time) const -> const Snapshot*
{
	const auto &snap = buffer[msToTicks(time) % SnapHistory];
	return snap.time == time ? &snap : nullptr;
}

auto PlayerXT::SnapshotBuffer::getNext(uint32_t time) const -> const Snapshot*
{
	const auto startTick = msToTicks(time);

	for (auto tick = startTick; tick < startTick + SnapHistory; tick++) {
		const auto &snap = buffer[tick % SnapHistory];
		if (msToTicks(snap.time) == tick && snap.time >= time)
			return &snap;
	}

	return nullptr;
}

auto PlayerXT::SnapshotBuffer::getPrev(uint32_t time) const -> const Snapshot*
{
	const auto startTick = msToTicks(time);

	for (auto tick = startTick; tick > startTick - SnapHistory; tick--) {
		const auto &snap = buffer[tick % SnapHistory];
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
		if (const auto *newSnap = xt.snapshots.get(lastProcessTime); newSnap != nullptr) {
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

	lastContactCount = snapshot.lastContactCount;
	jumpSurfaceLastContact = snapshot.jumpSurfaceLastContact;
}

bool PlayerXT::loadSnapshot(uint32_t time)
{
	const auto *snap = xt.snapshots.get(time);
	if (snap == nullptr)
		return false;

	loadSnapshot(*snap);
	return true;
}

bool PlayerXT::loadSnapshotInterpolated(uint32_t time)
{
	const auto *a = xt.snapshots.getPrev(time);
	if (a == nullptr)
		return false;

	if (a->time == time) {
		loadSnapshot(*a, true);
		return true;
	}

	const auto *b = xt.snapshots.getNext(time);
	if (b == nullptr)
		return false;

	const auto fraction = (float)(time - a->time) / (b->time - a->time);
	loadSnapshot(Snapshot::interpolate(*a, *b, fraction), true);
	return true;
}

void PlayerXT::startLagCompensation(uint32_t time)
{
	const auto *a = xt.lagCompensationSnapshots.getPrev(time);
	const auto *b = xt.lagCompensationSnapshots.getNext(time);

	if (a == nullptr && b == nullptr)
		return;

	xt.lagCompensationBackup = createSnapshot();

	if (a != nullptr && (a->time == time || b == nullptr)) {
		loadSnapshot(*a);
		xt.lagCompensationTarget = *a;
		return;
	}
	
	if (b != nullptr && a == nullptr) {
		loadSnapshot(*b);
		xt.lagCompensationTarget = *b;
		return;
	}

	const auto fraction = (float)(time - a->time) / (b->time - a->time);
	const auto snapshot = Snapshot::interpolate(*a, *b, fraction);
	loadSnapshot(snapshot);
	xt.lagCompensationTarget = snapshot;
}

void PlayerXT::endLagCompensation()
{
	if (!xt.lagCompensationTarget.isValid())
		return;

	// Preserve any velocity change from explosions
	const auto impulse = getLinearVelocity() - xt.lagCompensationTarget.velocity;
	loadSnapshot(xt.lagCompensationBackup);
	setLinearVelocity(getLinearVelocity() + impulse);

	xt.lagCompensationTarget.invalidate();
}

void PlayerXT::startLagCompensationAll(const SimObject *exclude, uint32_t time)
{
	if (sg.manager == nullptr)
		return;

	auto *lagCompensatedSet = (SimSet*)sg.manager->findObject(LagCompensatedSetId);

	if (lagCompensatedSet == nullptr)
		return;

	for (const auto object : lagCompensatedSet->objectList) {
		if (auto *player = (PlayerXT*)object.get(); player != exclude)
			player->startLagCompensation(time);
	}
}

void PlayerXT::endLagCompensationAll(const SimObject *exclude)
{
	if (sg.manager == nullptr)
		return;

	auto *lagCompensatedSet = (SimSet*)sg.manager->findObject(LagCompensatedSetId);

	if (lagCompensatedSet == nullptr)
		return;

	for (const auto object : lagCompensatedSet->objectList) {
		if (auto *player = (PlayerXT*)object.get(); player != exclude)
			player->endLagCompensation();
	}
}

void PlayerXT::saveLagCompensationSnapshotAll(uint32_t time)
{
	if (sg.manager == nullptr)
		return;

	auto *lagCompensatedSet = (SimSet*)sg.manager->findObject(LagCompensatedSetId);

	if (lagCompensatedSet == nullptr)
		return;

	for (const auto object : lagCompensatedSet->objectList) {
		auto *player = (PlayerXT*)object.get();
		player->saveLagCompensationSnapshot(time);
	}
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
			if (!hasFocus) {
				// Remote ghost
				updateMove(&lastPlayerMove, false);
				continue;
			}

			auto *psc = (PlayerPSCXT*)cg.psc;
			auto *move = psc->getClientMove(lastProcessTime);

			if (move == nullptr)
				break;

			const auto &subtickRecord = psc->getSubtick(lastProcessTime);

			float subtickPitch;
			float subtickYaw;

			if (subtickRecord.subtick != NoSubtick) {
				// Lets updateMove know not to update image states yet
				xt.currentSubtick = subtickRecord.subtick;
				subtickPitch = viewPitch + subtickRecord.pitch;
				subtickYaw = getRot().z + subtickRecord.yaw;
			}

			updateMove(move, false);

			if (hasSubtick()) {
				const auto tickStart = lastProcessTime - TickMs;
				const auto subtickTime = tickStart + subtickRecord.subtick;
				loadSnapshotInterpolated(subtickTime);
				setViewAnglesClamped(subtickPitch, subtickYaw);

				// Update weapon with subtick state
				updateWeapon(*move);

				// Restore
				loadSnapshot(lastProcessTime);
				xt.currentSubtick = NoSubtick;
			}

			lastPlayerMove = *move;
		} while (lastProcessTime < curTime);
	}

	loadSnapshotInterpolated(curTime);
}

void PlayerXT::updateWeapon(const PlayerMove &move)
{
	if (lastPlayerMove.trigger && !move.trigger)
		setImageTriggerUp(0);
	else if (!lastPlayerMove.trigger && move.trigger)
		setImageTriggerDown(0);

	for (auto i = 0; i < MaxItemImages; i++)
		updateImageState(i, 0.032f);
}

void PlayerXT::serverUpdateMove(const PlayerMove *moves, int moveCount)
{
	if (dead)
		return;

	for (auto index = 0; index < moveCount; index++) {
		if (updateDebt > 5)
			break;

		updateDebt++;

		const auto &move = moves[index];
		const auto &subtickRecord = xt.subtickRecords[index];
		const auto &lagCompensationRequest = xt.lagCompensationRequests[index];

		float subtickPitch;
		float subtickYaw;

		// Clients shouldn't send more than MaxMovesXT moves, but are allowed to
		if (index < MaxMovesXT) {
			if (subtickRecord.subtick != NoSubtick) {
				// Lets updateMove know not to update image states yet
				xt.currentSubtick = subtickRecord.subtick;
				subtickPitch = viewPitch + subtickRecord.pitch;
				subtickYaw = getRot().z + subtickRecord.yaw;
			}

			xt.currentLagCompensation = clamp(
				lagCompensationRequest.time,
				sg.currentTime - cvars::net::maxLagCompensation,
				sg.currentTime);
		}

		if (move.useItem != -1) {
			char buf[16];
			sprintf_s(buf, "%d", move.useItem);
			Console->executef(3, "remoteUseItem", scriptThis(), buf);
		}

		updateDamage(0.032f);
		updateMove(&move, true);
		updateAnimation(0.032f);

		if (hasSubtick()) {
			const auto tickStart = lastProcessTime - TickMs;
			const auto subtickTime = tickStart + subtickRecord.subtick;
			loadSnapshotInterpolated(subtickTime);
			setViewAnglesClamped(subtickPitch, subtickYaw);

			// Update weapon with subtick state
			updateWeapon(move);

			// Restore
			loadSnapshot(lastProcessTime);
			xt.currentSubtick = NoSubtick;
		}

		xt.currentLagCompensation = -1;

		lastPlayerMove = move;
	}

	if (mount == nullptr || mountPoint != 1)
		setMaskBits(OrientationMask);

	updateSkip = 0;
}

void PlayerXT::ghostSetMove(
	PlayerMove *move, const Point3F &newPos, const Point3F &newVel,
	bool newContact, float newRot, float newPitch, int skipCount, bool noInterp)
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

	if (serverNetcodeVersion >= Netcode::XT::ClockSync && cg.psc != nullptr) {
		// Translate the snapshot time to client time
		const auto *psc = (PlayerPSCXT*)cg.psc;
		const auto clockOffset = cg.currentTime - psc->xt.syncedClock;
		lastProcessTime = psc->xt.serverClock + clockOffset;
	} else {
		// Use average time of frame since we don't know when the packet arrived
		lastProcessTime = (cg.currentTime + cg.lastTime + 1) / 2;
	}

	// Adjust for time nudge and extra move tick
	lastProcessTime += cvars::net::timeNudge - TickMs;
	invalidatePrediction(lastProcessTime);
	saveSnapshot(lastProcessTime);

	// State sent by server is from before the move, so simulate once
	updateMove(move, false);
	lastPlayerMove = *move;
}

void PlayerXT::addPredictedProjectile(Projectile *projectile, int type)
{
	const auto &data = *projectile->m_projectileData;

	const auto direction = Point3F(projectile->getTransform().row<1>());
	const auto baseVelocity = direction * data.muzzleVelocity;
	const auto inheritedVelocity = projectile->m_shooterVel * data.inheritedVelocityScale;
	const auto velocity = baseVelocity + inheritedVelocity;
	projectile->setLinearVelocity(velocity);
	projectile->m_instTerminalVelocity = velocity;
}

void PlayerXT::clientFireImageProjectile(int imageSlot)
{
	const auto &itemImage = itemImageList[imageSlot];
	const auto &imageData = *getItemImageData(itemImage.imageId);

	// Normally calls Player::onFire on the server, seems unused though
	if (imageData.projectile.type == -1)
		return;

	TMat3F muzzleTransform;

	if (imageData.accuFire && !dead)
		muzzleTransform = getAimedMuzzleTransform(imageSlot);
	else
		muzzleTransform = getMuzzleTransform(imageSlot) * getTransform();

	auto *projectile = createProjectile(imageData.projectile);
	projectile->initProjectile(muzzleTransform, getLinearVelocity(), getId());
	projectile->netFlags.set(IsGhost);
	manager->addObject(projectile);
	addPredictedProjectile(projectile, imageData.projectile.type);
}