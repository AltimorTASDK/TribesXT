#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "plugins/netXT/playerXT.h"
#include "plugins/netXT/playerPSCXT.h"
#include "plugins/netXT/version.h"
#include "util/hooks.h"
#include "nofix/x86Hook.h"

class BitStream;

namespace cvars::net {
inline int timeNudge = 48;
};

class NetXTPlugin : public SimConsolePlugin {
	static inline NetXTPlugin *instance;

public:
	static NetXTPlugin *get()
	{
		return instance;
	}

private:
	static PlayerXT *__fastcall hook_Player_ctor(PlayerXT*);

	static void __fastcall hook_Player_serverUpdateMove(
		PlayerXT*, edx_t, PlayerMove *moves, int moveCount);

	static void __fastcall hook_Player_ghostSetMove(
		PlayerXT*, edx_t, PlayerMove *move, const Point3F &newPos, const Point3F &newVel,
		bool newContact, float newRot, float newPitch, int skipCount, bool noInterp);

	static void __fastcall hook_Player_updateMove(
		PlayerXT*, edx_t, PlayerMove *curMove, bool server);

	static void hook_Player_clientProcess_move(PlayerXT*, uint32_t curTime);
	static void hook_Player_clientProcess_move_asm();

	static uint32_t __fastcall hook_Player_packUpdate(
		PlayerXT*, edx_t, Net::GhostManager *gm, uint32_t mask, BitStream *stream);

	static PlayerPSCXT *__fastcall hook_PlayerPSC_ctor(PlayerPSCXT*, edx_t, bool in_isServer);

	static bool __fastcall hook_PlayerPSC_writePacket(
		PlayerPSCXT*, edx_t, BitStream *bstream, uint32_t &key);

	static void __fastcall hook_PlayerPSC_clientCollectInput(
		PlayerPSCXT*, edx_t, uint32_t startTime, uint32_t endTime);

	static void hook_clampAngleDelta(float *angle, float range);

	static void __x86Hook hook_PlayerPSC_readPacket_setTime(CpuState &cs);
	static void __x86Hook hook_PlayerPSC_readPacket_move(CpuState &cs);
	static void __x86Hook hook_PlayerPSC_writePacket_move(CpuState &cs);
	static void __x86Hook hook_PlayerPSC_onSimActionEvent(CpuState &cs);

	static bool hook_PacketStream_checkPacketSend_check();
	static void hook_PacketStream_checkPacketSend_check_asm();

	struct {
		struct {
			// Fix client reading netcode version bytes backwards
			StaticCodePatch<0x44341E, "\x54"> fixNetcodeVersionMajor;
			StaticCodePatch<0x443422, "\x44"> fixNetcodeVersionMinor;
		} FearCSDelegate;
		struct {
			// Don't skip Player clientProcess when lastProcessTime is currentTime
			StaticCodePatch<0x4E8E02, "\x90\x90"> skipProcessTimeCheck;
		} FearGame;
		struct {
			// Use PlayerXT
			StaticCodePatch<0x42F7D9, PlayerXT::SIZEOF> allocationSize1;
			StaticCodePatch<0x4AE8D7, PlayerXT::SIZEOF> allocationSize2;
			StaticCodePatch<0x4CE4B8, PlayerXT::SIZEOF> allocationSize3;
			StaticCodePatch<0x4CFDA2, PlayerXT::SIZEOF> allocationSize4;
			StaticJmpHook<0x4ACE70, hook_Player_ctor> ctor;
			// Run client moves on server
			StaticJmpHook<0x4BBB40, hook_Player_serverUpdateMove> serverUpdateMove;
			// Interpolate/extrapolate remote player
			StaticJmpHook<0x4BBCA0, hook_Player_ghostSetMove> ghostSetMove;
			// Run a single player move
			StaticJmpHook<0x4BA640, hook_Player_updateMove> updateMove;
			// Skip updateImageState calls and handle it ourselves
			// jmp 0x4BA66C
			StaticCodePatch<0x4BA653, "\xEB\x17"> updateMove_noImages;
			// Predict/interpolate/extrapolate on the client
			StaticJmpHook<0x4BC2B3, hook_Player_clientProcess_move_asm> clientProcess_move;
			// Send player states from the previous move on the server
			StaticJmpHook<0x4BB760, hook_Player_packUpdate> packUpdate;
		} Player;
		struct {
			// Use PlayerPSCXT
			StaticCodePatch<0x4429F5, PlayerPSCXT::SIZEOF> allocationSize;
			StaticJmpHook<0x484A10, hook_PlayerPSC_ctor> ctor;
			// Send player states from the previous move on the server
			StaticJmpHook<0x482E30, hook_PlayerPSC_writePacket> writePacket;
			// Collect subtick inputs
			StaticJmpHook<0x484E30, hook_PlayerPSC_clientCollectInput> clientCollectInput;
			// Don't clamp IDACTION_PITCH
			StaticCodePatch<0x483F9F, "\x18"> onSimActionEvent_noPitchClamp1;
			StaticCodePatch<0x483FAE, "\x18"> onSimActionEvent_noPitchClamp2;
			// Don't clamp IDACTION_YAW
			StaticCodePatch<0x483F10, "\x18"> onSimActionEvent_noYawClamp1;
			StaticCodePatch<0x483F1F, "\x18"> onSimActionEvent_noYawClamp2;
			// Don't clamp move angle deltas on the server
			StaticJmpHook<0x482A90, hook_clampAngleDelta> clampAngleDelta;
			// Invalidate predicted snapshots after a rollback
			x86Hook readPacket_setTime = {hook_PlayerPSC_readPacket_setTime, 0x485945, 1};
			// Read subtick
			x86Hook readPacket_move    = {hook_PlayerPSC_readPacket_move,    0x485634, 1};
			// Write subtick
			x86Hook writePacket_move   = {hook_PlayerPSC_writePacket_move,   0x483977, 2};
			// Handle subtick inputs
			x86Hook onSimActionEvent   = {hook_PlayerPSC_onSimActionEvent,   0x483DF4, 2};
		} PlayerPSC;
		struct {
			StaticJmpHook<0x51A2E4, hook_PacketStream_checkPacketSend_check_asm> checkPacketSend_check;
		} PacketStream;
	} hooks;

public:
	NetXTPlugin()
	{
		instance = this;
		ourNetcodeVersion = Netcode::XT::Latest;
	}

	~NetXTPlugin()
	{
		instance = nullptr;
		ourNetcodeVersion = Netcode::New;
	}

	void init() override;
};
