#pragma once

#include <compare>
#include <cstdint>

struct NetcodeVersion {
	uint8_t major;
	uint8_t minor;

	constexpr uint16_t get() const
	{
		return (major << 8) | minor;
	}

	constexpr auto operator<=>(const NetcodeVersion &other) const
	{
		return get() <=> other.get();
	}
};

namespace Netcode {
constexpr auto Old = NetcodeVersion(1, 0);
constexpr auto New = NetcodeVersion(1, 1); // 1.40
}

inline auto &serverNetcodeVersion = *(NetcodeVersion*)0x6D0FF8;
inline auto &ourNetcodeVersion    = *(NetcodeVersion*)0x6D0FFC;