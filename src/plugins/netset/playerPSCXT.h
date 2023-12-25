#pragma once

#include "tribes/playerPSC.h"
#include "util/struct.h"

class BitStream;

class PlayerPSCXT : public PlayerPSC {
public:
	static constexpr auto SubtickHistory = std::bit_ceil(MaxMoves);
	static constexpr uint8_t NoSubtick = -1;

	struct SubtickRecord {
		uint8_t subtick = NoSubtick;
		float pitch;
		float yaw;
	};

	struct DataXT {
		// Client only
		SubtickRecord subtickRecords[SubtickHistory];
		SubtickRecord pendingSubtickRecord;
		int prevFrameTriggerCount = 0;
		uint8_t heldTriggerSubtick = NoSubtick;
	};

	FIELD(PlayerPSC::SIZEOF, DataXT, xt);

	static constexpr size_t SIZEOF = PlayerPSC::SIZEOF + sizeof(DataXT);

protected:
	bool wasTriggerPressedSubtick() const;
	bool isTriggerReleased() const;

public:
	void collectSubtickInput(uint32_t startTime, uint32_t endTime);
	void writeSubtick(BitStream *stream, int moveIndex);
	void readSubtick(BitStream *stream);
};
