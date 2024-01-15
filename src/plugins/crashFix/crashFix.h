#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "nofix/x86Hook.h"

class CrashFixPlugin : public SimConsolePlugin {
	static void __x86Hook hook_SimWinConsolePlugin_endFrame_fixCrash(CpuState &cs);

	struct {
		struct {
			// Fix reading stack garbage after not error checking ReadConsoleInputA
			x86Hook endFrame_fixCrash = {hook_SimWinConsolePlugin_endFrame_fixCrash, 0x56136A, 2};
		} SimWinConsolePlugin;
	} hooks;
};
