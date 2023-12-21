#pragma once

#include "darkstar/Ml/ml.h"
#include "tribes/constants.h"
#include "tribes/player.h"
#include "util/struct.h"
#include <bit>
#include <cstddef>
#include <cstdint>

class PlayerXT : public Player {
public:
	static constexpr auto SNAP_HISTORY = std::bit_ceil((size_t)(1.0 * TICK_RATE));

	// Maps to Player::read/writePacketData fields
	struct Snapshot {
		static Snapshot interpolate(const Snapshot &a, const Snapshot &b, float t);

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
	};

	FIELD(Player::SIZEOF, DataXT, xt);

	static constexpr size_t SIZEOF = Player::SIZEOF + sizeof(DataXT);

	const Snapshot *getSnapshot(uint32_t time) const
	{
		const auto &snap = xt.snapshots[msToTicks(time) % SNAP_HISTORY];
		return snap.tick == msToTicks(time) ? &snap : nullptr;
	}

	Snapshot createSnapshot(uint32_t time) const;

	void saveSnapshot(uint32_t time)
	{
		const auto index = msToTicks(time) % SNAP_HISTORY;
		xt.snapshots[index] = createSnapshot(time);
	}

	void invalidatePrediction(uint32_t time)
	{
		for (auto &snap : xt.snapshots) {
			if (snap.tick >= msToTicksRoundUp(time))
				snap.tick = -1;
		}
	}

	void loadSnapshot(const Snapshot &snapshot, bool useMouseInput = false);
	bool loadSnapshot(uint32_t time);
	bool loadSnapshotInterpolated(uint32_t time);
	
	void clientMove(unsigned int curTime);
};