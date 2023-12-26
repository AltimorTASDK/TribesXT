#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"

class FearPlugin;

namespace cvars::xt {
inline float speed;
inline float health;
inline float energy;
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

	struct {
		struct {
			StaticJmpHook<0x48BC40, hook_FearPlugin_endFrame> endFrame;
		} FearPlugin;
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
