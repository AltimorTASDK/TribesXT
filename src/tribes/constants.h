#pragma once

constexpr auto TickMs   = 32u;
constexpr auto TickSecs = TickMs / 1000.f;
constexpr auto TickRate = 1000.f / TickMs;

constexpr uint32_t msToTicks(uint32_t ms)
{
	return ms / TickMs;
}

constexpr uint32_t msToTicksRoundUp(uint32_t ms)
{
	return (ms + TickMs - 1) / TickMs;
}

constexpr uint32_t roundMsDownToTick(uint32_t ms)
{
	return ms - (ms % TickMs);
}

constexpr uint32_t roundMsUpToTick(uint32_t ms)
{
	return roundMsDownToTick(ms + TickMs - 1);
}
