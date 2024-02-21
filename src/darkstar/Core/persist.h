#pragma once

#include "darkstar/Core/streamio.h"
#include <cstdint>

namespace Persistent {

class AbstractTaggedClass;

class Base {
};

class VersionedBase : public Base {
};

inline Base *create(uint32_t tag)
{
	using func_t = Base*(*)(uint32_t);
	return ((func_t)0x4131E0)(tag);
}

} // namespace Persistent
