#pragma once

#include "darkstar/SimObjects/fxRenderImage.h"
#include "tribes/gameBase.h"
#include "util/meta.h"
#include "util/struct.h"

class BitStream;
class ShapeBase;

class Projectile : public GameBase {
public:
	FIELD(0x25C, fxRenderImage, m_renderImage);
	FIELD(0x344, ShapeBase*, m_pShooter);
	FIELD(0x350, Point3F, m_shooterVel);

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
};
