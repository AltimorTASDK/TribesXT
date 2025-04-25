#include "darkstar/Core/bitstream.h"
#include "darkstar/Ml/random.h"
#include "darkstar/Sim/simGame.h"
#include "darkstar/Sim/Net/ghostManager.h"
#include "tribes/bullet.h"
#include "tribes/fearDcl.h"
#include "tribes/grenade.h"
#include "tribes/playerPSC.h"
#include "tribes/projectile.h"
#include "tribes/rocketDumb.h"
#include "tribes/worldGlobals.h"
#include "plugins/netXT/netXT.h"
#include "plugins/netXT/playerXT.h"
#include "plugins/netXT/playerPSCXT.h"
#include "plugins/netXT/version.h"
#include "util/math.h"
#include <bit>
#include <cmath>
#include <vector>

// Must uniquely identify any previously unacked move
constexpr auto PredictionKeyMask = PlayerPSCXT::MaxMovesXT - 1;
constexpr auto PredictionKeyBits = std::bit_width(PredictionKeyMask);

void NetXTPlugin::hook_GhostManager_writePacket_newGhost(CpuState &cs)
{
	const auto *ghostManager = (Net::GhostManager*)cs.reg.ebp;
	const auto *info = (Net::GhostInfo*)cs.reg.esi;
	auto *stream = *(BitStream**)(cs.reg.esp + 0x30);

	if (!isProjectileTag(info->obj->getGhostTag()))
		return;

	const auto *projectile = (Projectile*)info->obj;
	const auto *scopeObject = ghostManager->scopeObject;
	const auto *shooter = projectile->m_pShooter;

	if (stream->writeFlag(shooter != nullptr && shooter == scopeObject))
		stream->writeInt(projectile->predictionKeyXT, PredictionKeyBits);
}

void NetXTPlugin::hook_GhostManager_readPacket(
	Net::GhostManager *ghostManager, edx_t, BitStream *bstream, uint32_t time)
{
	get()->hooks.Net.GhostManager.readPacket.callOriginal(ghostManager, bstream, time);

	if (!Netcode::XT::ClientProjectiles.check() || !ghostManager->allowGhosts)
		return;

	// Copy for safe iteration
	const auto &list = SimSet::iterate<Projectile>(ClientProjectileSetId);
	const auto projectiles = std::vector(list.begin(), list.end());

	for (const auto projectile : projectiles) {
		// Delete mispredicted projectiles
		if (projectile->predictionKeyXT < (uint32_t)cg.psc->firstMoveSeq)
			projectile->deleteObject();
	}
}

Persistent::Base *NetXTPlugin::hook_GhostManager_readPacket_newGhost(BitStream *stream, uint32_t tag)
{
	if (Netcode::XT::ClientProjectiles.check() && isProjectileTag(tag) && stream->readFlag()) {
		const auto predictionKey = stream->readInt(PredictionKeyBits);

		for (const auto projectile : SimSet::iterate<Projectile>(ClientProjectileSetId)) {
			if ((projectile->predictionKeyXT & PredictionKeyMask) == predictionKey) {
				projectile->removeFromSet(ClientProjectileSetId);
				projectile->m_lastUpdated = projectile->spawnTimeXT;
				return projectile.get();
			}
		}
	}

	return Persistent::create(tag);
}

__declspec(naked) Persistent::Base *NetXTPlugin::hook_GhostManager_readPacket_newGhost_asm()
{
	__asm
	{
		push eax
		push esi
		call hook_GhostManager_readPacket_newGhost
		add esp, 8
		mov ecx, 0x519554
		jmp ecx
	}
}

bool NetXTPlugin::hook_SimManager_registerObject(SimManager *manager, edx_t, SimObject *obj)
{
	// Don't re-add predicted projectiles when they get ghosted
	if (obj->getManager() != nullptr)
		return true;

	return get()->hooks.SimManager.registerObject.callOriginal(manager, obj);
}

void NetXTPlugin::hook_FearGame_consoleCallback_newGame(CpuState &cs)
{
	auto *manager = (SimManager*)cs.reg.esi;
	setUpWorld(manager);
}

