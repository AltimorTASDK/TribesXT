#include "tribes/fearPlugin.h"
#include "tribes/playerPSC.h"
#include "tribes/worldGlobals.h"
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

void ScriptXTPlugin::init()
{
	console->addVariable(0, "$xt::speed",  CMDConsole::Float, &cvars::xt::speed);
	console->addVariable(0, "$xt::health", CMDConsole::Float, &cvars::xt::health);
	console->addVariable(0, "$xt::energy", CMDConsole::Float, &cvars::xt::energy);
}