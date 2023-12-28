#pragma once

#include "util/struct.h"

class Player;
class PlayerPSC;
class SimManager;

class WorldGlobals {
public:
	FIELD(0x00, uint32_t, lastTime);
	FIELD(0x04, uint32_t, currentTime);
	FIELD(0x08, uint32_t, timeBase);
	FIELD(0x14, SimManager*, manager);
	FIELD(0x1C, PlayerPSC*, psc);
	FIELD(0x30, Player*, player);
};

inline auto &cg = *(WorldGlobals*)0x6D4FA8;
inline auto &sg = *(WorldGlobals*)0x6D5020;
inline auto *&wg = *(WorldGlobals**)0x747028;
