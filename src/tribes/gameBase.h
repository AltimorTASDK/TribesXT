#pragma once

#include "darkstar/Ml/ml.h"
#include "darkstar/Sim/simMovement.h"
#include "util/memory.h"
#include "util/struct.h"
#include <cstddef>
#include <cstdint>

class GameBase : public SimMovement {
public:
	struct GameBaseData {
		std::byte padding00[0x34];
	};

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

	float getDamageLevel() const
	{
		return call_virtual<62, decltype(&GameBase::getDamageLevel)>(this);
	}

	float getEnergyLevel() const
	{
		return call_virtual<63, decltype(&GameBase::getEnergyLevel)>(this);
	}

	bool getMuzzleTransform(int slot, TMat3F *mat) const
	{
		return call_virtual<67, bool(GameBase::*)(int, TMat3F*) const>(this, slot, mat);
	}

	TMat3F getMuzzleTransform(int slot) const
	{
		TMat3F mat;
		getMuzzleTransform(slot, &mat);
		return mat;
	}

	void clientProcess(uint32_t curTime)
	{
		call_virtual<85, decltype(&GameBase::clientProcess)>(this, curTime);
	}

	void serverProcess(uint32_t curTime)
	{
		call_virtual<86, decltype(&GameBase::serverProcess)>(this, curTime);
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
