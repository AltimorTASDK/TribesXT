#include "darkstar/console/console.h"
#include "tribes/player.h"
#include "plugins/netset/netset.h"

void NetSetPlugin::hook_Player_updateMove(Player *player, edx_t, PlayerMove *curMove, bool server)
{
	Console->printf("updateMove");
	get().hooks.Player_updateMove.callOriginal(player, curMove, server);
}