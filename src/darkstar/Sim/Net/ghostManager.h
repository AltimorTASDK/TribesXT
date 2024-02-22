#pragma once

#include "darkstar/Core/bitset.h"
#include "darkstar/Sim/simNetObject.h"
#include "util/struct.h"

namespace Net {

class GhostManager {
public:
	static constexpr auto MaxGhostCount = 1024u;

	struct PacketObjectRef {
		uint32_t mask;
		uint32_t ghostInfoFlags;
		GhostInfo *ghost;
		PacketObjectRef *nextRef;
		PacketObjectRef *nextUpdateChain;
	};

	FIELD(0x8C, SimNetObject*, scopeObject);
	FIELD(0xC4, bool, allowGhosts);
};

struct GhostInfo {
	enum Flags {
		Valid = BIT(0),
		InScope = BIT(1),
		ScopeAlways = BIT(2),
		NotYetGhosted = BIT(3),
		Ghosting = BIT(4),
		KillGhost = BIT(5),
		KillingGhost = BIT(6),
		LastGhostAlwaysPacket = BIT(7),
	};

	SimNetObject *localGhost; // local ghost for remote object
	uint32_t updateMask;      // 32 bits of object information - object owns these
	SimNetObject *obj;        // the 'real' object.
	GhostManager::PacketObjectRef *updateChain;
	uint32_t flags;           // static, fluff, etc.
	GhostInfo *nextRef;
	GhostInfo *prevRef;
	uint16_t ghostIndex;
	uint16_t updateSkipCount;
};

void setLastError(const char *in_pErrorString)
{
	((decltype(setLastError)*)0x517EF0)(in_pErrorString);
}

} // namespace Net
