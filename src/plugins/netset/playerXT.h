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
	static constexpr auto SnapHistory = std::bit_ceil((size_t)(1.0 * TickRate));

	// Maps to Player::read/writePacketData fields
	struct Snapshot {
		static Snapshot interpolate(const Snapshot &a, const Snapshot &b, float t);

		uint32_t tick = -1;
		uint32_t time = -1;
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
		Snapshot snapshots[SnapHistory];
	};

	FIELD(Player::SIZEOF, DataXT, xt);

	static constexpr size_t SIZEOF = Player::SIZEOF + sizeof(DataXT);

	const Snapshot *getSnapshot(uint32_t time) const
	{
		const auto &snap = xt.snapshots[msToTicks(time) % SnapHistory];
		if (snap.tick != msToTicks(time))
			return nullptr;
		else if (snap.time > time)
			return getSnapshot(roundMsDownToTick(time) - 1);
		else
			return &snap;
	}

	Snapshot createSnapshot(uint32_t time = 0) const;

	void saveSnapshot(uint32_t time)
	{
		const auto index = msToTicks(time) % SnapHistory;
		xt.snapshots[index] = createSnapshot(time);
	}

	void invalidatePrediction(uint32_t time)
	{
		for (auto &snap : xt.snapshots) {
			if (snap.time >= time) {
				snap.tick = -1;
				snap.time = -1;
			}
		}
	}

	void loadSnapshot(const Snapshot &snapshot, bool useMouse = false);
	bool loadSnapshot(uint32_t time);
	bool loadSnapshotInterpolated(uint32_t time);
	
	void clientMove(uint32_t curTime);
	void serverUpdateMove(PlayerMove *moves, int moveCount);

	void ghostSetMove(
		PlayerMove *move, const Point3F &newPos, const Point3F &newVel,
		bool newContact, float newRot, float newPitch, int skipCount, bool noInterp,
		int timeNudge);
};