#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"

class Player;
struct PlayerMove;

class NetSetPlugin : public SimConsolePlugin {
	static inline NetSetPlugin *instance;

public:
	static NetSetPlugin &get()
	{
		return *instance;
	}

private:
	static void __fastcall hook_Player_updateMove(
		Player *player, edx_t, PlayerMove *curMove, bool server);

	struct {
		StaticJmpHook<0x4BA640, hook_Player_updateMove> Player_updateMove;
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
};
