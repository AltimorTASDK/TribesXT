#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"
#include "nofix/x86Hook.h"

class BitStream;
class Bullet;
class SimRenderQueryImage;

class TracerXTPlugin : public SimConsolePlugin {
	static inline TracerXTPlugin *instance;

	static inline float tracerWidth = 1.0f;

public:
	static TracerXTPlugin *get()
	{
		return instance;
	}

private:
	static void __x86Hook hook_Bullet_TracerRenderImage_render_orientation(CpuState &cs);

	static void __x86Hook hook_Bullet_onSimRenderQueryImage_setWidth(CpuState &cs);
	static void __x86Hook hook_Bullet_readInitialPacket_setSpawnTime(CpuState &cs);
	static void __x86Hook hook_Bullet_writeInitialPacket_setElapsed(CpuState &cs);
	static void __x86Hook hook_Bullet_onAdd_client(CpuState &cs);

	static void __fastcall hook_Bullet_onSimRenderQueryImage(
		Bullet*, edx_t, SimRenderQueryImage *image);

	struct {
		struct {
			struct {
				// Make the tracer always face the camera properly
				x86Hook render_orientation = {hook_Bullet_TracerRenderImage_render_orientation, 0x4C0370, 3};
			} TracerRenderImage;
			// Keep m_spawnTime set by us in readInitialPacket
			StaticCodePatch<0x4BFF7F, "\x90\x90\x90\x90\x90\x90"> preserveSpawnTime;
			// Keep m_spawnPosition set in readInitialPacket
			StaticCodePatch<0x4BFF5D, "\x90\x90\x90\x90\x90\x90"> preserveSpawnPositionX;
			StaticCodePatch<0x4BFF63, "\x90\x90\x90\x90\x90\x90"> preserveSpawnPositionY;
			StaticCodePatch<0x4BFF69, "\x90\x90\x90\x90\x90\x90"> preserveSpawnPositionZ;
			// Draw tracers immediately on bullet spawn
			// mov edx, ebp
			StaticCodePatch<0x4BF313, "\x89\xEA\x90"> setFirstDrawTime;
			// Make tracers follow inherited velocity
			StaticJmpHook<0x4BF140, hook_Bullet_onSimRenderQueryImage> onSimRenderQueryImage;
			// Set custom tracer width
			x86Hook onSimRenderQueryImage_setWidth = {hook_Bullet_onSimRenderQueryImage_setWidth, 0x4BF371, 3};
			// Set m_spawnTime in readInitialPacket based on elapsed time from server
			x86Hook readInitialPacket_setSpawnTime = {hook_Bullet_readInitialPacket_setSpawnTime, 0x4BE01A, 1};
			// Set elapsed time in writeInitialPacket based on m_spawnTime like a normal person
			x86Hook writeInitialPacket_setElapsed = {hook_Bullet_writeInitialPacket_setElapsed, 0x4BDF16, 2};
			// Store shooter velocity in onAdd
			x86Hook onAdd_client = {hook_Bullet_onAdd_client, 0x4C0002, 1};
		} Bullet;
	} hooks;

public:
	TracerXTPlugin()
	{
		instance = this;
	}

	~TracerXTPlugin()
	{
		instance = nullptr;
	}

	void init() override;
};
