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

auto &cg = *(WorldGlobals*)0x6D4FA8;
auto &sg = *(WorldGlobals*)0x6D5020;
auto *&wg = *(WorldGlobals**)0x747028;
