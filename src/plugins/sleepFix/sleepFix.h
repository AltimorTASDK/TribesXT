#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"

class SleepFixPlugin : public SimConsolePlugin {
	struct {
		struct {
			// Don't Sleep between serverProcess and clientProcess
			// jmp 0x4E9154
			StaticCodePatch<0x4E9146, "\xEB\x0C"> onIdle_Sleep;
		} FearMain;
	} hooks;
};
