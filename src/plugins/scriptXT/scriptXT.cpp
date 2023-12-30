#include "darkstar/Sim/simEv.h"
#include "tribes/fearPlugin.h"
#include "tribes/playerPSC.h"
#include "tribes/worldGlobals.h"
#include "plugins/netXT/version.h"
#include "plugins/scriptXT/scriptXT.h"

void ScriptXTPlugin::hook_FearPlugin_endFrame(FearPlugin *plugin)
{
	get()->hooks.FearPlugin.endFrame.callOriginal(plugin);

	if (cg.psc == nullptr || cg.psc->controlObject == nullptr)
		return;

	const auto *player = cg.psc->controlObject;

	cvars::xt::speed  = player->getLinearVelocity().length();
	cvars::xt::health = (1 - player->getDamageLevel()) * 100;
	cvars::xt::energy = (1 - player->getEnergyLevel()) * 100;
}

bool ScriptXTPlugin::hook_PlayerPSC_processQuery(PlayerPSC *psc, edx_t, SimQuery *query)
{
	if (!get()->hooks.PlayerPSC.processQuery.callOriginal(psc, query))
		return false;

	// Default function only returns true for SimCameraQueryType
	auto *qp = (SimCameraQuery*)query;

	// Only on XT servers
	if (serverNetcodeVersion >= Netcode::XT::PrefDamageFlash)
		qp->cameraInfo.alphaBlend *= clamp(cvars::pref::damageFlash, 0.f, 1.f);

	return true;
}

void ScriptXTPlugin::init()
{
	console->addVariable(0, "$xt::speed",  CMDConsole::Float, &cvars::xt::speed);
	console->addVariable(0, "$xt::health", CMDConsole::Float, &cvars::xt::health);
	console->addVariable(0, "$xt::energy", CMDConsole::Float, &cvars::xt::energy);

	console->addVariable(0, "$pref::damageFlash", CMDConsole::Float, &cvars::pref::damageFlash);
}