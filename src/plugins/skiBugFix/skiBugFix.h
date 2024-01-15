#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"

class SkiBugFixPlugin : public SimConsolePlugin {
	struct {
		struct {
			// Increase max collision attempts before deadstopping
			StaticCodePatch<0x52EB82, 8> DefaultCollisionRetryCount;
		} SimMovement;
	} hooks;
};
