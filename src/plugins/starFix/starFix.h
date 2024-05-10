#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"
#include "nofix/x86Hook.h"

class StarFixPlugin : public SimConsolePlugin {
	static void hook_StarField_render_setZ_rect();
	static void hook_StarField_render_setZ_point();

	static void hook_Processor_SSE_transformProject_setZ(CpuState &cs);

	struct {
		struct {
			StaticJmpHook<0x4DD126, hook_StarField_render_setZ_rect> render_setZ_rect;
			StaticJmpHook<0x4DD156, hook_StarField_render_setZ_point> render_setZ_point;
			StaticCodePatch<0x65FBE8, 1.0> distanceMultiplier;
		} StarField;
		struct {
			x86Hook transformProject_setZ = {hook_Processor_SSE_transformProject_setZ, 0x50E5E1, 1};
		} Processor_SSE;
	} hooks;
};
