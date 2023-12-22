#pragma once

#include "util/struct.h"

class PlayerPSC;
class SimManager;

class WorldGlobals {
public:
	FIELD(0x04, unsigned int, currentTime);
	FIELD(0x08, unsigned int, timeBase);
	FIELD(0x14, SimManager*, manager);
	FIELD(0x1C, PlayerPSC*, psc);
};

inline auto &cg = *(WorldGlobals*)0x6D4FA8;
inline auto &sg = *(WorldGlobals*)0x6D5020;
inline auto *&wg = *(WorldGlobals**)0x747028;
