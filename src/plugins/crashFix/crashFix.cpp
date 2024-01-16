#include "plugins/crashFix/crashFix.h"
#include <cstring>

void CrashFixPlugin::hook_SimWinConsolePlugin_endFrame_fixCrash(CpuState &cs)
{
	// Take the jbe if ReadConsoleInputA failed
	if (cs.reg.eax == 0)
		cs.eflag.cf = 1;
}

static const char *concatArguments(int startIndex, int argc, const char **argv)
{
	static char scratchBuffer[1024];

	auto argIndex = startIndex;
	size_t charIndex = 0;
	size_t totalIndex = 0;

	while (true) {
		if (const auto ch = argv[argIndex][charIndex++]; ch != '\0') {
			scratchBuffer[totalIndex] = ch;
			if (++totalIndex >= sizeof(scratchBuffer) - 1)
				break;
		} else {
			charIndex = 0;
			if (++argIndex >= argc)
				break;
		}
	}

	scratchBuffer[totalIndex] = '\0';
	return scratchBuffer;
}

const char *CrashFixPlugin::hook_c_dbecho(CMDConsole *console, int, int argc, const char **argv)
{
	if (argc >= 2) {
		const auto dblevel = atoi(argv[1]);
		if (argc >= 3)
			console->dbprintf(dblevel, "%s", concatArguments(2, argc, argv));
		else
			console->dbprintf(dblevel, "");
	}

	return "True";
}