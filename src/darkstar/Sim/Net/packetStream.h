#pragma once

#include "darkstar/Sim/simBase.h"
#include "util/meta.h"
#include <cstdint>

namespace Net {
class PacketStream;
}

class Net::PacketStream : public SimGroup {
public:
	uint32_t getAverageRTT() const
	{
		using func_t = to_static_function_t<decltype(&PacketStream::getAverageRTT)>;
		return ((func_t)0x519CC0)(this);
	}
};
