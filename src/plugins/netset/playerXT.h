#pragma once

#include "darkstar/Ml/ml.h"
#include "tribes/constants.h"
#include "tribes/player.h"
#include <bit>
#include <cstdint>

class PlayerXT : public Player {
public:
	static constexpr auto SNAP_HISTORY = std::bit_ceil((size_t)(1.0 * TICK_RATE));

	// Maps to Player::read/writePacketData fields
	struct Snapshot {
		uint32_t tick = -1;
		float yaw;
		Point3F position;
		Point3F velocity;
		float pitch;
		float energy;
		float traction;
		int jumpSurfaceLastContact;
		uint8_t pingStatus;
		bool contact;
		bool jetting;
		bool crouching;
	};

	struct DataXT {
		Snapshot snapshots[SNAP_HISTORY];
	} xt;

	Snapshot createSnapshot(uint32_t time) const
	{
		return {
			.tick = msToTicks(time),
			.yaw = getRot().z,
			.position = getLinearPosition(),
			.velocity = getLinearVelocity(),
			.pitch = viewPitch,
			.energy = energy,
			.traction = traction,
			.jumpSurfaceLastContact = jumpSurfaceLastContact,
			.pingStatus = getSensorPinged(),
			.contact = contact,
			.jetting = jetting,
			.crouching = crouching
		};
	}

	const Snapshot *getSnapshot(uint32_t time) const
	{
		const auto &snap = xt.snapshots[msToTicks(time) % SNAP_HISTORY];
		return snap.tick == msToTicks(time) ? &snap : nullptr;
	}

	void saveSnapshot(uint32_t time)
	{
		const auto index = msToTicks(time) % SNAP_HISTORY;
		xt.snapshots[index] = createSnapshot(time);
	}

	bool loadSnapshot(uint32_t time);
};