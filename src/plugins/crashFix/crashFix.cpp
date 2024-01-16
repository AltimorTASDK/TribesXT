#include "darkstar/SimObjects/simWinConsolePlugin.h"
#include "plugins/crashFix/crashFix.h"
#include <algorithm>
#include <cstdarg>

static char scratchBuffer[1024];

void CrashFixPlugin::hook_SimWinConsolePlugin_endFrame_fixCrash(CpuState &cs)
{
	// Take the jbe if ReadConsoleInputA failed
	if (cs.reg.eax == 0)
		cs.eflag.cf = 1;
}

void CrashFixPlugin::hook_SimWinConsolePlugin_printf(SimWinConsolePlugin *console, const char *s, ...)
{
	// Fix buffer overflow
	va_list args;
	va_start(args, s);
	const auto length = vsnprintf(scratchBuffer, sizeof(scratchBuffer), s, args);

	if (length < 0)
		return;

	const auto written = std::min((size_t)length, sizeof(scratchBuffer) - 1);

	DWORD bytes;
	WriteFile(console->stdOut, scratchBuffer, written, &bytes, nullptr);
	FlushFileBuffers(console->stdOut);
}

static const char *concatArguments(int startIndex, int argc, const char **argv)
{
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