void NetXTPlugin::hook_FearGame_serverProcess(FearGame *game)
{
	get()->hooks.FearGame.serverProcess.callOriginal(game);

	// Check if the server ticked, which also coincides with packet send
	if (sg.currentTime != sg.lastTime)
		PlayerXT::saveLagCompensationSnapshotAll(sg.currentTime);
}

void NetXTPlugin::hook_GameBaseData_pack(GameBase::GameBaseData *data, edx_t, BitStream *stream)
{
	get()->hooks.GameBase.GameBaseData.pack.callOriginal(data, stream);
	stream->writeString(data->className);
}
void NetXTPlugin::hook_GameBaseData_unpack(GameBase::GameBaseData *data, edx_t, BitStream *stream)
{
	get()->hooks.GameBase.GameBaseData.unpack.callOriginal(data, stream);
	if (Netcode::XT::GameBaseDataSendClassName.check())
		data->className = stream->readSTString();
}

PlayerXT *NetXTPlugin::hook_Player_ctor(PlayerXT *player)
{
	// Initialize new fields
	new (&player->xt) PlayerXT::DataXT;
	return get()->hooks.Player.ctor.callOriginal(player);
}

bool NetXTPlugin::hook_Player_onAdd(PlayerXT *player)
{
	if (!get()->hooks.Player.onAdd.callOriginal(player))
		return false;

	player->addToSet(LagCompensatedSetId);
	return true;
}

void NetXTPlugin::hook_Player_kill(PlayerXT *player)
{
	get()->hooks.Player.kill.callOriginal(player);
	player->removeFromSet(LagCompensatedSetId);
}

void NetXTPlugin::hook_Player_serverUpdateMove(
	PlayerXT *player, edx_t, PlayerMove *moves, int moveCount)
{
	player->serverUpdateMove(moves, moveCount);
}

void NetXTPlugin::hook_Player_ghostSetMove(
	PlayerXT *player, edx_t, PlayerMove *move, const Point3F &newPos, const Point3F &newVel,
	bool newContact, float newRot, float newPitch, int skipCount, bool noInterp)
{
	player->ghostSetMove(move, newPos, newVel, newContact,
	                     newRot, newPitch, skipCount, noInterp);
}

void NetXTPlugin::hook_Player_updateMove(PlayerXT *player, edx_t, PlayerMove *curMove, bool server)
{
	get()->hooks.Player.updateMove.callOriginal(player, curMove, server);
	player->lastProcessTime += TickMs;
	player->saveSnapshot(player->lastProcessTime);
}

void NetXTPlugin::hook_Player_updateMove_jumpAnim(CpuState &cs)
{
	// Track jump count so the server can drive the animation
	if (auto *player = (PlayerXT*)cs.reg.ebp; !player->isGhost())
		player->xt.jumpCount++;

	// Prevent clients from interrupting ANIM_LAND with ANIM_JUMPRUN
	// Vanilla clients normally believe they can't jump at all out of ANIM_LAND
	if (cs.reg.eax == Player::ANIM_LAND)
		cs.reg.eax = Player::ANIM_JUMPRUN;
}

uint32_t NetXTPlugin::hook_Player_packUpdate(
	PlayerXT *player, edx_t, Net::GhostManager *gm, uint32_t mask, BitStream *stream)
{
	const auto retMask = get()->hooks.Player.packUpdate.callOriginal(player, gm, mask, stream);
	player->packUpdateXT(gm, mask, stream);
	return retMask;
}

void NetXTPlugin::hook_Player_unpackUpdate(
	PlayerXT *player, edx_t, Net::GhostManager *gm, BitStream *stream)
{
	get()->hooks.Player.unpackUpdate.callOriginal(player, gm, stream);
	player->unpackUpdateXT(gm, stream);
}

