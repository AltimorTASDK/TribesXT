#pragma once

#include "darkstar/Core/bitset.h"
#include "darkstar/Core/tVector.h"
#include "darkstar/Sim/simBase.h"
#include "util/struct.h"

namespace Net {

class GhostManager;
struct GhostInfo;
struct GhostRef {
	GhostManager *ghostManager;
	GhostInfo *ghostInfo;
};

} // namespace Net

class SimNetObject : public SimObject {
	friend class PlayerXT;

protected:
	enum NetFlag {
		IsGhost       = BIT(1), // This is a ghost
		PolledGhost   = BIT(5), // Poll this ghost for updates to non-ghost
		ScopeAlways   = BIT(6), // if set, object always ghosts to clientReps
		ScopeLocal    = BIT(7), // Ghost only to local client 2049
		Ghostable     = BIT(8), // new flag -- set if this object CAN ghost
		Locked        = BIT(9), // set if the object is locked for editing
		MaxNetFlagBit = 15,
	};

	FIELD(0x54, BitSet32, netFlags);
	FIELD(0x60, Vector<Net::GhostRef>, ghosts);

public:
	bool isPolledGhost() const
	{
		return netFlags.test(PolledGhost);
	}

	bool isGhost() const
	{
		return netFlags.test(IsGhost);
	}

	bool isScopeLocal() const
	{
		return netFlags.test(ScopeLocal);
	}

	bool isScopeable() const
	{
		return netFlags.test(Ghostable) && !netFlags.test(ScopeAlways);
	}

	uint32_t getGhostTag() const
	{
		return call_virtual<24, decltype(&SimNetObject::getGhostTag)>(this);
	}

	void setMaskBits(uint32_t orMask)
	{
		using func_t = to_static_function_t<decltype(&SimNetObject::setMaskBits)>;
		((func_t)0x530F80)(this, orMask);
	}
};
