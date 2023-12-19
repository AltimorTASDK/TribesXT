#pragma once

#include "util/meta.h"

class SimConsolePlugin;

class SimGame {
public:
	static SimGame *get()
	{
		return *(SimGame**)0x7510D8;
	}

	void registerPlugin(SimConsolePlugin *pl)
	{
		using func_t = to_static_function_t<decltype(&SimGame::registerPlugin)>;
		((func_t)0x527FD0)(this, pl);
	}
};
