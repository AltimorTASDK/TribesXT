#pragma once

#include "darkstar/Ml/ml.h"
#include "tribes/projectile.h"
#include "util/struct.h"

class Bullet : public Projectile {
public:
	class BulletData {
	public:
		FIELD(0xA8, float, tracerLength);
	};

	class TracerRenderImage {
	public:
		FIELD(0x18, Point3F, m_endPoint);
		FIELD(0x24, Point3F, m_startPoint);
	};

	FIELD(0x494, uint32_t, m_spawnTime);
	FIELD(0x498, Point3F, m_spawnPosition);
	FIELD(0x5D0, BulletData*, m_pBulletData);
};
