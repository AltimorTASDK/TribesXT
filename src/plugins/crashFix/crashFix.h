#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"
#include "nofix/x86Hook.h"

class CrashFixPlugin : public SimConsolePlugin {
	static void hook_SimWinConsolePlugin_endFrame_fixCrash(CpuState &cs);
	static const char *hook_c_dbecho(CMDConsole *console, int, int argc, const char **argv);

	struct {
		struct {
			// Fix reading stack garbage after not error checking ReadConsoleInputA
			x86Hook endFrame_fixCrash = {hook_SimWinConsolePlugin_endFrame_fixCrash, 0x56136A, 2};
		} SimWinConsolePlugin;
		// Fix arg concat crash in c_dbecho
		StaticJmpHook<0x403DD0, hook_c_dbecho> c_dbecho;
	} hooks;
};
