#pragma once

#include "tribes/constants.h"
#include "tribes/playerPSC.h"
#include "util/struct.h"
#include <bit>

class BitStream;

class PlayerPSCXT : public PlayerPSC {
public:
	// More efficient modulo
	static constexpr auto MaxMovesXT = std::bit_ceil(MaxMoves);

	static constexpr uint8_t NoSubtick = -1;

	static constexpr auto ClockTickBits = 32 - TickShift;
	static constexpr auto ClockHistory = std::bit_ceil((size_t)(1.0 * TickRate));

	struct SubtickRecord {
		uint8_t subtick = NoSubtick;
		float pitch;
		float yaw;
	};

	struct LagCompensationRequest {
		uint32_t time = -1;
	};

	struct DataXT {
		// Client only
		SubtickRecord subtickRecords[MaxMovesXT];
		SubtickRecord pendingSubtickRecord;
		int prevFrameTriggerCount = 0;
		uint8_t heldTriggerSubtick = NoSubtick;

		// Client only
		uint32_t serverClock = 0;
		uint32_t syncedClock = 0;
		uint32_t clockHistoryIndex = 0;
		int clockErrorHistory[ClockHistory] = {};
	};

	FIELD(PlayerPSC::SIZEOF, DataXT, xt);

	static constexpr size_t SIZEOF = PlayerPSC::SIZEOF + sizeof(DataXT);

public:
	void preSimActionEvent(int action, float eventValue);
	void collectSubtickInput(uint32_t startTime, uint32_t endTime);
	const SubtickRecord &getSubtick(uint32_t time);
	void writeSubtick(BitStream *stream, int moveIndex);
	void readSubtick(BitStream *stream);

	void writeLagCompensation(BitStream *stream, int moveIndex);
	void readLagCompensation(BitStream *stream);

	void clientUpdateClock(uint32_t startTime, uint32_t endTime);
	void writeClockSync(BitStream *stream);
	void readClockSync(BitStream *stream);
};
