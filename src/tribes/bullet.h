#pragma once

#include "darkstar/Ml/ml.h"
#include "tribes/projectile.h"
#include "util/struct.h"

class Bullet : public Projectile {
public:
	class TracerRenderImage {
	public:
		FIELD(0x18, Point3F, m_endPoint);
		FIELD(0x24, Point3F, m_startPoint);
	};

	FIELD(0x494, uint32_t, m_spawnTime);
	FIELD(0x498, Point3F, m_spawnPosition);
};
