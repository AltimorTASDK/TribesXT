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

		bool isValid() const
		{
			return time != -1;
		}

		void invalidate()
		{
			time = -1;
		}
	};

	struct SnapshotBuffer {
		Snapshot buffer[SnapHistory];

		const Snapshot *get(uint32_t time) const;
		const Snapshot *getNext(uint32_t time) const;
		const Snapshot *getPrev(uint32_t time) const;

		Snapshot &operator[](size_t index) { return buffer[index]; }
		const Snapshot &operator[](size_t index) const { return buffer[index]; }
	};

	struct ImageSnapshot {
		uint32_t moveCount = -1;
		struct {
			int typeId;
			float delayTime;
			bool triggerDown;
			bool ammo;
		} images[MaxItemImages];

		bool isValid() const
		{
			return moveCount != -1;
		}

		void invalidate()
		{
			moveCount = -1;
		}
	};

	struct ImageSnapshotBuffer {
		ImageSnapshot buffer[SnapHistory];

		const ImageSnapshot *get(uint32_t moveCount) const;

		ImageSnapshot &operator[](size_t index) { return buffer[index]; }
		const ImageSnapshot &operator[](size_t index) const { return buffer[index]; }
	};

	struct DataXT {
		// Timed by lastProcessTime
		SnapshotBuffer snapshots;

		// Timed by xt.moveCount
		ImageSnapshotBuffer imageSnapshots;

		// Server only, timed by sg.currentTime
		SnapshotBuffer lagCompensationSnapshots;
		Snapshot lagCompensationTarget;
		Snapshot lagCompensationBackup;

		// Server only
		SubtickRecord subtickRecords[MaxMovesXT];
		// Server + client
		uint8_t currentSubtick = NoSubtick;

		// Server only
		LagCompensationRequest lagCompensationRequests[MaxMovesXT];
		uint32_t currentLagCompensation = -1;

		// Server + client
		uint32_t maxProcessTime = 0;
		uint32_t moveCount = 0;
		bool lastTrigger = false;
	};

	FIELD(Player::SIZEOF, DataXT, xt);

	static constexpr size_t SIZEOF = Player::SIZEOF + sizeof(DataXT);

	bool hasSubtick() const
	{
		return xt.currentSubtick != NoSubtick;
	}

	bool hasLagCompensation() const
	{
		return xt.currentLagCompensation != -1;
	}

	bool isRollback() const
	{
		return isGhost() && lastProcessTime <= xt.maxProcessTime;
	}

	Snapshot createSnapshot(uint32_t time = 0) const;

	void saveSnapshot(uint32_t time)
	{
		const auto index = msToTicks(time) % SnapHistory;
		xt.snapshots[index] = createSnapshot(time);
	}

	// Instantly apply mouse input to rotation
	void applyAccumulatedAim();

	void loadSnapshot(const Snapshot &snapshot);
	bool loadSnapshot(uint32_t time);
	bool loadSnapshotInterpolated(uint32_t time);
	void loadSnapshotLagCompensation(const Snapshot &snapshot);

	void saveLagCompensationSnapshot(uint32_t time)
	{
		const auto index = msToTicks(time) % SnapHistory;
		xt.lagCompensationSnapshots[index] = createSnapshot(time);
	}

	void startLagCompensation(uint32_t time);
	void endLagCompensation();

	static void startLagCompensationAll(const SimObject *exclude, uint32_t time);
	static void endLagCompensationAll(const SimObject *exclude);
	static void saveLagCompensationSnapshotAll(uint32_t time);

	void invalidatePrediction(uint32_t time)
	{
		for (auto &snapshot : xt.snapshots.buffer) {
			if (snapshot.time >= time)
				snapshot.invalidate();
		}
	}

	ImageSnapshot createImageSnapshot(uint32_t moveCount) const;

	void saveItemImageSnapshot(uint32_t moveCount)
	{
		xt.imageSnapshots[moveCount % SnapHistory] = createImageSnapshot(moveCount);
	}

	void invalidateImagePrediction(uint32_t moveCount)
	{
		for (auto &snapshot : xt.imageSnapshots.buffer) {
			if (snapshot.moveCount >= moveCount)
				snapshot.invalidate();
		}
	}

	void setViewAngles(float pitch, float yaw);
	void setViewAnglesClamped(float pitch, float yaw);
	
	void updateItem(const PlayerMove &move);
	void updateWeapon(const PlayerMove &move);
	void serverUpdateMove(const PlayerMove *moves, int moveCount);
	void clientMove(uint32_t curTime);

	void ghostSetMove(
		PlayerMove *move, const Point3F &newPos, const Point3F &newVel,
		bool newContact, float newRot, float newPitch, int skipCount, bool noInterp);

	void initProjectileXT(Projectile *projectile);
	void initPredictedProjectile(Projectile *projectile, int type);
	void clientFireImageProjectile(int imageSlot);
};