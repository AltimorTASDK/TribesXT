#include "plugins/netset/playerXT.h"
#include "plugins/netset/netset.h"
#include "util/tribes/console.h"

PlayerXT *NetSetPlugin::hook_Player_ctor(PlayerXT *player)
{
	// Initialize new fields
	new (&player->xt) PlayerXT::DataXT;
	return get()->hooks.Player.ctor.callOriginal(player);
}

void NetSetPlugin::hook_Player_updateMove(PlayerXT *player, edx_t, PlayerMove *curMove, bool server)
{
	get()->hooks.Player.updateMove.callOriginal(player, curMove, server);
	player->lastProcessTime += TICK_MS;
	player->saveSnapshot(player->lastProcessTime);
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

void NetSetPlugin::hook_PlayerPSC_readPacket_setTime(CpuState &cs)
{
	auto *player = (PlayerXT*)cs.reg.eax;
	player->invalidatePrediction(player->lastProcessTime);
	player->saveSnapshot(player->lastProcessTime);
}

static PlayerXT::Snapshot testSnapshot;

static void c_remoteSaveSnapshot(PlayerXT *client)
{
	Console->printf(CON_GREEN, "remoteSaveSnapshot called");
	testSnapshot = client->createSnapshot(0);
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