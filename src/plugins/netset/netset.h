#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "plugins/netset/playerXT.h"
#include "util/hooks.h"

class Player;
struct PlayerMove;

class NetSetPlugin : public SimConsolePlugin {
	static inline NetSetPlugin *instance;

public:
	static NetSetPlugin *get()
	{
		return instance;
	}

private:
	static PlayerXT *__fastcall hook_Player_ctor(PlayerXT *player);
	static void __fastcall hook_Player_updateMove(
		PlayerXT *player, edx_t, PlayerMove *curMove, bool server);

	struct {
		struct {
			StaticCodePatch<0x42F7D9, PlayerXT::SIZEOF> allocationSize1;
			StaticCodePatch<0x4AE8D7, PlayerXT::SIZEOF> allocationSize2;
			StaticCodePatch<0x4CE4B8, PlayerXT::SIZEOF> allocationSize3;
			StaticCodePatch<0x4CFDA2, PlayerXT::SIZEOF> allocationSize4;
			StaticJmpHook<0x4ACE70, hook_Player_ctor> ctor;
			StaticJmpHook<0x4BA640, hook_Player_updateMove> updateMove;
		} Player;
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
