#pragma once

#include "darkstar/Core/encryptedPointer.h"
#include "darkstar/SimObjects/fxRenderImage.h"
#include "tribes/gameBase.h"
#include "util/memory.h"
#include "util/meta.h"
#include "util/struct.h"
#include <cstdint>

class BitStream;
class ResourceObject;
class ShapeBase;

class Projectile : public GameBase {
public:
	struct ProjectileData : GameBaseData {
		const char *bulletShapeName;
		int explosionTag;
		int expRandCycle;
		bool collideWithOwner;
		uint32_t ownerGraceMS;
		float collisionRadius;
		float mass;
		int damageClass;
		float damageValue;
		int damageType;
		float explosionRadius;
		float kickBackStrength;
		float aimDeflection;
		float muzzleVelocity;
		float terminalVelocity;
		float acceleration;
		float totalTime;
		float liveTime;
		float projSpecialTime;
		bool projSpecialBool;
		float lightRange;
		ColorF lightColor;
		float inheritedVelocityScale;
		int soundId;
		ResourceObject *m_projRes;
	};

	static_assert(sizeof(ProjectileData) == 0xA0);

	FIELD(0x254, EncryptedPointer<ProjectileData>, m_projectileData);
	FIELD(0x25C, fxRenderImage, m_renderImage);
	FIELD(0x2FC, uint32_t, m_startInterp);
	FIELD(0x300, uint32_t, m_endInterp);
	FIELD(0x304, Point3F, m_interpFrom);
	FIELD(0x310, Point3F, m_interpTo);
	FIELD(0x31C, uint32_t, m_lastUpdated);
	FIELD(0x344, ShapeBase*, m_pShooter);
	FIELD(0x350, Point3F, m_shooterVel);
	FIELD(0x35C, Point3F, m_instTerminalVelocity);

	// Reuse these fields since Projectile is a base class and can't be easily extended
	FIELD(0x2FC, uint32_t, subtickOffsetXT);
	FIELD(0x300, uint32_t, lagCompensationOffsetXT);
	FIELD(0x304, uint32_t, predictionKeyXT);
	FIELD(0x308, uint32_t, spawnTimeXT);

	void initProjectile(
		const TMat3F &in_rTrans, const Point3F &in_rShooterVel, SimObjectId in_shooterId)
	{
		call_virtual<109, decltype(&Projectile::initProjectile)>(
			this, in_rTrans, in_rShooterVel, in_shooterId);
	}

	void readInitialPacket(Net::GhostManager *io_pGM, BitStream *io_pStream)
	{
		using func_t = to_static_function_t<decltype(&Projectile::readInitialPacket)>;
		((func_t)0x4C0E00)(this, io_pGM, io_pStream);
	}

	void writeInitialPacket(Net::GhostManager *io_pGM, BitStream *io_pStream)
	{
		using func_t = to_static_function_t<decltype(&Projectile::writeInitialPacket)>;
		((func_t)0x4C0EB0)(this, io_pGM, io_pStream);
	}

	void deflectProjectile(float in_deflection)
	{
		using func_t = to_static_function_t<decltype(&Projectile::deflectProjectile)>;
		((func_t)0x4C1120)(this, in_deflection);
	}

	bool hasSubtick() const
	{
		return subtickOffsetXT != -1;
	}

	bool hasLagCompensation() const
	{
		return lagCompensationOffsetXT != -1;
	}
};
