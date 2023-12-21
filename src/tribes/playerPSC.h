#pragma once

#include "tribes/player.h"
#include "util/struct.h"

class PlayerPSC {
public:
	FIELD(0x80, PlayerMove, curMove);
	FIELD(0xA1, bool, isServer);

	PlayerMove *getClientMove(unsigned int time)
	{
		using func_t = to_static_function_t<decltype(&PlayerPSC::getClientMove)>;
		return ((func_t)0x482DF0)(this, time);
	}
};
