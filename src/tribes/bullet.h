#pragma once

#include "darkstar/Ml/ml.h"
#include "darkstar/Sim/simRenderGrp.h"
#include "tribes/projectile.h"
#include "util/meta.h"
#include "util/struct.h"

class Bullet : public Projectile {
public:
	class BulletData {
	public:
		FIELD(0xA8, float, tracerLength);
	};

	class TracerRenderImage : public SimRenderImage {
	public:
		FIELD(0x18, Point3F, m_endPoint);
		FIELD(0x24, Point3F, m_startPoint);
		FIELD(0x30, float, m_factor);
	};

	FIELD(0x494, uint32_t, m_spawnTime);
	FIELD(0x498, Point3F, m_spawnPosition);
	FIELD(0x4A4, Point3F, m_spawnVelocity);
	FIELD(0x4B0, float, m_spawnVelocityLen);
	FIELD(0x4B4, Point3F, m_spawnDirection);
	FIELD(0x5D0, BulletData*, m_pBulletData);

	void setSearchBoundaries()
	{
		using func_t = to_static_function_t<decltype(&Bullet::setSearchBoundaries)>;
		((func_t)0x4BF650)(this);
	}
};
