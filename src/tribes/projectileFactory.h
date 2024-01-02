#pragma once

class Projectile;

struct ProjectileDataType {
	int type;
	int dataType;
};

inline Projectile *createProjectile(const ProjectileDataType &projectile)
{
	using func_t = decltype(&createProjectile);
	return ((func_t)0x4C2490)(projectile);
}
