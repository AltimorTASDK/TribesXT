#pragma once

enum PersTag {
	BulletPersTag     = 600,
	GrenadePersTag    = 601,
	RocketDumbPersTag = 602,
};

inline bool isProjectileTag(uint32_t tag)
{
	switch (tag) {
	case BulletPersTag:
	case GrenadePersTag:
	case RocketDumbPersTag:
		return true;
	default:
		return false;
	}
}
