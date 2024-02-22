#pragma once

#include <compare>
#include <cstdint>

inline auto &serverNetcodeVersion = *(struct NetcodeVersion*)0x6D0FF8;
inline auto &ourNetcodeVersion    = *(struct NetcodeVersion*)0x6D0FFC;

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

	bool check() const
	{
		return serverNetcodeVersion >= *this;
	}
};

namespace Netcode {
constexpr auto Old = NetcodeVersion(1, 0);
constexpr auto New = NetcodeVersion(1, 1); // 1.40
}