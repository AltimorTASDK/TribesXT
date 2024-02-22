#include "darkstar/Sim/simEv.h"
#include "darkstar/Sim/Net/packetStream.h"
#include "tribes/fearPlugin.h"
#include "tribes/playerPSC.h"
#include "tribes/worldGlobals.h"
#include "plugins/netXT/version.h"
#include "plugins/scriptXT/scriptXT.h"

void ScriptXTPlugin::hook_FearPlugin_endFrame(FearPlugin *plugin)
{
	get()->hooks.FearPlugin.endFrame.callOriginal(plugin);

	if (cg.psc != nullptr && cg.psc->controlObject != nullptr) {
		const auto *player = cg.psc->controlObject;

		cvars::xt::speed  = player->getLinearVelocity().length();
		cvars::xt::health = (1 - player->getDamageLevel()) * 100;
		cvars::xt::energy = (1 - player->getEnergyLevel()) * 100;
	}

	if (cg.packetStream != nullptr)
		cvars::xt::ping = cg.packetStream->getAverageRTT();
}

void ScriptXTPlugin::hook_TextFormat_formatControlString_imageWidth(CpuState &cs)
{
	const auto imageWidth = (int)cs.reg.eax;
	auto &width = *(int*)(cs.reg.esp + 0x164);

	// Force images to fit for e.g. resizing HUD bars
	if (imageWidth > width)
		width = imageWidth;
}

void ScriptXTPlugin::hook_Player_getCameraTransform_140Check(CpuState &cs)
{
	cs.eflag.cf = !cvars::pref::newThirdPerson;
	cs.eflag.zf = cs.eflag.cf;
}

void ScriptXTPlugin::hook_Player_getEyeTransform_140Check(CpuState &cs)
{
	// Keep the protocol check here since this affects projectile aim
	if (!cs.eflag.cf && !cs.eflag.zf) {
		cs.eflag.cf = !cvars::pref::newThirdPerson;
		cs.eflag.zf = cs.eflag.cf;
	}
}

void ScriptXTPlugin::hook_PlayerPSC_writePacket_checkThirdPerson(CpuState &cs)
{
	// Make the third person flag sent to the server accurately reflect getCameraTransform
	const auto *psc = (PlayerPSC*)cs.reg.ebp;
	cs.eflag.pf = !cvars::pref::newThirdPerson || psc->camDist < 0.01f;
}

bool ScriptXTPlugin::hook_PlayerPSC_processQuery(PlayerPSC *psc, edx_t, SimQuery *query)
{
	if (!get()->hooks.PlayerPSC.processQuery.callOriginal(psc, query))
		return false;

	// Default function only returns true for SimCameraQueryType
	auto *qp = (SimCameraQuery*)query;

	// Only on XT servers
	if (Netcode::XT::PrefDamageFlash.check())
		qp->cameraInfo.alphaBlend *= clamp(cvars::pref::damageFlash, 0.f, 1.f);

	return true;
}

void ScriptXTPlugin::init()
{
	console->addVariable(0, "$xt::speed",  CMDConsole::Float, &cvars::xt::speed);
	console->addVariable(0, "$xt::health", CMDConsole::Float, &cvars::xt::health);
	console->addVariable(0, "$xt::energy", CMDConsole::Float, &cvars::xt::energy);
	console->addVariable(0, "$xt::ping",   CMDConsole::Int,   &cvars::xt::ping);

	console->addVariable(0, "$pref::damageFlash", CMDConsole::Float, &cvars::pref::damageFlash);

	console->addVariable(0, "$pref::newThirdPerson", CMDConsole::Bool, &cvars::pref::newThirdPerson);
}