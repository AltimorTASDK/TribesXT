#pragma once

#include "darkstar/Sim/simNetObject.h"
#include "util/memory.h"

class SimContainer : public SimNetObject {
public:
	bool addObject(SimContainer *object)
	{
		return call_virtual<30, decltype(&SimContainer::addObject)>(this, object);
	}
};
