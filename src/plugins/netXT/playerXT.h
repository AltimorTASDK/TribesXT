#pragma once

#include "darkstar/Ml/ml.h"
#include "tribes/constants.h"
#include "tribes/player.h"
#include "plugins/netXT/playerPSCXT.h"
#include "util/struct.h"
#include <bit>
#include <cstddef>
#include <cstdint>

class PlayerXT : public Player {
	// Receive subtick and lag compensation data from PlayerPSCXT
	using SubtickRecord = PlayerPSCXT::SubtickRecord;
	using LagCompensationRequest = PlayerPSCXT::LagCompensationRequest;
	static constexpr auto MaxMovesXT = PlayerPSCXT::MaxMovesXT;
	static constexpr auto NoSubtick = PlayerPSCXT::NoSubtick;

public:
	static constexpr auto SnapHistory = std::bit_ceil((size_t)(1.0 * TickRate));

	// Maps to Player::read/writePacketData fields
	struct Snapshot {
		static Snapshot interpolate(const Snapshot &a, const Snapshot &b, float t);

		uint32_t time = -1;
		float yaw;
		Point3F position;
		Point3F velocity;
		float pitch;
		float energy;
		float traction;
		int lastContactCount;
		int jumpSurfaceLastContact;
		uint8_t pingStatus;
		bool contact;
		bool jetting;
		bool crouching;
	};

	struct SnapshotBuffer {
		Snapshot buffer[SnapHistory];

		const Snapshot *get(uint32_t time) const;
		const Snapshot *getNext(uint32_t time) const;
		const Snapshot *getPrev(uint32_t time) const;

		Snapshot &operator[](size_t index) { return buffer[index]; }
		const Snapshot &operator[](size_t index) const { return buffer[index]; }
	};

	struct DataXT {
		// Timed by lastProcessTime
		SnapshotBuffer snapshots;

		// Server only, timed by sg.currentTime
		SnapshotBuffer lagCompensationSnapshots;
		Snapshot lagCompensationBackup;

		// Server only
		SubtickRecord subtickRecords[MaxMovesXT];
		bool applySubtick = false;

		// Server only
		LagCompensationRequest lagCompensationRequests[MaxMovesXT];
		LagCompensationRequest lastLagCompensationRequest;
		bool applyLagCompensation = false;
	};

	FIELD(Player::SIZEOF, DataXT, xt);

	static constexpr size_t SIZEOF = Player::SIZEOF + sizeof(DataXT);

	Snapshot createSnapshot(uint32_t time = 0) const;

	void saveSnapshot(uint32_t time)
	{
		const auto index = msToTicks(time) % SnapHistory;
		xt.snapshots[index] = createSnapshot(time);
	}

	void loadSnapshot(const Snapshot &snapshot, bool useMouse = false);
	bool loadSnapshot(uint32_t time);
	bool loadSnapshotInterpolated(uint32_t time);
	bool startLagCompensation(uint32_t time);
	void endLagCompensation();

	static void startLagCompensationAll(uint32_t time);
	static void endLagCompensationAll();

	void invalidatePrediction(uint32_t time)
	{
		for (auto &snap : xt.snapshots.buffer) {
			if (snap.time >= time)
				snap.time = -1;
		}
	}

	void setViewAngles(float pitch, float yaw);
	void setViewAnglesClamped(float pitch, float yaw);
	
	void clientMove(uint32_t curTime);
	void updateWeapon(const PlayerMove &move);
	void serverUpdateMove(const PlayerMove *moves, int moveCount);

	void ghostSetMove(
		PlayerMove *move, const Point3F &newPos, const Point3F &newVel,
		bool newContact, float newRot, float newPitch, int skipCount, bool noInterp);
};