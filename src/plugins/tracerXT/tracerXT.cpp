#include "tribes/bullet.h"
#include "tribes/constants.h"
#include "tribes/shapeBase.h"
#include "tribes/worldGlobals.h"
#include "plugins/tracerXT/tracerXT.h"
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

void TracerXTPlugin::hook_Bullet_onSimRenderQueryImage_setWidth(CpuState &cs)
{
	*(float*)(cs.reg.esp + 0x30) = tracerWidth;
}

void TracerXTPlugin::hook_Bullet_readInitialPacket_setSpawnTime(CpuState &cs)
{
	auto *bullet = (Bullet*)cs.reg.ebx;
	auto *elapsed = (uint32_t*)&cs.reg.eax;

	// Work around the poor calculation from vanilla servers
	*elapsed = std::max(*elapsed, TickMs);

	bullet->m_spawnTime = cg.currentTime - *elapsed;
}

void TracerXTPlugin::hook_Bullet_writeInitialPacket_setElapsed(CpuState &cs)
{
	auto *bullet = (Bullet*)cs.reg.esi;
	cs.reg.ebp = sg.currentTime - bullet->m_spawnTime;
}

void TracerXTPlugin::hook_Bullet_onAdd_client(CpuState &cs)
{
	auto *bullet = (Bullet*)cs.reg.esi;
	if (bullet->m_pShooter != nullptr)
		bullet->m_shooterVel = bullet->m_pShooter->getLinearVelocity();
}

void TracerXTPlugin::hook_Bullet_onSimRenderQueryImage(
	Bullet *bullet, edx_t, SimRenderQueryImage *image)
{
	// Pull inherited velocity out for tracer calcs
	const auto velocity = bullet->getLinearVelocity();
	bullet->setLinearVelocity(velocity - bullet->m_shooterVel);

	// Apply custom length
	const auto baseLength = bullet->m_pBulletData->tracerLength;
	bullet->m_pBulletData->tracerLength *= tracerLength;

	get()->hooks.Bullet.onSimRenderQueryImage.callOriginal(bullet, image);

	bullet->m_pBulletData->tracerLength = baseLength;
	bullet->setLinearVelocity(velocity);
}

void TracerXTPlugin::init()
{
	console->addVariable(0, "tracer::width", CMDConsole::Float, &tracerWidth);
	console->addVariable(0, "tracer::length", CMDConsole::Float, &tracerLength);
}