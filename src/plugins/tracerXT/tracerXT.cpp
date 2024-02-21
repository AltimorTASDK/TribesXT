#include "darkstar/Core/bitstream.h"
#include "tribes/bullet.h"
#include "tribes/constants.h"
#include "tribes/shapeBase.h"
#include "tribes/worldGlobals.h"
#include "plugins/netXT/version.h"
#include "plugins/tracerXT/tracerXT.h"
#include "util/math.h"
#include "nofix/x86Hook.h"
#include <algorithm>
#include <bit>
#include <cstddef>

void TracerXTPlugin::hook_Bullet_TracerRenderImage_render_orientation(CpuState &cs)
{
	const auto *image = (Bullet::TracerRenderImage*)cs.reg.esi;
	const auto &bulletVec = *(Point3F*)(cs.reg.esp + 0x18);
	const auto *camera = *(std::byte**)(cs.reg.edi + 4);
	const auto &cameraMatrix = *(TMat3F*)(camera + 0x70);

	const auto cameraToBullet = image->m_endPoint - cameraMatrix.p;
	const auto normalVec = Point3F::cross(bulletVec, cameraToBullet).normalized();
	*(Point3F*)(cs.reg.esp + 0xC) = normalVec;
}

void TracerXTPlugin::hook_Bullet_writeInitialPacket(
	Bullet *bullet, edx_t, Net::GhostManager *ghostManager, BitStream *stream)
{
	bullet->Projectile::writeInitialPacket(ghostManager, stream);

	stream->write(bullet->m_spawnPosition);

	const auto elapsedMs = sg.currentTime - bullet->m_spawnTime;
	if (bullet->hasSubtick())
		stream->writeInt(elapsedMs + TickMs - bullet->subtickOffsetXT, 15);
	else
		stream->writeInt(elapsedMs, 15);

	const auto direction = bullet->m_spawnVelocity / bullet->m_spawnVelocityLen;
	stream->writeNormalVector(direction, 20);
	stream->writeIntClamped((int)(bullet->m_spawnVelocityLen * 16), 14);

	// Using Player::packUpdate velocity quantization
	const auto shooterSpeed = bullet->m_shooterVel.length();
	const auto shooterDirection = bullet->m_shooterVel / shooterSpeed;
	stream->writeIntClamped((int)(shooterSpeed * 512), 17);
	stream->writeNormalVector(shooterDirection, 10);
}

void TracerXTPlugin::hook_Bullet_readInitialPacket(
	Bullet *bullet, edx_t, Net::GhostManager *ghostManager, BitStream *stream)
{
	bullet->Projectile::readInitialPacket(ghostManager, stream);

	stream->read(&bullet->m_spawnPosition);

	// Work around the poor calculation from vanilla servers
	const auto elapsedMs = std::max(stream->readUInt(15), TickMs) - TickMs;
	const auto elapsedSecs = msToSecs(elapsedMs);

	// Start tracer from initial position
	bullet->m_spawnTime = cg.currentTime - elapsedMs;

	bullet->m_spawnDirection = stream->readNormalVector(20);
	bullet->m_renderImage.faceDirection(bullet->m_spawnDirection);

	bullet->m_spawnVelocityLen = stream->readInt(14) / 16.f;
	bullet->m_spawnVelocity = bullet->m_spawnDirection * bullet->m_spawnVelocityLen;
	bullet->setLinearVelocity(bullet->m_spawnVelocity);

	const auto position = bullet->m_spawnPosition + bullet->m_spawnVelocity * elapsedSecs;
	bullet->setTransform({bullet->getRotation(), position});

	if (serverNetcodeVersion >= Netcode::XT::TracerInheritance) {
		// Using Player::packUpdate velocity quantization
		const auto shooterSpeed = stream->readInt(17) / 512.f;
		const auto shooterDirection = stream->readNormalVector(10);
		bullet->m_shooterVel = shooterDirection * shooterSpeed;
	} else {
		// Fall back to current velocity
		if (bullet->m_pShooter != nullptr)
			bullet->m_shooterVel = bullet->m_pShooter->getLinearVelocity();
		else
			bullet->m_shooterVel = {};
	}
}

void TracerXTPlugin::hook_Bullet_onSimRenderQueryImage(
	Bullet *bullet, edx_t, SimRenderQueryImage *image)
{
	// Pull inherited velocity out for tracer calcs
	const auto &data = *bullet->m_projectileData;
	const auto velocity = bullet->getLinearVelocity();
	const auto inheritedVelocity = bullet->m_shooterVel * data.inheritedVelocityScale;
	bullet->setLinearVelocity(velocity - inheritedVelocity);

	// Apply custom length
	const auto baseLength = bullet->m_pBulletData->tracerLength;
	bullet->m_pBulletData->tracerLength *= clamp(cvars::pref::tracerLength, 0.f, 1.f);

	get()->hooks.Bullet.onSimRenderQueryImage.callOriginal(bullet, image);

	bullet->m_pBulletData->tracerLength = baseLength;
	bullet->setLinearVelocity(velocity);
}

void TracerXTPlugin::hook_Bullet_onSimRenderQueryImage_setWidth(CpuState &cs)
{
	*(float*)(cs.reg.esp + 0x30) = clamp(cvars::pref::tracerWidth, 0.f, 1.f);
}

void TracerXTPlugin::init()
{
	console->addVariable(0, "pref::tracerWidth",  CMDConsole::Float, &cvars::pref::tracerWidth);
	console->addVariable(0, "pref::tracerLength", CMDConsole::Float, &cvars::pref::tracerLength);
}