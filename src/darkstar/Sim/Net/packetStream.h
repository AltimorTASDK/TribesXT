#pragma once

#include "darkstar/Sim/simBase.h"
#include "util/meta.h"
#include "util/struct.h"
#include <cstdint>

namespace Net {
class PacketStream;
}

class Net::PacketStream : public SimGroup {
public:
	enum Mode {
		NormalMode,   // standard client mode
		ReplyMode,    // standard server mode (reply to client packets)
		RecordMode,   // client+recording
		PlaybackMode, // client only playback recording
		ServerMode,   // special tribes mode
	};

	FIELD(0x9F4, Mode, streamMode);

	uint32_t getAverageRTT() const
	{
		using func_t = to_static_function_t<decltype(&PacketStream::getAverageRTT)>;
		return ((func_t)0x519CC0)(this);
	}
};
