#pragma once

#include "util/meta.h"
#include "util/struct.h"

class SimConsolePlugin;

class SimGame {
public:
	static SimGame *get()
	{
		return *(SimGame**)0x7510D8;
	}

	FIELD(0x1F0, double, frameStart);

	void registerPlugin(SimConsolePlugin *pl)
	{
		using func_t = to_static_function_t<decltype(&SimGame::registerPlugin)>;
		((func_t)0x527FD0)(this, pl);
	}

	double getTime() const
	{
		return frameStart;
	}

	uint32_t getTimeMs() const
	{
		return (uint32_t)(getTime() * 1000);
	}
};
