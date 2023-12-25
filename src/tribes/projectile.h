#pragma once

#include "tribes/gameBase.h"
#include "util/struct.h"

class ShapeBase;

class Projectile : public GameBase {
public:
	FIELD(0x344, ShapeBase*, m_pShooter);
	FIELD(0x350, Point3F, m_shooterVel);
};