void NetXTPlugin::hook_Player_serverProcess_emptyMove(CpuState &cs)
{
	auto *player = (PlayerXT*)cs.reg.ebp;
	// Update weapon after move
	for (auto i = 0; i < Player::MaxItemImages; i++)
		player->updateImageState(i, 0.032f);
}

 void NetXTPlugin::hook_Player_clientProcess_move(PlayerXT *player, uint32_t curTime)
 {
	 player->clientMove(curTime);
 }

__declspec(naked) void NetXTPlugin::hook_Player_clientProcess_move_asm()
{
	__asm
	{
		push [esp+0x34]
		push ebx
		call hook_Player_clientProcess_move
		add esp, 8
		// Jump past move/interp
		fldz
		mov eax, 0x4BC409
		jmp eax
	}
}

void NetXTPlugin::hook_Player_fireImageProjectile_init(CpuState &cs)
{
	auto *player = (PlayerXT*)cs.reg.esi;
	auto *projectile = (Projectile*)cs.reg.edi;
	player->initProjectileXT(projectile);
}

void NetXTPlugin::hook_Player_fireImageProjectile(PlayerXT *player, edx_t, int imageSlot)
{
	get()->hooks.Player.fireImageProjectile.callOriginal(player, imageSlot);

	if (Netcode::XT::ClientProjectiles.check() && player->isGhost())
		player->clientFireImageProjectile(imageSlot);
}

void NetXTPlugin::hook_Player_updateImageState(PlayerXT *player, edx_t, int imageSlot, float dt)
{
	// Update ammo state on the client
	auto *itemImage = &player->itemImageList[imageSlot];

	if (itemImage->imageId != -1 && player->isGhost()) {
		const auto &imageData = *player->getItemImageData(itemImage->imageId);

		if (imageData.ammoType == -1)
			itemImage->ammo = player->energy > imageData.minEnergy;
		else
			itemImage->ammo = cg.psc->itemCount(imageData.ammoType) > 0;
	}

	get()->hooks.Player.updateImageState.callOriginal(player, imageSlot, dt);
}

void NetXTPlugin::hook_Player_unpackItemImages_initial(CpuState &cs)
{
	// Initialize to Activate state if the server doesn't tell us to start in Fire
	auto *image = (Player::ItemImageEntry*)(cs.reg.ebx - 0x1C);
	image->state = cs.eflag.zf ? Player::ItemImageEntry::Activate
	                           : Player::ItemImageEntry::Fire;
}

void NetXTPlugin::hook_ItemImageData_pack(Player::ItemImageData *data, edx_t, BitStream *stream)
{
	get()->hooks.Player.ItemImageData.pack.callOriginal(data, stream);
	stream->write(data->accuFire);
}

void NetXTPlugin::hook_ItemImageData_unpack(Player::ItemImageData *data, edx_t, BitStream *stream)
{
	get()->hooks.Player.ItemImageData.unpack.callOriginal(data, stream);
	if (Netcode::XT::ItemImageDataSendAccuFire.check())
		stream->read(&data->accuFire);
}

PlayerPSCXT *NetXTPlugin::hook_PlayerPSC_ctor(PlayerPSCXT *psc, edx_t, bool in_isServer)
{
	// Initialize new fields
	new (&psc->xt) PlayerPSCXT::DataXT;
	return get()->hooks.PlayerPSC.ctor.callOriginal(psc, in_isServer);
}

bool NetXTPlugin::hook_PlayerPSC_writePacket(
	PlayerPSCXT *psc, edx_t, BitStream *bstream, uint32_t &key)
{
	if (psc->isServer)
		psc->writeClockSync(bstream);

	return get()->hooks.PlayerPSC.writePacket.callOriginal(psc, bstream, key);
}

void NetXTPlugin::hook_PlayerPSC_readPacket(
	PlayerPSCXT *psc, edx_t, BitStream *bstream, uint32_t currentTime)
{
	if (psc->isServer) {
		if (auto *player = psc->getPlayerXT(); player != nullptr)
			player->xt.moveCount = psc->lastPlayerMove;
	} else {
		psc->readClockSync(bstream);
	}

	get()->hooks.PlayerPSC.readPacket.callOriginal(psc, bstream, currentTime);
}

