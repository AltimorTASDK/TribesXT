#include "darkstar/console/console.h"
#include "darkstar/Core/bitstream.h"
#include "darkstar/Sim/Net/packetStream.h"
#include "tribes/bullet.h"
#include "tribes/fearDcl.h"
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

auto PlayerXT::Snapshot::interpolateMs(const Snapshot &a, const Snapshot &b, uint32_t time) -> Snapshot
{
	return interpolate(a, b, (float)(time - a.time) / (b.time - a.time));
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

auto PlayerXT::ImageSnapshotBuffer::get(uint32_t moveCount) const -> const ImageSnapshot*
{
	const auto &snap = buffer[moveCount % SnapHistory];
	return snap.moveCount == moveCount ? &snap : nullptr;
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
		.lastContactCount = lastContactCount,
		.jumpSurfaceLastContact = jumpSurfaceLastContact,
		.pingStatus = getSensorPinged(),
		.contact = contact,
		.jetting = jetting,
		.crouching = crouching
	};
}

void PlayerXT::applyAccumulatedAim()
{
	if (!hasFocus)
		return;

	if (cg.packetStream == nullptr)
		return;

	if (cg.packetStream->streamMode == Net::PacketStream::PlaybackMode)
		return;

	// Use accumulated mouse input for local player
	if (const auto *snap = xt.snapshots.get(lastProcessTime); snap != nullptr) {
		const auto pitch = snap->pitch + cg.psc->curMove.pitch;
		const auto yaw = snap->yaw + cg.psc->curMove.turnRot;
		setViewAnglesClamped(pitch, yaw);
	}
}

// Corresponds to Player::readPacketData
void PlayerXT::loadSnapshot(const Snapshot &snapshot)
{
	setLinearVelocity(snapshot.velocity);
	setSensorPinged(snapshot.pingStatus);
	setRot({getRot().x, getRot().y, snapshot.yaw});
	setPos(snapshot.position);

	if (mount != nullptr) {
		const auto mountTransform = mount->getObjectMountTransform(mountPoint);
		setTransform(TMat3F(EulerF(getRot()), {0, 0, 0}) * mountTransform);
	}

	viewPitch = snapshot.pitch;
	energy = snapshot.energy;
	contact = snapshot.contact;
	jetting = snapshot.jetting;
	traction = snapshot.traction;

	if (crouching != snapshot.crouching) {
		crouching = snapshot.crouching;
		setAnimation(crouching ? ANIM_CROUCH : ANIM_STAND);
	}

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
		loadSnapshot(*a);
		return true;
	}

	const auto *b = xt.snapshots.getNext(time);
	if (b == nullptr)
		return false;

	loadSnapshot(Snapshot::interpolateMs(*a, *b, time));
	return true;
}

bool PlayerXT::loadSnapshotSubtick(uint32_t time)
{
	const auto *a = xt.snapshots.getPrev(time);
	if (a == nullptr)
		return false;

	if (a->time == time) {
		loadSnapshotPositionOnly(*a);
		return true;
	}

	const auto *b = xt.snapshots.getNext(time);
	if (b == nullptr)
		return false;

	loadSnapshotPositionOnly(Snapshot::interpolateMs(*a, *b, time));
	return true;
}

void PlayerXT::loadSnapshotPositionOnly(const Snapshot &snapshot)
{
	setPos(snapshot.position);

	if (mount != nullptr) {
		const auto mountTransform = mount->getObjectMountTransform(mountPoint);
		setTransform(TMat3F(EulerF(getRot()), {0, 0, 0}) * mountTransform);
	}
}

void PlayerXT::startLagCompensation(uint32_t time)
{
	const auto *a = xt.lagCompensationSnapshots.getPrev(time);
	const auto *b = xt.lagCompensationSnapshots.getNext(time);

	if (a == nullptr && b == nullptr)
		return;

	xt.lagCompensationBackup = createSnapshot();

	if (a != nullptr && (a->time == time || b == nullptr)) {
		loadSnapshotPositionOnly(*a);
		xt.lagCompensationTarget = *a;
		return;
	}
	
	if (b != nullptr && a == nullptr) {
		loadSnapshotPositionOnly(*b);
		xt.lagCompensationTarget = *b;
		return;
	}

	const auto snapshot = Snapshot::interpolateMs(*a, *b, time);
	loadSnapshotPositionOnly(snapshot);
	xt.lagCompensationTarget = snapshot;
}

void PlayerXT::endLagCompensation()
{
	if (!xt.lagCompensationTarget.isValid())
		return;

	loadSnapshotPositionOnly(xt.lagCompensationBackup);

	xt.lagCompensationTarget.invalidate();
}

void PlayerXT::startLagCompensationAll(const SimObject *exclude, uint32_t time)
{
	if (sg.manager == nullptr)
		return;

	for (const auto player : SimSet::iterate<PlayerXT>(LagCompensatedSetId)) {
		if (player.get() != exclude)
			player->startLagCompensation(time);
	}
}

void PlayerXT::endLagCompensationAll(const SimObject *exclude)
{
	if (sg.manager == nullptr)
		return;

	for (const auto player : SimSet::iterate<PlayerXT>(LagCompensatedSetId)) {
		if (player.get() != exclude)
			player->endLagCompensation();
	}
}

