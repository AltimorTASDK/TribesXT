#pragma once

#include "darkstar/Core/bitset.h"
#include "darkstar/Sim/simEvDecl.h"
#include "util/struct.h"
#include <cstddef>

struct SimQuery {
	int type;
};

class SimObject {
	friend class SimGroup;

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
	bool addToSet(uint32_t setId)
	{
		using func_t = bool(__thiscall*)(SimObject*, uint32_t);
		return ((func_t)0x51C6C0)(this, setId);
	}

	bool addToSet(const char *name)
	{
		using func_t = bool(__thiscall*)(SimObject*, const char*);
		return ((func_t)0x51C710)(this, name);
	}

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

class SimSet : public SimObject {
public:
	static constexpr size_t SIZEOF = 0x6C;

	static void *operator new(size_t count)
	{
		return ::operator new(SimSet::SIZEOF);
	}

	SimSet(bool own)
	{
		using func_t = SimSet*(__thiscall*)(SimSet*, bool);
		((func_t)0x51D9E0)(this, own);
	}

	SimObject *addObject(SimObject *object)
	{
		return call_virtual<23, decltype(&SimSet::addObject)>(this, object);
	}
};

class SimGroup : public SimSet {
public:
	void assignName(SimObject *object, const char *name)
	{
		return call_virtual<25, decltype(&SimGroup::assignName)>(this, object, name);
	}

	SimObject *findObject(const char *name) const
	{
		using func_t = SimObject*(SimGroup::*)(const char*) const;
		return call_virtual<26, func_t>(this, name);
	}

	SimObject *findObject(uint32_t id) const
	{
		using func_t = SimObject*(SimGroup::*)(uint32_t) const;
		return call_virtual<27, func_t>(this, id);
	}

	using SimSet::addObject;

	SimObject *addObject(SimObject *object, uint32_t id)
	{
		object->id = id;
		return addObject(object);
	}

	SimObject *addObject(SimObject *object, const char *name)
	{
		addObject(object);
		assignName(object, name);
		return object;
	}
};

class SimManager : public SimGroup {
};
