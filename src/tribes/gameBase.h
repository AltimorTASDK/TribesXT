#pragma once

#include "darkstar/Ml/ml.h"
#include "darkstar/Sim/simMovement.h"
#include "util/memory.h"
#include "util/struct.h"
#include <cstdint>

class GameBase : public SimMovement {
public:
	static constexpr uint8_t SensorPingedBase = 24;

	FIELD(0x214, uint32_t, sensorInfoBits);
	FIELD(0x224, uint32_t, ownerId);
	FIELD(0x22C, uint32_t, lastProcessTime);

	void setSensorPinged(uint8_t fPinged)
	{
		sensorInfoBits = (sensorInfoBits & ((1 << SensorPingedBase) - 1)) |
		                 ((uint32_t)fPinged << SensorPingedBase);
	}

	uint8_t getSensorPinged() const
	{
		return (uint8_t)(sensorInfoBits >> SensorPingedBase);
	}

	void getObjectMountTransform(int mountPoint, TMat3F *mat) const
	{
		call_virtual<91, void(GameBase::*)(int, TMat3F*) const>(this, mountPoint, mat);
	}

	TMat3F getObjectMountTransform(int mountPoint) const
	{
		TMat3F transform;
		getObjectMountTransform(mountPoint, &transform);
		return transform;
	}
};
