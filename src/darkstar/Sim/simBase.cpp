#include "darkstar/Sim/simBase.h"
#include "tribes/worldGlobals.h"

static auto emptyObjectList = SimObjectList();

const SimObjectList &SimSet::iterate(SimObjectId id)
{
	const auto *set = (SimSet*)wg->manager->findObject(id);
	return set != nullptr ? set->objectList : emptyObjectList;
}

const SimObjectList &SimSet::iterate(const char *name)
{
	const auto *set = (SimSet*)wg->manager->findObject(name);
	return set != nullptr ? set->objectList : emptyObjectList;
}
