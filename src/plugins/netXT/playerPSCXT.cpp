#include "darkstar/Core/bitstream.h"
#include "tribes/constants.h"
#include "tribes/fear.strings.h"
#include "tribes/worldGlobals.h"
#include "plugins/netXT/playerPSCXT.h"
#include "plugins/netXT/playerXT.h"
#include "plugins/netXT/version.h"

void PlayerPSCXT::preSimActionEvent(int action, float eventValue)
{
	// currentTime isn't updated yet, so it corresponds to startTime below
	const auto subtick = (uint8_t)(cg.currentTime % TickMs);

	switch (action) {
	case IDACTION_FIRE1:
		if (xt.pendingSubtickRecord.subtick == NoSubtick) {
			xt.pendingSubtickRecord = {
				.subtick = subtick,
				.pitch = curMove.pitch,
				.yaw = curMove.turnRot
			};
		}
		xt.heldTriggerSubtick = subtick;
		break;

	case IDACTION_BREAK1:
		// This counts as shooting if you weren't holding the trigger
		if (!(triggerCount & 1)) {
			if (xt.pendingSubtickRecord.subtick == NoSubtick) {
				xt.pendingSubtickRecord = {
					.subtick = subtick,
					.pitch = curMove.pitch,
					.yaw = curMove.turnRot
				};
			}
		}
		xt.heldTriggerSubtick = NoSubtick;
		break;
	}
}

void PlayerPSCXT::collectSubtickInput(uint32_t startTime, uint32_t endTime)
{
	// Matches clientCollectInput move timing
	const auto startTick = msToTicks(startTime - 1);
	const auto endTick = msToTicks(endTime - 1);
	const auto subtick = (uint8_t)(startTime % TickMs);

	// Preserve the subtick offset if the player holds the trigger across ticks
	if (xt.pendingSubtickRecord.subtick == NoSubtick) {
		if (xt.heldTriggerSubtick != NoSubtick) {
			if (subtick >= xt.heldTriggerSubtick || endTick != startTick) {
				xt.pendingSubtickRecord = {
					.subtick = xt.heldTriggerSubtick,
					.pitch = curMove.pitch,
					.yaw = curMove.turnRot
				};
			}
		}
	}

	if (endTick != startTick) {
		for (auto tick = startTick + 1; tick <= endTick; tick++)
			xt.subtickRecords[tick % SubtickHistory] = xt.pendingSubtickRecord;

		xt.pendingSubtickRecord.subtick = NoSubtick;
	}

	xt.prevFrameTriggerCount = triggerCount;
}

void PlayerPSCXT::writeSubtick(BitStream *stream, int moveIndex)
{
	if (serverNetcodeVersion < Netcode::XT::Subtick)
		return;

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

void PlayerPSCXT::readSubtick(BitStream *stream)
{
	auto record = SubtickRecord();

	if (stream->readFlag()) {
		record.subtick = stream->readInt(TickShift);
		if (stream->readFlag())
			stream->read(&record.pitch);
		if (stream->readFlag())
			stream->read(&record.yaw);
	}

	if (moves.size() >= SubtickHistory || controlPlayer == nullptr)
		return;

	// Pass to the player for serverUpdateMove
	auto *player = (PlayerXT*)controlPlayer;
	player->xt.subtickRecords[moves.size()] = record;
}