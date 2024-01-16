#pragma once

#include "darkstar/console/console.h"
#include "darkstar/Sim/simConsolePlugin.h"
#include <Windows.h>

class SimWinConsolePlugin : public SimConsolePlugin, public CMDConsumer {
public:
	static constexpr size_t MAX_CMDS = 10;

	bool winConsoleEnabled;

	HANDLE stdOut;
	HANDLE stdIn;
	HANDLE stdErr;
	char inbuf[512];
	int  inpos;
	bool lineOutput;
	char curTabComplete[512];
	int  tabCompleteStart;
	char rgCmds[MAX_CMDS][512];
	int  iCmdIndex;
};
