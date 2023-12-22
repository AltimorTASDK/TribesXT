#include "tribes/playerPSC.h"
#include "plugins/netset/playerXT.h"
#include "plugins/netset/netset.h"
#include "util/tribes/console.h"

PlayerXT *NetSetPlugin::hook_Player_ctor(PlayerXT *player)
{
	// Initialize new fields
	new (&player->xt) PlayerXT::DataXT;
	return get()->hooks.Player.ctor.callOriginal(player);
}

void NetSetPlugin::hook_Player_serverUpdateMove(PlayerXT *player, edx_t, PlayerMove *moves, int moveCount)
{
	player->serverUpdateMove(moves, moveCount);
}

void NetSetPlugin::hook_Player_updateMove(PlayerXT *player, edx_t, PlayerMove *curMove, bool server)
{
	get()->hooks.Player.updateMove.callOriginal(player, curMove, server);

	// Update item images after moving instead
	for (auto i = 0; i < Player::MaxItemImages; i++)
		player->updateImageState(i, 0.032f);

	player->lastProcessTime += TickMs;
	player->saveSnapshot(player->lastProcessTime);
}

__declspec(naked) void NetSetPlugin::hook_Player_updateMove_noImages()
{
	__asm
	{
		// Don't run updateImageState before moving
		mov eax, 0x4BA66C
		jmp eax
	}
}

 void NetSetPlugin::hook_Player_clientProcess_move(PlayerXT *player, uint32_t curTime)
 {
	 player->clientMove(curTime);
 }

__declspec(naked) void NetSetPlugin::hook_Player_clientProcess_move_asm()
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

uint32_t NetSetPlugin::hook_Player_packUpdate(PlayerXT *player, edx_t, Net::GhostManager *gm, uint32_t mask, BitStream *stream)
{
	const auto snapshot = player->createSnapshot();

	player->loadSnapshot(player->lastProcessTime - TickMs);
	const auto result = get()->hooks.Player.packUpdate.callOriginal(player, gm, mask, stream);
	player->loadSnapshot(snapshot);

	return result;
}

void NetSetPlugin::hook_PlayerPSC_readPacket_setTime(CpuState &cs)
{
	auto *player = (PlayerXT*)cs.reg.eax;
	player->invalidatePrediction(player->lastProcessTime);
	player->saveSnapshot(player->lastProcessTime);
}

bool NetSetPlugin::hook_PlayerPSC_writePacket(PlayerPSC *psc, edx_t, BitStream *bstream, uint32_t &key)
{
	if (!psc->isServer || psc->controlPlayer == nullptr)
		return get()->hooks.PlayerPSC.writePacket.callOriginal(psc, bstream, key);

	auto *player = (PlayerXT*)psc->controlPlayer;
	const auto snapshot = player->createSnapshot();

	player->loadSnapshot(player->lastProcessTime - TickMs);
	const auto result = get()->hooks.PlayerPSC.writePacket.callOriginal(psc, bstream, key);
	player->loadSnapshot(snapshot);

	return result;
}

static PlayerXT::Snapshot testSnapshot;

static void c_remoteSaveSnapshot(PlayerXT *client)
{
	Console->printf(CON_GREEN, "remoteSaveSnapshot called");
	testSnapshot = client->createSnapshot();
}

static void c_remoteLoadSnapshot(PlayerXT *client)
{
	Console->printf(CON_GREEN, "remoteLoadSnapshot called");
	client->loadSnapshot(testSnapshot);
}

void NetSetPlugin::init()
{
	addCommandXT<"remoteSaveSnapshot", c_remoteSaveSnapshot>(console);
	addCommandXT<"remoteLoadSnapshot", c_remoteLoadSnapshot>(console);
}