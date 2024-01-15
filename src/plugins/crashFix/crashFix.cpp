#include "plugins/crashFix/crashFix.h"

void CrashFixPlugin::hook_SimWinConsolePlugin_endFrame_fixCrash(CpuState &cs)
{
	// Take the jbe if ReadConsoleInputA failed
	if (cs.reg.eax == 0)
		cs.eflag.cf = 1;
}