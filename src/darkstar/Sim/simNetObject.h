#pragma once

#include "darkstar/sim/simObject.h"
#include "util/struct.h"

class SimNetObject : public SimObject {
protected:
	enum NetFlag {
		IsGhost       = 1 << 1, // This is a ghost
		PolledGhost   = 1 << 5, // Poll this ghost for updates to non-ghost
		ScopeAlways   = 1 << 6, // if set, object always ghosts to clientReps
		ScopeLocal    = 1 << 7, // Ghost only to local client 2049
		Ghostable     = 1 << 8, // new flag -- set if this object CAN ghost
		Locked        = 1 << 9, // set if the object is locked for editing
		MaxNetFlagBit = 15,
	};

	enum {
		OrientationMask = 0x2000
	};

	FIELD(0x54, uint32_t, netFlags);

public:
	bool isPolledGhost() const
	{
		return netFlags & PolledGhost;
	}

	bool isGhost() const
	{
		return netFlags & IsGhost;
	}

	bool isScopeLocal() const
	{
		return netFlags & ScopeLocal;
	}

	bool isScopeable() const
	{
		return (netFlags & Ghostable) && !(netFlags & ScopeAlways);
	}

	void setMaskBits(uint32_t orMask)
	{
		using func_t = to_static_function_t<decltype(&SimNetObject::setMaskBits)>;
		((func_t)0x530F80)(this, orMask);
	}
};
