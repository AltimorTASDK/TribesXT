#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "util/hooks.h"
#include "nofix/x86Hook.h"

class BitStream;
class Bullet;
struct SimRenderQueryImage;
namespace Net {
class GhostManager;
}

namespace cvars::pref {
inline float tracerWidth  = 1.0f;
inline float tracerLength = 1.0f;
};

class TracerXTPlugin : public SimConsolePlugin {
	static inline TracerXTPlugin *instance;

public:
	static TracerXTPlugin *get()
	{
		return instance;
	}

private:
	static void __x86Hook hook_Bullet_TracerRenderImage_render_orientation(CpuState &cs);

	static void __fastcall hook_Bullet_writeInitialPacket(
		Bullet*, edx_t, Net::GhostManager *ghostManager, BitStream *stream);

	static void __fastcall hook_Bullet_readInitialPacket(
		Bullet*, edx_t, Net::GhostManager *ghostManager, BitStream *stream);

	static void __fastcall hook_Bullet_onSimRenderQueryImage(
		Bullet*, edx_t, SimRenderQueryImage *image);

	static void __x86Hook hook_Bullet_onSimRenderQueryImage_setWidth(CpuState &cs);

	struct {
		struct {
			struct {
				// Make the tracer always face the camera properly
				x86Hook render_orientation = {hook_Bullet_TracerRenderImage_render_orientation, 0x4C0370, 3};
			} TracerRenderImage;
			// Keep m_spawnPosition set in readInitialPacket
			StaticCodePatch<0x4BFF5D, "\x90\x90\x90\x90\x90\x90"> preserveSpawnPositionX;
			StaticCodePatch<0x4BFF63, "\x90\x90\x90\x90\x90\x90"> preserveSpawnPositionY;
			StaticCodePatch<0x4BFF69, "\x90\x90\x90\x90\x90\x90"> preserveSpawnPositionZ;
			// Keep m_spawnTime set by us in readInitialPacket
			StaticCodePatch<0x4BFF7F, "\x90\x90\x90\x90\x90\x90"> preserveSpawnTime;
			// Set elapsed time in writeInitialPacket based on m_spawnTime like a normal person
			StaticJmpHook<0x4BDE60, hook_Bullet_writeInitialPacket> writeInitialPacket;
			// Set m_spawnTime in readInitialPacket based on elapsed time from server
			StaticJmpHook<0x4BDFD0, hook_Bullet_readInitialPacket> readInitialPacket;
			// Make tracers follow inherited velocity and set custom length
			StaticJmpHook<0x4BF140, hook_Bullet_onSimRenderQueryImage> onSimRenderQueryImage;
			// Draw tracers immediately on bullet spawn
			// mov edx, ebp
			StaticCodePatch<0x4BF313, "\x89\xEA\x90"> onSimRenderQueryImage_setFirstDrawTime;
			// Set custom tracer width
			x86Hook onSimRenderQueryImage_setWidth = {hook_Bullet_onSimRenderQueryImage_setWidth, 0x4BF371, 3};
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
