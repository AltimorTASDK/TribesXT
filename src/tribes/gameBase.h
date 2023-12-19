#pragma once

#include "darkstar/Sim/simMovement.h"
#include "util/struct.h"
#include <cstdint>

class GameBase : public SimMovement {
public:
	static constexpr uint8_t SensorPingedBase = 24;

	FIELD(0x214, uint32_t, sensorInfoBits);
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
};
