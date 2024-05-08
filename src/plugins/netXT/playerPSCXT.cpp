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

constexpr uint8_t msToSubtick(uint32_t ms)
{
	// 32 instead of 0, since that's when we finish interpolating
	return (ms - 1) % TickMs + 1;
}

void PlayerPSCXT::preSimActionEvent(int action, float eventValue)
{
	switch (action) {
	case IDACTION_FIRE1:
	case IDACTION_BREAK1:
		// Check for fresh trigger pull, including BREAK1 with trigger released
		if (isTriggerHeld())
			break;

		// currentTime isn't updated yet, so it corresponds to startTime below
		xt.heldTriggerSubtick = msToSubtick(cg.currentTime);

		if (xt.pendingSubtickRecord.subtick == NoSubtick) {
			xt.pendingSubtickRecord = {
				.subtick = xt.heldTriggerSubtick,
				.pitch = curMove.pitch,
				.yaw = curMove.turnRot
			};
		}
	}
}

void PlayerPSCXT::collectSubtickInput(uint32_t startTime, uint32_t endTime)
{
	// Matches clientCollectInput move timing
	const auto startTick = msToTicks(startTime - 1);
	const auto endTick = msToTicks(endTime - 1);

	// Preserve the subtick offset if the player holds the trigger across ticks
	if (isTriggerHeld() && xt.pendingSubtickRecord.subtick == NoSubtick) {
		if (endTick != startTick || msToSubtick(endTime) >= xt.heldTriggerSubtick) {
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
		stream->writeInt(record.subtick - 1, TickShift);
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
		record.subtick = stream->readInt(TickShift) + 1;
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

	// Get the synced clock at the beginning of the tick, adjusted by timenudge
	const auto syncedTickStart = xt.syncedClock - msToSubtick(cg.currentTime - 1);
	auto lagCompensationTime = syncedTickStart - movesBack * TickMs - cvars::net::timeNudge;

	// Adjust by subtick if possible
	if (subtickRecord.subtick != NoSubtick)
		lagCompensationTime += subtickRecord.subtick - TickMs;

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

	const auto historySize = std::min(xt.clockHistoryIndex, ClockHistory);

	const auto errorSum = std::accumulate(
		&xt.clockErrorHistory[0],
		&xt.clockErrorHistory[historySize],
		0);

	const auto clockCorrection = clamp(cvars::net::clientClockCorrection, 0, 64);

	if ((uint32_t)std::abs(errorSum) <= clockCorrection * historySize)
		return;

	// Must be signed division
	const auto error = errorSum / (int)historySize;

	// Adjust clock based on average error
	xt.syncedClock += error;

	for (size_t i = 0; i < historySize; i++)
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
	xt.clockErrorHistory[xt.clockHistoryIndex % ClockHistory] = xt.serverClock - xt.syncedClock;
	xt.clockHistoryIndex++;
}