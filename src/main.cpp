#include "darkstar/console/console.h"
#include "darkstar/Sim/simGame.h"
#include "plugins/netset/netset.h"
#include <Windows.h>

#define REGISTER_PLUGIN(type) [] { \
	SimGame::get()->registerPlugin(new (type)); \
	Console->printf(CON_YELLOW, "TribesXT: Loaded " #type); \
}()

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved)
{
	if (reason != DLL_PROCESS_ATTACH)
		return FALSE;

	REGISTER_PLUGIN(NetSetPlugin);

	return TRUE;
}
