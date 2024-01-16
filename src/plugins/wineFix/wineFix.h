#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"
#include "nofix/x86Hook.h"

class WineFixPlugin : public SimConsolePlugin {
	static void hook_SimWinConsolePlugin_consoleCallback_allocConsole(CpuState &cs);

	struct {
		struct {
			// Attach dedi console to parent process if possible
			// Force ensure_tty_input_thread call on Wine
			x86Hook consoleCallback_allocConsole = {hook_SimWinConsolePlugin_consoleCallback_allocConsole, 0x561278, 1,
			                                        x86Hook::NOINSTRUCTION};
		} SimWinConsolePlugin;
	} hooks;
};
