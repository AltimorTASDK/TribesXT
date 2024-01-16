#include "plugins/wineFix/wineFix.h"
#include <Windows.h>

void WineFixPlugin::hook_SimWinConsolePlugin_consoleCallback_allocConsole(CpuState &cs)
{
	// Try using the parent console instead of spawning a window.
	if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
		// This calls ensure_tty_input_thread on Wine, which doesn't happen
		// automatically after AttachConsole/AllocConsole. Normally reading
		// from stdin would call it, but because T1 checks for unread input
		// with GetNumberOfConsoleInputEvents before reading stdin, that
		// never happens.
		ReadFile(GetStdHandle(STD_INPUT_HANDLE), nullptr, 0, nullptr, nullptr);
	}
}