#include "plugins/netset/playerXT.h"
#include "plugins/netset/netset.h"
#include "util/tribes/console.h"

void NetSetPlugin::hook_Player_updateMove(PlayerXT *player, edx_t, PlayerMove *curMove, bool server)
{
	get()->hooks.Player.updateMove.callOriginal(player, curMove, server);
}

static const char *c_remoteSaveSnapshot(PlayerXT *client)
{
	Console->printf(CON_GREEN, "remoteSaveSnapshot called");
	client->saveSnapshot(0);
	return "";
}

static const char *c_remoteLoadSnapshot(PlayerXT *client)
{
	Console->printf(CON_GREEN, "remoteLoadSnapshot called");
	client->loadSnapshot(0);
	return "";
}

void NetSetPlugin::init()
{
	addCommandXT<"remoteSaveSnapshot", c_remoteSaveSnapshot>(console);
	addCommandXT<"remoteLoadSnapshot", c_remoteLoadSnapshot>(console);
}