void PlayerXT::saveLagCompensationSnapshotAll(uint32_t time)
{
	if (sg.manager == nullptr)
		return;

	for (const auto player : SimSet::iterate<PlayerXT>(LagCompensatedSetId))
		player->saveLagCompensationSnapshot(time);
}

auto PlayerXT::createImageSnapshot(uint32_t moveCount) const -> ImageSnapshot
{
	auto snapshot = ImageSnapshot{.moveCount = moveCount};

	for (auto i = 0; i < MaxItemImages; i++) {
		const auto &image = itemImageList[i];
		snapshot.images[i].typeId = image.typeId;
		snapshot.images[i].delayTime = image.delayTime;
		snapshot.images[i].triggerDown = image.triggerDown;
		snapshot.images[i].ammo = image.ammo;
	}

	return snapshot;
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

void PlayerXT::updateItem(const PlayerMove &move)
{
	const auto item = move.useItem;

	if (item == -1)
		return;

	if (!isGhost()) {
		char buf[16];
		sprintf_s(buf, "%d", item);
		Console->executef(3, "remoteUseItem", scriptThis(), buf);
		return;
	}

	if (isRollbackPreMove())
		return;

	if (cg.psc->itemCount(item) == 0)
		return;

	const auto *data = getItemData(item);

	if (data == nullptr)
		return;

	const auto *className = data->className;

	// Must be a weapon
	if (className == nullptr || _stricmp(className, "Weapon") != 0)
		return;

	const auto *image = getItemImageData(data->imageId);

	if (image == nullptr)
		return;

	// Must have ammo or be an energy weapon
	if (image->ammoType != -1 && cg.psc->itemCount(image->ammoType) <= 0)
		return;

	// Predict weapon swap
	mountItem(item, 0, -1);
}

void PlayerXT::updateWeapon(const PlayerMove &move)
{
	if (isRollback())
		return;

	if (xt.lastTrigger && !move.trigger)
		setImageTriggerUp(0);
	else if (!xt.lastTrigger && move.trigger)
		setImageTriggerDown(0);

	xt.lastTrigger = move.trigger;

	for (auto i = 0; i < MaxItemImages; i++)
		updateImageState(i, 0.032f);
}

void PlayerXT::serverUpdateMove(const PlayerMove *moves, int moveCount)
{
	if (dead)
		return;

	for (auto index = 0; index < moveCount; index++) {
		if (updateDebt > 5)
			continue;

		updateDebt++;

		const auto &move = moves[index];
		const auto &subtickRecord = xt.subtickRecords[index];
		const auto &lagCompensationRequest = xt.lagCompensationRequests[index];

		xt.currentLagCompensation = clamp(
			lagCompensationRequest.time,
			sg.currentTime - cvars::net::maxLagCompensation,
			sg.currentTime);

		updateDamage(0.032f);

		if (subtickRecord.subtick != NoSubtick) {
			const auto lastTickTime = lastProcessTime - TickMs;
			const auto subtickTime = lastTickTime + subtickRecord.subtick;
			const auto subtickPitch = viewPitch + subtickRecord.pitch;
			const auto subtickYaw = getRot().z + subtickRecord.yaw;
			xt.currentSubtick = subtickRecord.subtick;

			// Beacons don't get subtick because they have to be synced with movement
			updateItem(move);

			updateMove(&move, true);

			// Update weapon with subtick state
			loadSnapshotSubtick(subtickTime);
			setViewAnglesClamped(subtickPitch, subtickYaw);
			updateWeapon(move);

			// Restore
			loadSnapshot(lastProcessTime);
			xt.currentSubtick = NoSubtick;
		} else {
			updateItem(move);
			updateMove(&move, true);
			updateWeapon(move);
		}

		updateAnimation(0.032f);

		xt.currentLagCompensation = -1;

		lastPlayerMove = move;
		xt.moveCount++;
	}

	if (mount == nullptr || mountPoint != 1)
		setMaskBits(OrientationMask);

	updateSkip = 0;
}

// clientProcess partial reimplementation
bool PlayerXT::clientTickOnce()
{
	if (!hasFocus) {
		// Remote ghost
		updateMove(&lastPlayerMove, false);
		return true;
	}

	auto *psc = (PlayerPSCXT*)cg.psc;
	auto *move = psc->getClientMove(lastProcessTime);

	if (move == nullptr)
		return false;

	const auto &subtickRecord = psc->getSubtick(lastProcessTime);

	if (subtickRecord.subtick != NoSubtick) {
		const auto lastTickTime = lastProcessTime - TickMs;
		const auto subtickTime = lastTickTime + subtickRecord.subtick;
		const auto subtickPitch = viewPitch + subtickRecord.pitch;
		const auto subtickYaw = getRot().z + subtickRecord.yaw;
		xt.currentSubtick = subtickRecord.subtick;

		// Beacons don't get subtick because they have to be synced with movement
		updateItem(*move);

		updateMove(move, false);

		// Update weapon with subtick state
		loadSnapshotSubtick(subtickTime);
		setViewAnglesClamped(subtickPitch, subtickYaw);
		updateWeapon(*move);

		// Restore
		loadSnapshot(lastProcessTime);
		xt.currentSubtick = NoSubtick;
	} else {
		updateItem(*move);
		updateMove(move, false);
		updateWeapon(*move);
	}

	lastPlayerMove = *move;
	xt.maxProcessTime = std::max(xt.maxProcessTime, lastProcessTime);
	xt.moveCount++;
	return true;
}

void PlayerXT::clientMove(uint32_t curTime)
{
	if (!hasFocus && curTime > xt.lastGhostRecvTime + cvars::net::ghostTimeout) {
		// ghost timed out, snap back to last server position and stop moving
		loadSnapshot(xt.lastGhostProcessTime);
		return;
	}

	if (lastProcessTime < curTime) {
		loadSnapshot(lastProcessTime);
		do {
			if (!clientTickOnce())
				break;
		} while (lastProcessTime < curTime);
	}

	loadSnapshotInterpolated(curTime);
	applyAccumulatedAim();
}

void PlayerXT::packUpdateXT(Net::GhostManager *gm, uint32_t mask, BitStream *stream)
{
	stream->writeInt(xt.jumpCount, 3);
}

void PlayerXT::unpackUpdateXT(Net::GhostManager *gm, BitStream *stream)
{
	if (!Netcode::XT::SendCurrentPlayerState.check())
		return;

	const auto oldJumpCount = xt.jumpCount;
	xt.jumpCount = stream->readInt(3);

	// Let the server drive ghost jump animations because we may not run the move
	if (xt.jumpCount != oldJumpCount && !hasFocus && !dead) {
		if (currentAnimation != ANIM_JUMPRUN && currentAnimation != ANIM_LAND)
			setAnimation(ANIM_JUMPRUN);
	}
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

	if (Netcode::XT::ClockSync.check() && cg.psc != nullptr) {
		// Translate the snapshot time to client time
		const auto *psc = (PlayerPSCXT*)cg.psc;
		const auto clockOffset = psc->xt.serverClock - psc->xt.syncedClock;
		lastProcessTime = cg.currentTime + clockOffset;
	} else {
		// Use average time of frame since we don't know when the packet arrived
		lastProcessTime = (cg.currentTime + cg.lastTime + 1) / 2;
	}

	// Adjust for time nudge
	lastProcessTime += cvars::net::timeNudge;

	// Adjust for extra move tick
	if (!Netcode::XT::SendCurrentPlayerState.check())
		lastProcessTime -= TickMs;

	invalidatePrediction(lastProcessTime);
	saveSnapshot(lastProcessTime);

	if (Netcode::XT::SendCurrentPlayerState.check()) {
		// Set fields that would've been set by updateMove
		falling = newVel.z <= -10.f;
	} else {
		// State sent by server is from before the move, so simulate once
		updateMove(move, false);
	}

	lastPlayerMove = *move;

	xt.lastGhostRecvTime = cg.currentTime;
	xt.lastGhostProcessTime = lastProcessTime;
}

void PlayerXT::initProjectileXT(Projectile *projectile)
{
	if (hasSubtick())
		projectile->subtickOffsetXT = xt.currentSubtick;

	if (hasLagCompensation())
		projectile->lagCompensationOffsetXT = sg.currentTime - xt.currentLagCompensation;

	projectile->predictionKeyXT = xt.moveCount;
	projectile->spawnTimeXT = wg->currentTime;
}

void PlayerXT::initPredictedProjectile(Projectile *projectile, int type)
{
	projectile->m_pShooter = this;

	const auto &data = *projectile->m_projectileData;

	const auto direction = Point3F(projectile->getTransform().row<1>());
	const auto baseVelocity = direction * data.muzzleVelocity;
	const auto inheritedVelocity = projectile->m_shooterVel * data.inheritedVelocityScale;
	const auto velocity = baseVelocity + inheritedVelocity;
	projectile->setLinearVelocity(velocity);
	projectile->deflectProjectile(data.aimDeflection);
	projectile->m_instTerminalVelocity = getLinearVelocity();

	// Match the passage of time on the server
	projectile->spawnTimeXT = lastProcessTime - TickMs;

	if (hasSubtick()) {
		// Match visual to subtick lag compensation
		projectile->spawnTimeXT += xt.currentSubtick - TickMs;
	}

	projectile->m_lastUpdated = projectile->spawnTimeXT;

	if (projectile->getGhostTag() == BulletPersTag) {
		auto *bullet = (Bullet*)projectile;
		bullet->m_spawnTime = bullet->spawnTimeXT;
		bullet->m_spawnVelocity = bullet->getLinearVelocity();
		bullet->m_spawnVelocityLen = bullet->getLinearVelocity().length();
		bullet->m_spawnPosition = bullet->getLinearPosition();
		bullet->setSearchBoundaries();
	}
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
	initProjectileXT(projectile);

	manager->addObject(projectile);
	projectile->addToSet(ClientProjectileSetId);

	initPredictedProjectile(projectile, imageData.projectile.type);
}