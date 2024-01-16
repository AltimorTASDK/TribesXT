#include "darkstar/console/console.h"
#include "darkstar/Sim/simGame.h"
#include "plugins/crashFix/crashFix.h"
#include "plugins/netXT/netXT.h"
#include "plugins/scriptXT/scriptXT.h"
#include "plugins/tracerXT/tracerXT.h"
#include "plugins/skiBugFix/skiBugFix.h"
#include "plugins/wineFix/wineFix.h"
#include <Windows.h>

extern const char *buildString;

#define REGISTER_PLUGIN(type) [] { \
	SimGame::get()->registerPlugin(new (type)); \
	Console->printf(CON_YELLOW, "TribesXT: Loaded " #type); \
}()

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved)
{
	if (reason != DLL_PROCESS_ATTACH)
		return FALSE;

	Console->printf(CON_GREEN, buildString);

	REGISTER_PLUGIN(CrashFixPlugin);
	REGISTER_PLUGIN(NetXTPlugin);
	REGISTER_PLUGIN(ScriptXTPlugin);
	REGISTER_PLUGIN(TracerXTPlugin);
	REGISTER_PLUGIN(SkiBugFixPlugin);
	REGISTER_PLUGIN(WineFixPlugin);

	return TRUE;
}
