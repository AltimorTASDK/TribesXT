#pragma once

#include "util/meta.h"
#include "util/struct.h"
#include <cstdint>

class Random {
public:
	uint32_t x[5];

	Random(uint32_t seed)
	{
		setSeed(seed);
	}

	void setSeed(uint32_t seed)
	{
		using func_t = to_static_function_t<decltype(&Random::setSeed)>;
		((func_t)0x44A820)(this, seed);
	}

	float getFloat()
	{
		using func_t = float(__thiscall*)(Random*);
		return ((func_t)0x4297C0)(this);
	}
};