void NetXTPlugin::hook_PlayerPSC_readPacket_checkReadMove(CpuState &cs)
{
	const auto *psc = (PlayerPSCXT*)cs.reg.ebx;

	// Check if the client is trying to pack more moves than max
	if (!cs.eflag.zf && psc->moves.size() >= PlayerPSC::MaxMoves) {
		Net::setLastError("Too many moves.");
		cs.eflag.zf = true;
	}
}

void NetXTPlugin::hook_PlayerPSC_clientCollectInput(
	PlayerPSCXT *psc, edx_t, uint32_t startTime, uint32_t endTime)
{
	psc->clientUpdateClock(startTime, endTime);
	psc->collectSubtickInput(startTime, endTime);
	get()->hooks.PlayerPSC.clientCollectInput.callOriginal(psc, startTime, endTime);
}

void NetXTPlugin::hook_clampAngleDelta(float *angle, float range)
{
	// Check subnormal/infinite/NaN
	if (std::isnormal(*angle))
		*angle = normalize_radians_signed(*angle);
	else
		*angle = 0.f;
}

void NetXTPlugin::hook_PlayerPSC_readPacket_setTime(CpuState &cs)
{
	const auto *psc = (PlayerPSCXT*)cs.reg.ebx;

	if (auto *player = psc->getPlayerXT(); player != nullptr) {
		if (Netcode::XT::SendCurrentPlayerState.check()) {
			// Adjust for extra move kept in the buffer
			player->lastProcessTime += TickMs;
		}
		player->invalidatePrediction(player->lastProcessTime);
		player->saveSnapshot(player->lastProcessTime);
		player->xt.moveCount = psc->firstMoveSeq + 1;
	}
}

void NetXTPlugin::hook_PlayerPSC_readPacket_move(CpuState &cs)
{
	auto *psc = (PlayerPSCXT*)cs.reg.ebx;
	auto *stream = (BitStream*)cs.reg.ebp;
	psc->readSubtick(stream);
	psc->readLagCompensation(stream);
}

void NetXTPlugin::hook_PlayerPSC_writePacket_move(CpuState &cs)
{
	auto *psc = (PlayerPSCXT*)cs.reg.ebp;
	auto *stream = (BitStream*)cs.reg.edi;
	const auto moveIndex = (int)cs.reg.eax;
	psc->writeSubtick(stream, moveIndex);
	psc->writeLagCompensation(stream, moveIndex);
}

void NetXTPlugin::hook_PlayerPSC_onSimActionEvent(CpuState &cs)
{
	auto *psc = (PlayerPSCXT*)cs.reg.ecx;
	const auto action = (int)cs.reg.eax;
	const auto eventValue = *(float*)(cs.reg.ebx + 0x28);
	psc->preSimActionEvent(action, eventValue);
}

bool NetXTPlugin::hook_PacketStream_checkPacketSend_check()
{
	// Check if server ticked or client produced a move
	if (wg == &sg)
		return sg.currentTime != sg.lastTime;
	else
		return msToTicks(cg.currentTime - 1) != msToTicks(cg.lastTime - 1);
}

__declspec(naked) void NetXTPlugin::hook_PacketStream_checkPacketSend_check_asm()
{
	__asm {
		call hook_PacketStream_checkPacketSend_check
		test al, al
		je skip
		mov eax, 0x51A362
		jmp eax
	skip:
		mov eax, 0x51A574
		jmp eax
	}
}

Projectile *NetXTPlugin::hook_Projectile_ctor(Projectile *projectile, edx_t, int in_datFileId)
{
	get()->hooks.Projectile.ctor.callOriginal(projectile, in_datFileId);
	projectile->subtickOffsetXT = -1;
	projectile->lagCompensationOffsetXT = -1;
	projectile->predictionKeyXT = -1;
	projectile->spawnTimeXT = -1;
	return projectile;
}

