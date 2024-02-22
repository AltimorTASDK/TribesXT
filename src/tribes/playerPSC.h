#pragma once

#include "darkstar/Core/tVector.h"
#include "tribes/gameBase.h"
#include "tribes/player.h"
#include "util/struct.h"

class PlayerPSC {
public:
	static constexpr size_t MaxMoves = 30;

	static constexpr size_t SIZEOF = 0x6C8;

	FIELD(0x58, Vector<PlayerMove>, moves);
	FIELD(0x6C, Vector<PlayerMove>, pmvec);
	FIELD(0x80, PlayerMove, curMove);
	FIELD(0xA1, bool, isServer);
	FIELD(0xA4, int, triggerCount);
	FIELD(0xA8, int, prevTriggerCount);
	FIELD(0xAC, float, camDist);
	FIELD(0xB4, GameBase*, controlObject);
	FIELD(0xB8, Player*, controlPlayer);
	FIELD(0x3DC, int, lastPlayerMove);
	FIELD(0x3E0, int, firstMoveSeq);
	FIELD(0x3E8, int, curGuiMode);
	FIELD(0x3EE, bool, fInObserverMode);
	FIELD(0x3F0, float, damageFlash);

	PlayerMove *getClientMove(unsigned int time)
	{
		using func_t = to_static_function_t<decltype(&PlayerPSC::getClientMove)>;
		return ((func_t)0x482DF0)(this, time);
	}
};
