#pragma once

#include "darkstar/Sim/simConsolePlugin.h"
#include "plugins/netXT/playerXT.h"
#include "plugins/netXT/playerPSCXT.h"
#include "plugins/netXT/version.h"
#include "util/hooks.h"
#include "nofix/x86Hook.h"

class BitStream;
class Bullet;
class FearGame;
class Grenade;
class Projectile;
class RocketDumb;
class SimManager;

constexpr char LagCompensatedSetId[] = "LagCompensatedSet";

namespace cvars::net {
// Offset for inserting server snapshots into the client interpolation buffer
inline int timeNudge = 48;
// How far to allow the client's synced clock to drift from the server before correcting
inline int clientClockCorrection = 8;
// How far back in time to allow lag compensation to
inline int maxLagCompensation = 250;
}

class NetXTPlugin : public SimConsolePlugin {
	static inline NetXTPlugin *instance;

public:
	static NetXTPlugin *get()
	{
		return instance;
	}

private:
	static void hook_FearGame_consoleCallback_newGame(CpuState &cs);

	static void __fastcall hook_FearGame_serverProcess(FearGame*);

	static PlayerXT *__fastcall hook_Player_ctor(PlayerXT*);

	static bool __fastcall hook_Player_onAdd(PlayerXT*);

	static void __fastcall hook_Player_serverUpdateMove(
		PlayerXT*, edx_t, PlayerMove *moves, int moveCount);

	static void __fastcall hook_Player_ghostSetMove(
		PlayerXT*, edx_t, PlayerMove *move, const Point3F &newPos, const Point3F &newVel,
		bool newContact, float newRot, float newPitch, int skipCount, bool noInterp);

	static void __fastcall hook_Player_updateMove(
		PlayerXT*, edx_t, PlayerMove *curMove, bool server);

	static void hook_Player_serverProcess_emptyMove(CpuState &cs);

	static void hook_Player_clientProcess_move(PlayerXT*, uint32_t curTime);
	static void hook_Player_clientProcess_move_asm();

	static uint32_t __fastcall hook_Player_packUpdate(
		PlayerXT*, edx_t, Net::GhostManager *gm, uint32_t mask, BitStream *stream);

	static void hook_Player_fireImageProjectile_init(CpuState &cs);

	static void hook_Player_updateMove_landAnim(CpuState &cs);

	static void __fastcall hook_Player_fireImageProjectile(PlayerXT*, edx_t, int imageSlot);

	static PlayerPSCXT *__fastcall hook_PlayerPSC_ctor(PlayerPSCXT*, edx_t, bool in_isServer);

	static bool __fastcall hook_PlayerPSC_writePacket(
		PlayerPSCXT*, edx_t, BitStream *bstream, uint32_t &key);

	static void __fastcall hook_PlayerPSC_readPacket(
		PlayerPSCXT*, edx_t, BitStream *bstream, uint32_t currentTime);

	static void __fastcall hook_PlayerPSC_clientCollectInput(
		PlayerPSCXT*, edx_t, uint32_t startTime, uint32_t endTime);

	static void hook_clampAngleDelta(float *angle, float range);

	static void hook_PlayerPSC_readPacket_setTime(CpuState &cs);
	static void hook_PlayerPSC_readPacket_move(CpuState &cs);
	static void hook_PlayerPSC_writePacket_move(CpuState &cs);
	static void hook_PlayerPSC_onSimActionEvent(CpuState &cs);

	static bool hook_PacketStream_checkPacketSend_check();
	static void hook_PacketStream_checkPacketSend_check_asm();

	static Projectile *__fastcall hook_Projectile_ctor(Projectile*, edx_t, int in_datFileId);

	static void __fastcall hook_Bullet_serverProcess(Bullet*, edx_t, uint32_t in_currTime);
	static void __fastcall hook_RocketDumb_serverProcess(RocketDumb*, edx_t, uint32_t in_currTime);
	static void __fastcall hook_Grenade_serverProcess(Grenade*, edx_t, uint32_t in_currTime);

