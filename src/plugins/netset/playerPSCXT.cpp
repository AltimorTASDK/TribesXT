#include "darkstar/Core/bitstream.h"
#include "tribes/constants.h"
#include "tribes/worldGlobals.h"
#include "plugins/netset/playerPSCXT.h"
#include "plugins/netset/playerXT.h"
#include <algorithm>

bool PlayerPSCXT::wasTriggerPressedSubtick() const
{
	// Check if the player clicked between frames, even if they released after
	if (triggerCount == xt.prevFrameTriggerCount)
		return false;
	if (triggerCount == ((xt.prevFrameTriggerCount + 1) & ~1))
		return false;
	return true;
}

bool PlayerPSCXT::isTriggerReleased() const
{
	return !(triggerCount & 1);
}

void PlayerPSCXT::collectSubtickInput(uint32_t startTime, uint32_t endTime)
{
	// Matches clientCollectInput move timing
	const auto startTick = msToTicks(startTime - 1);
	const auto endTick = msToTicks(endTime - 1);
	const auto subtick = (uint8_t)(endTime % TickMs);

	if (endTick != startTick) {
		xt.lockedInForTick = false;
		for (auto tick = startTick + 1; tick <= endTick; tick++)
			xt.subtickRecords[tick % SubtickHistory].subtick = NoSubtick;
	}

	if (wasTriggerPressedSubtick())
		xt.heldTriggerSubtick = subtick;
	else if (isTriggerReleased())
		xt.heldTriggerSubtick = NoSubtick;

	// Check if the player clicked for the first time this tick or held
	// the trigger for a full tick
	if (!xt.lockedInForTick && subtick >= xt.heldTriggerSubtick) {
		xt.lockedInForTick = true;
		for (auto tick = std::max(startTick + 1, endTick); tick <= endTick; tick++) {
			xt.subtickRecords[tick % SubtickHistory] = {
				.subtick = subtick,
				.pitch = curMove.pitch,
				.yaw = curMove.turnRot
			};
		}
	}

	xt.prevFrameTriggerCount = triggerCount;
}

void PlayerPSCXT::writeSubtick(BitStream *stream, int moveIndex)
{
	// Reverse of the index calc in getClientMove
	const auto movesBack = moves.size() - moveIndex - 1;
	const auto moveTick = msToTicks(cg.currentTime - 1) - movesBack;
	const auto &record = xt.subtickRecords[moveTick % SubtickHistory];

	if (stream->writeFlag(record.subtick != NoSubtick)) {
		stream->writeInt(record.subtick, TickShift);
		if (stream->writeFlag(record.pitch != 0))
			stream->write(record.pitch);
		if (stream->writeFlag(record.yaw != 0))
			stream->write(record.yaw);
	}
}

void PlayerPSCXT::readSubtick(BitStream *stream, int skipCount)
{
	SubtickRecord record;

	if (stream->readFlag()) {
		record.subtick = stream->readInt(TickShift);
		if (stream->readFlag())
			stream->read(&record.pitch);
		if (stream->readFlag())
			stream->read(&record.yaw);
	} else {
		record.subtick = NoSubtick;
	}

	if (skipCount != 0 || moves.size() >= SubtickHistory || controlPlayer == nullptr)
		return;

	// Pass to the player for serverUpdateMove
	auto *player = (PlayerXT*)controlPlayer;
	player->xt.subtickRecords[moves.size()] = record;
}