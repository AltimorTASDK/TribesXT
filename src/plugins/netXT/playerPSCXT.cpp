#include "darkstar/Core/bitstream.h"
#include "tribes/constants.h"
#include "tribes/fear.strings.h"
#include "tribes/worldGlobals.h"
#include "plugins/netXT/netXT.h"
#include "plugins/netXT/playerPSCXT.h"
#include "plugins/netXT/playerXT.h"
#include "plugins/netXT/version.h"
#include <algorithm>
#include <numeric>

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
	if (xt.pendingSubtickRecord.subtick == NoSubtick && xt.heldTriggerSubtick != NoSubtick) {
		if (subtick >= xt.heldTriggerSubtick || endTick != startTick) {
			xt.pendingSubtickRecord = {
				.subtick = xt.heldTriggerSubtick,
				.pitch = curMove.pitch,
				.yaw = curMove.turnRot
			};
		}
	}

	if (endTick != startTick) {
		for (auto tick = startTick + 1; tick <= endTick; tick++)
			xt.subtickRecords[tick % MaxMovesXT] = xt.pendingSubtickRecord;

		xt.pendingSubtickRecord.subtick = NoSubtick;
	}
}

auto PlayerPSCXT::getSubtick(uint32_t time) const -> const SubtickRecord&
{
	return xt.subtickRecords[msToTicks(time) % MaxMovesXT];
}

void PlayerPSCXT::writeSubtick(BitStream *stream, int moveIndex)
{
	if (!Netcode::XT::Subtick.check())
		return;

	// Reverse of the index calc in getClientMove
	const auto movesBack = moves.size() - moveIndex - 1;
	const auto moveTick = msToTicks(cg.currentTime - 1) - movesBack;
	const auto &record = xt.subtickRecords[moveTick % MaxMovesXT];

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

	// Pass to the player for serverUpdateMove
	if (auto *player = getPlayerXT(); player != nullptr)
		player->xt.subtickRecords[moves.size()] = record;
}

void PlayerPSCXT::writeLagCompensation(BitStream *stream, int moveIndex)
{
	if (!Netcode::XT::LagCompensation.check())
		return;

	// Reverse of the index calc in getClientMove
	const auto movesBack = moves.size() - moveIndex - 1;
	const auto moveTick = msToTicks(cg.currentTime - 1) - movesBack;
	const auto &subtickRecord = xt.subtickRecords[moveTick % MaxMovesXT];

	const auto syncedTimeBase = xt.syncedClock - cvars::net::timeNudge;
	const auto syncedTimeMove = syncedTimeBase - movesBack * TickMs;

	uint32_t lagCompensationTime;

	// Adjust by subtick if possible
	if (subtickRecord.subtick != NoSubtick)
		lagCompensationTime = syncedTimeMove - TickMs + subtickRecord.subtick;
	else
		lagCompensationTime = syncedTimeMove;

	stream->writeInt(lagCompensationTime, 32);
}

void PlayerPSCXT::readLagCompensation(BitStream *stream)
{
	const auto lagCompensationTime = stream->readInt(32);
	// Pass to the player for serverUpdateMove
	if (auto *player = getPlayerXT(); player != nullptr)
		player->xt.lagCompensationRequests[moves.size()].time = lagCompensationTime;
}

void PlayerPSCXT::clientUpdateClock(uint32_t startTime, uint32_t endTime)
{
	if (!Netcode::XT::ClockSync.check())
		return;

	xt.syncedClock += endTime - startTime;
	xt.clockHistoryIndex = (xt.clockHistoryIndex + 1) % ClockHistory;

	const auto errorSum = std::accumulate(
		&xt.clockErrorHistory[0],
		&xt.clockErrorHistory[ClockHistory],
		0);

	const auto clockCorrection = clamp(cvars::net::clientClockCorrection, 0, 64);

	if ((uint32_t)std::abs(errorSum) <= clockCorrection * ClockHistory)
		return;

	// Must be signed division
	const auto error = errorSum / (int)ClockHistory;

	// Adjust clock based on average error
	xt.syncedClock += error;

	for (size_t i = 0; i < ClockHistory; i++)
		xt.clockErrorHistory[i] -= error;

	Console->printf(CON_YELLOW, "Clock correction: %d ms", error);
}

void PlayerPSCXT::writeClockSync(BitStream *stream)
{
	stream->writeInt(msToTicks(sg.currentTime), ClockTickBits);
}

void PlayerPSCXT::readClockSync(BitStream *stream)
{
	if (!Netcode::XT::ClockSync.check())
		return;

	xt.serverClock = ticksToMs(stream->readInt(ClockTickBits));
	xt.clockErrorHistory[xt.clockHistoryIndex] = xt.serverClock - xt.syncedClock;
}