	struct {
		struct {
			// Fix client reading netcode version bytes backwards
			StaticCodePatch<0x44341E, "\x54"> fixNetcodeVersionMajor;
			StaticCodePatch<0x443422, "\x44"> fixNetcodeVersionMinor;
		} FearCSDelegate;
		struct {
			// Don't skip Player clientProcess when lastProcessTime is currentTime
			StaticCodePatch<0x4E8E02, "\x90\x90"> clientProcess_skipProcessTimeCheck;
			// Add new objects to a new client/server
			x86Hook consoleCallback_newGame = {hook_FearGame_consoleCallback_newGame, 0x4E8288, 2};
			// Update lag compensation snapshots
			StaticJmpHook<0x4E8EE0, hook_FearGame_serverProcess> serverProcess;
		} FearGame;
		struct {
			// Use PlayerXT
			StaticCodePatch<0x42F7D9, PlayerXT::SIZEOF> allocationSize1;
			StaticCodePatch<0x4AE8D7, PlayerXT::SIZEOF> allocationSize2;
			StaticCodePatch<0x4CE4B8, PlayerXT::SIZEOF> allocationSize3;
			StaticCodePatch<0x4CFDA2, PlayerXT::SIZEOF> allocationSize4;
			StaticJmpHook<0x4ACE70, hook_Player_ctor> ctor;
			// Add player to LagCompensatedSet
			StaticJmpHook<0x4AB830, hook_Player_onAdd> onAdd;
			// Run client moves on server
			StaticJmpHook<0x4BBB40, hook_Player_serverUpdateMove> serverUpdateMove;
			// Interpolate/extrapolate remote player
			StaticJmpHook<0x4BBCA0, hook_Player_ghostSetMove> ghostSetMove;
			// Run a single player move
			StaticJmpHook<0x4BA640, hook_Player_updateMove> updateMove;
			// Skip updateImageState calls and handle it ourselves
			// jmp 0x4BA66C
			StaticCodePatch<0x4BA653, "\xEB\x17"> updateMove_noImages;
			// Allow remote players to jump during prediction
			StaticCodePatch<0x4BA7DB, "\xEB"> updateMove_predictJump;
			// Handle forced moves for unresponsive clients
			x86Hook serverProcess_emptyMove = {hook_Player_serverProcess_emptyMove, 0x4BC72E, 1};
			// Predict/interpolate/extrapolate on the client
			StaticJmpHook<0x4BC2B3, hook_Player_clientProcess_move_asm> clientProcess_move;
			// Send player states from the previous move on the server
			StaticJmpHook<0x4BB760, hook_Player_packUpdate> packUpdate;
			// Pass subtick + lag compensation data to projectile
			x86Hook fireImageProjectile_init = {hook_Player_fireImageProjectile_init, 0x4B3985, 3};
			x86Hook startImageFire_init      = {hook_Player_fireImageProjectile_init, 0x4B2223, 3};
			// Remove the client's check for preventing jumps during hard landings
			// It's fake because the server doesn't track animations
			StaticCodePatch<0x4BA7C2, "\x90\x90\x90\x90\x90\x90"> noClientHardLandingCheck;
			// Preserve the visuals by not interrupting the hard landing animation
			x86Hook updateMove_landAnim = {hook_Player_updateMove_landAnim, 0x4BA7F1, 1};
			// Run weapon update logic on the client
			StaticCodePatch<0x4B3DC0, "\x90\x90\x90\x90\x90\x90"> updateWeaponOnClient1;
			StaticCodePatch<0x4B413A, "\x90\x90\x90\x90\x90\x90"> updateWeaponOnClient2;
			StaticCodePatch<0x4B4196, "\x90\x90"> updateWeaponOnClient3;
			StaticCodePatch<0x4B41FA, "\x90\x90"> updateWeaponOnClient4;
			StaticCodePatch<0x4B422E, "\x90\x90"> updateWeaponOnClient5;
			// Predict shots on the client
			StaticJmpHook<0x4B3860, hook_Player_fireImageProjectile> fireImageProjectile;
			// Don't rely on the server for trigger/fire state
			StaticCodePatch<0x4B4049, "\x90\x90\x90"> ignoreServerFire1;
			StaticCodePatch<0x4B409E, "\x90\x90\x90"> ignoreServerFire2;
			StaticCodePatch<0x4B40A5, "\x90\x90\x90\x90\x90"> ignoreServerFire3;
		} Player;
		struct {
			// Use PlayerPSCXT
			StaticCodePatch<0x4429F5, PlayerPSCXT::SIZEOF> allocationSize;
			StaticJmpHook<0x484A10, hook_PlayerPSC_ctor> ctor;
			// Send player states from the previous move on the server
			StaticJmpHook<0x482E30, hook_PlayerPSC_writePacket> writePacket;
			// Protocol extensions
			StaticJmpHook<0x4854C0, hook_PlayerPSC_readPacket> readPacket;
			// Collect subtick inputs and maintain client clock
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
			// Send a packet with every server tick/client move
			StaticJmpHook<0x51A2E4, hook_PacketStream_checkPacketSend_check_asm> checkPacketSend_check;
		} PacketStream;
		struct {
			// Initialize repurposed fields
			StaticJmpHook<0x4C1300, hook_Projectile_ctor> ctor;
		} Projectile;
		struct {
			// Lag compensation
			StaticJmpHook<0x4BF950, hook_Bullet_serverProcess> serverProcess;
		} Bullet;
		struct {
			// Lag compensation
			StaticJmpHook<0x4CAB90, hook_RocketDumb_serverProcess> serverProcess;
		} RocketDumb;
		struct {
			// Lag compensation
			StaticJmpHook<0x4C3F60, hook_Grenade_serverProcess> serverProcess;
		} Grenade;
	} hooks;

	static void setUpWorld(SimManager *manager);

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
