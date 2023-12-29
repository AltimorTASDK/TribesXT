#pragma once

#include <bit>
#include <cstdint>

constexpr auto TickMs    = 32u;
constexpr auto TickSecs  = TickMs / 1000.f;
constexpr auto TickRate  = 1000.f / TickMs;
constexpr auto TickShift = std::bit_width(TickMs - 1);
constexpr auto TickMask  = ~(TickMs - 1);

constexpr uint32_t msToTicks(uint32_t ms)
{
	return ms >> TickShift;
}

constexpr uint32_t msToTicksRoundUp(uint32_t ms)
{
	return msToTicks(ms + TickMs - 1);
}

constexpr uint32_t roundMsDownToTick(uint32_t ms)
{
	return ms & TickMask;
}

constexpr uint32_t roundMsUpToTick(uint32_t ms)
{
	return roundMsDownToTick(ms + TickMs - 1);
}

constexpr uint32_t ticksToMs(uint32_t ticks)
{
	return ticks << TickShift;
}

constexpr float msToSecs(uint32_t ms)
{
	return ms * 0.001f;
}
