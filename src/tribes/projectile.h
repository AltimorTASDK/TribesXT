#pragma once

#include "darkstar/SimObjects/fxRenderImage.h"
#include "tribes/gameBase.h"
#include "util/memory.h"
#include "util/meta.h"
#include "util/struct.h"

class BitStream;
class ShapeBase;

class Projectile : public GameBase {
public:
	FIELD(0x25C, fxRenderImage, m_renderImage);
	FIELD(0x2FC, uint32_t, m_startInterp);
	FIELD(0x300, uint32_t, m_endInterp);
	FIELD(0x304, Point3F, m_interpFrom);
	FIELD(0x310, Point3F, m_interpTo);
	FIELD(0x31C, uint32_t, m_lastUpdated);
	FIELD(0x344, ShapeBase*, m_pShooter);
	FIELD(0x350, Point3F, m_shooterVel);

	// Reuse these fields on the server since Projectile is a base class
	// and can't be easily extended
	FIELD(0x2FC, uint32_t, subtickOffsetXT);
	FIELD(0x300, uint32_t, lagCompensationOffsetXT);

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

	bool hasSubtick() const
	{
		return subtickOffsetXT != -1;
	}

	bool hasLagCompensation() const
	{
		return lagCompensationOffsetXT != -1;
	}
};
