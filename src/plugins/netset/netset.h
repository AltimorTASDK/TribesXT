#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "plugins/netset/playerXT.h"
#include "util/hooks.h"
#include "nofix/x86Hook.h"

namespace Net {
class GhostManager;
}

class BitStream;
class Player;
struct PlayerMove;
class PlayerPSC;

class NetSetPlugin : public SimConsolePlugin {
	static inline NetSetPlugin *instance;

public:
	static NetSetPlugin *get()
	{
		return instance;
	}

private:
	static PlayerXT *__fastcall hook_Player_ctor(PlayerXT*);
	static void __fastcall hook_Player_serverUpdateMove(PlayerXT*, edx_t, PlayerMove *moves, int moveCount);
	static void __fastcall hook_Player_updateMove(PlayerXT*, edx_t, PlayerMove *curMove, bool server);
	static void hook_Player_updateMove_noImages();
	static void hook_Player_clientProcess_move(PlayerXT*, uint32_t curTime);
	static void hook_Player_clientProcess_move_asm();
	static uint32_t __fastcall hook_Player_packUpdate(PlayerXT*, edx_t, Net::GhostManager *gm, uint32_t mask, BitStream *stream);
	static void __x86Hook hook_PlayerPSC_readPacket_setTime(CpuState &cs);
	static bool __fastcall hook_PlayerPSC_writePacket(PlayerPSC*, edx_t, BitStream *bstream, uint32_t &key);

	struct {
		struct {
			StaticCodePatch<0x42F7D9, PlayerXT::SIZEOF> allocationSize1;
			StaticCodePatch<0x4AE8D7, PlayerXT::SIZEOF> allocationSize2;
			StaticCodePatch<0x4CE4B8, PlayerXT::SIZEOF> allocationSize3;
			StaticCodePatch<0x4CFDA2, PlayerXT::SIZEOF> allocationSize4;
			StaticJmpHook<0x4ACE70, hook_Player_ctor> ctor;
			StaticJmpHook<0x4BBB40, hook_Player_serverUpdateMove> serverUpdateMove;
			StaticJmpHook<0x4BA640, hook_Player_updateMove> updateMove;
			StaticJmpHook<0x4BA653, hook_Player_updateMove_noImages> updateMove_noImages;
			StaticJmpHook<0x4BC2B3, hook_Player_clientProcess_move_asm> clientProcess_move;
			//
			// handle ghost update masks
			//
			StaticJmpHook<0x4BB760, hook_Player_packUpdate> packUpdate;
		} Player;
		struct {
			StaticJmpHook<0x482E30, hook_PlayerPSC_writePacket> writePacket;
			x86Hook readPacket_setTime = {hook_PlayerPSC_readPacket_setTime, 0x485945, 1};
		} PlayerPSC;
	} hooks;

public:
	NetSetPlugin()
	{
		instance = this;
	}

	~NetSetPlugin()
	{
		instance = nullptr;
	}

	void init() override;
};
