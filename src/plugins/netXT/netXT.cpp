#include "darkstar/Core/bitstream.h"
#include "tribes/playerPSC.h"
#include "tribes/worldGlobals.h"
#include "plugins/netXT/netXT.h"
#include "plugins/netXT/playerXT.h"
#include "plugins/netXT/playerPSCXT.h"
#include "util/math.h"
#include "util/tribes/console.h"
#include <cmath>

PlayerXT *NetXTPlugin::hook_Player_ctor(PlayerXT *player)
{
	// Initialize new fields
	new (&player->xt) PlayerXT::DataXT;
	return get()->hooks.Player.ctor.callOriginal(player);
}

void NetXTPlugin::hook_Player_serverUpdateMove(
	PlayerXT *player, edx_t, PlayerMove *moves, int moveCount)
{
	player->serverUpdateMove(moves, moveCount);
}

void NetXTPlugin::hook_Player_ghostSetMove(
	PlayerXT *player, edx_t, PlayerMove *move, const Point3F &newPos, const Point3F &newVel,
	bool newContact, float newRot, float newPitch, int skipCount, bool noInterp)
{
	player->ghostSetMove(move, newPos, newVel, newContact, newRot,
	                     newPitch, skipCount, noInterp, cvars::net::timeNudge);
}

void NetXTPlugin::hook_Player_updateMove(PlayerXT *player, edx_t, PlayerMove *curMove, bool server)
{
	get()->hooks.Player.updateMove.callOriginal(player, curMove, server);

	if (!player->xt.applySubtick)
		player->updateWeapon(*curMove);

	player->lastProcessTime += TickMs;
	player->saveSnapshot(player->lastProcessTime);
}

__declspec(naked) void NetXTPlugin::hook_Player_updateMove_noImages()
{
	__asm
	{
		// Don't run updateImageState before moving
		mov eax, 0x4BA66C
		jmp eax
	}
}

 void NetXTPlugin::hook_Player_clientProcess_move(PlayerXT *player, uint32_t curTime)
 {
	 player->clientMove(curTime);
 }

__declspec(naked) void NetXTPlugin::hook_Player_clientProcess_move_asm()
{
	__asm
	{
		push [esp+0x34]
		push ebx
		call hook_Player_clientProcess_move
		add esp, 8
		// Jump past move/interp
		fldz
		mov eax, 0x4BC409
		jmp eax
	}
}

uint32_t NetXTPlugin::hook_Player_packUpdate(
	PlayerXT *player, edx_t, Net::GhostManager *gm, uint32_t mask, BitStream *stream)
{
	const auto snapshot = player->createSnapshot();

	// Clients expect the state before the most recent move
	player->loadSnapshot(player->lastProcessTime - TickMs);
	const auto result = get()->hooks.Player.packUpdate.callOriginal(player, gm, mask, stream);
	player->loadSnapshot(snapshot);

	return result;
}

PlayerPSCXT *NetXTPlugin::hook_PlayerPSC_ctor(PlayerPSCXT *psc, edx_t, bool in_isServer)
{
	// Initialize new fields
	new (&psc->xt) PlayerPSCXT::DataXT;
	return get()->hooks.PlayerPSC.ctor.callOriginal(psc, in_isServer);
}

bool NetXTPlugin::hook_PlayerPSC_writePacket(
	PlayerPSCXT *psc, edx_t, BitStream *bstream, uint32_t &key)
{
	if (!psc->isServer || psc->controlPlayer == nullptr)
		return get()->hooks.PlayerPSC.writePacket.callOriginal(psc, bstream, key);

	auto *player = (PlayerXT*)psc->controlPlayer;
	const auto snapshot = player->createSnapshot();

	// Clients expect the state before the most recent move
	player->loadSnapshot(player->lastProcessTime - TickMs);
	const auto result = get()->hooks.PlayerPSC.writePacket.callOriginal(psc, bstream, key);
	player->loadSnapshot(snapshot);

	return result;
}

void NetXTPlugin::hook_PlayerPSC_clientCollectInput(
	PlayerPSCXT *psc, edx_t, uint32_t startTime, uint32_t endTime)
{
	psc->collectSubtickInput(startTime, endTime);
	get()->hooks.PlayerPSC.clientCollectInput.callOriginal(psc, startTime, endTime);
}

void NetXTPlugin::hook_clampAngleDelta(float *angle, float range)
{
	// Check subnormal/infinite/NaN
	if (std::isnormal(*angle))
		*angle = normalize_radians_signed(*angle);
	else
		*angle = 0.f;
}

void NetXTPlugin::hook_PlayerPSC_readPacket_setTime(CpuState &cs)
{
	const auto *psc = (PlayerPSC*)cs.reg.ebx;
	if (auto *player = (PlayerXT*)psc->controlPlayer; player != nullptr) {
		player->invalidatePrediction(player->lastProcessTime);
		player->saveSnapshot(player->lastProcessTime);
	}
}

void NetXTPlugin::hook_PlayerPSC_readPacket_move(CpuState &cs)
{
	auto *psc = (PlayerPSCXT*)cs.reg.ebx;
	auto *stream = (BitStream*)cs.reg.ebp;
	psc->readSubtick(stream);
}

void NetXTPlugin::hook_PlayerPSC_writePacket_move(CpuState &cs)
{
	auto *psc = (PlayerPSCXT*)cs.reg.ebp;
	auto *stream = (BitStream*)cs.reg.edi;
	const auto moveIndex = (int)cs.reg.eax;
	psc->writeSubtick(stream, moveIndex);
}

void NetXTPlugin::hook_PlayerPSC_onSimActionEvent(CpuState &cs)
{
	auto *psc = (PlayerPSCXT*)cs.reg.ecx;
	const auto action = (int)cs.reg.eax;
	const auto eventValue = *(float*)(cs.reg.ebx + 0x28);
	psc->preSimActionEvent(action, eventValue);
}

bool NetXTPlugin::hook_PacketStream_checkPacketSend_check()
{
	// Check if server ticked or client produced a move
	if (wg == &sg)
		return sg.currentTime != sg.lastTime;
	else
		return msToTicks(cg.currentTime - 1) != msToTicks(cg.lastTime - 1);
}

__declspec(naked) void NetXTPlugin::hook_PacketStream_checkPacketSend_check_asm()
{
	__asm {
		call hook_PacketStream_checkPacketSend_check
		test al, al
		je skip
		mov eax, 0x51A362
		jmp eax
	skip:
		mov eax, 0x51A574
		jmp eax
	}
}

void NetXTPlugin::init()
{
	console->addVariable(0, "net::timeNudge", CMDConsole::Int, &cvars::net::timeNudge);
}