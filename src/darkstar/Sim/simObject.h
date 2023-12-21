#pragma once

#include "darkstar/Sim/simEvDecl.h"
#include "util/struct.h"

class SimObject {
	enum Flag {
		Deleted   = BIT(0),
		Removed   = BIT(1),
		DynamicId = BIT(2),
		Added     = BIT(3),
		Selected  = BIT(4),
		Expanded  = BIT(5),
	};
	FIELD(0x10, BitSet32, flags);

protected:
	FIELD(0x44, SimObjectId, id);

public:
	bool isDeleted() const
	{
		return flags.test(Deleted);
	}

	bool isRemoved() const
	{
		return flags.test(Deleted | Removed);
	}

	SimObjectId getId() const
	{
		return id;
	}
};