void NetXTPlugin::hook_Projectile_deflectProjectile(Projectile *projectile, edx_t, float deflection)
{
	if (deflection == 0.0f)
		return;

	// Used synced seed
	auto random = Random(projectile->predictionKeyXT);

	const auto spread = EulerF(
		(random.getFloat() - 0.5f) * 2 * math::pi * deflection,
		(random.getFloat() - 0.5f) * 2 * math::pi * deflection,
		(random.getFloat() - 0.5f) * 2 * math::pi * deflection);

	projectile->setLinearVelocity(projectile->getLinearVelocity() * RMat3F(spread));
}

void NetXTPlugin::hook_ProjectileData_pack(Projectile::ProjectileData *data, edx_t, BitStream *stream)
{
	get()->hooks.Projectile.ProjectileData.pack.callOriginal(data, stream);
	stream->write(data->inheritedVelocityScale);
	stream->write(data->aimDeflection);
}

void NetXTPlugin::hook_ProjectileData_unpack(Projectile::ProjectileData *data, edx_t, BitStream *stream)
{
	get()->hooks.Projectile.ProjectileData.unpack.callOriginal(data, stream);
	if (Netcode::XT::ProjectileDataSendInheritance.check())
		stream->read(&data->inheritedVelocityScale);
	if (Netcode::XT::ProjectileDataSendAimDeflection.check())
		stream->read(&data->aimDeflection);
}

static void serverProcessWithLagCompensation(Projectile *projectile, uint32_t in_currTime, void *serverProcessPtr)
{
	auto *serverProcess = (void(__thiscall*)(Projectile*, uint32_t))serverProcessPtr;

	if (projectile->shouldLagCompensate())
	{
		PlayerXT *shooter = (PlayerXT*)projectile->m_pShooter;
		const auto lagCompensationTime = in_currTime - projectile->lagCompensationOffsetXT;
		PlayerXT::startLagCompensationAll(shooter, lagCompensationTime);

		if (projectile->hasSubtick()) {
			// Shooter needs to be matched to projectile subtick for explosion impulse
			const auto subtickAdjustTime = in_currTime - TickMs + projectile->subtickOffsetXT;
			shooter->startLagCompensation(subtickAdjustTime);
			serverProcess(projectile, in_currTime);
			shooter->endLagCompensation();
		} else {
			serverProcess(projectile, in_currTime);
		}

		PlayerXT::endLagCompensationAll(shooter);
	} else {
		serverProcess(projectile, in_currTime);
	}
}

void NetXTPlugin::hook_Bullet_serverProcess(Bullet *projectile, edx_t, uint32_t in_currTime)
{
	serverProcessWithLagCompensation(projectile, in_currTime, get()->hooks.Bullet.serverProcess.getOriginal());
}

void NetXTPlugin::hook_RocketDumb_serverProcess(RocketDumb *projectile, edx_t, uint32_t in_currTime)
{
	serverProcessWithLagCompensation(projectile, in_currTime, get()->hooks.RocketDumb.serverProcess.getOriginal());
}

void NetXTPlugin::hook_Grenade_serverProcess(Grenade *projectile, edx_t, uint32_t in_currTime)
{
	serverProcessWithLagCompensation(projectile, in_currTime, get()->hooks.Grenade.serverProcess.getOriginal());
}

void NetXTPlugin::setUpWorld(SimManager *manager)
{
	manager->addObject(new SimSet(false), LagCompensatedSetId);
	manager->addObject(new SimSet(false), ClientProjectileSetId);
}

void NetXTPlugin::init()
{
	console->addVariable(0, "net::timeNudge",             CMDConsole::Int, &cvars::net::timeNudge);
	console->addVariable(0, "net::clientClockCorrection", CMDConsole::Int, &cvars::net::clientClockCorrection);
	console->addVariable(0, "net::maxLagCompensation",    CMDConsole::Int, &cvars::net::maxLagCompensation);
	console->addVariable(0, "net::ghostTimeout",          CMDConsole::Int, &cvars::net::ghostTimeout);

	// Set up already running worlds
	if (cg.manager != nullptr)
		setUpWorld(cg.manager);

	if (sg.manager != nullptr)
		setUpWorld(sg.manager);
}