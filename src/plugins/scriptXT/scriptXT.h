#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"

class FearPlugin;
class PlayerPSC;
struct SimQuery;

namespace cvars::xt {
inline float speed;
inline float health;
inline float energy;
};

namespace cvars::pref {
inline float damageFlash = 0.35f;
};

class ScriptXTPlugin : public SimConsolePlugin {
	static inline ScriptXTPlugin *instance;

public:
	static ScriptXTPlugin *get()
	{
		return instance;
	}

private:
	static void __fastcall hook_FearPlugin_endFrame(FearPlugin*);

	static bool __fastcall hook_PlayerPSC_processQuery(PlayerPSC*, edx_t, SimQuery *query);

	struct {
		struct {
			// Set new player info variables
			StaticJmpHook<0x48BC40, hook_FearPlugin_endFrame> endFrame;
		} FearPlugin;
		struct {
			// Fix right alignment with multiple formatting segments
			StaticCodePatch<0x55367F, "\xEB"> formatControlString_fixRightAlign;
		} TextFormat;
		struct {
			// Right align based on text control width rather than parent width
			// push edi
			// mov edi, [esi+0x1F8]
			// jmp 0x478E0E
			StaticCodePatch<0x478DF6, "\x57\x8B\xBE\xF8\x01\x00\x00\xEB\x0F"> setScriptValue_fixExtent;
			// ret 8
			StaticCodePatch<0x478E60, "\xC2\x08\x00"> parentResized_fixExtent;
		} FearGuiFormattedText;
		struct {
			// Apply damage flash cvar
			StaticJmpHook<0x484720, hook_PlayerPSC_processQuery> processQuery;
		} PlayerPSC;
	} hooks;

public:
	ScriptXTPlugin()
	{
		instance = this;
	}

	~ScriptXTPlugin()
	{
		instance = nullptr;
	}

	void init() override;
};
