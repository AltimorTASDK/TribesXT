#pragma once

constexpr auto TICK_MS = 32u;
constexpr auto TICK_SECS = TICK_MS / 1000.f;
constexpr auto TICK_RATE = 1000.f / TICK_MS;

constexpr uint32_t msToTicks(uint32_t ms)
{
	return ms / TICK_MS;
}

constexpr uint32_t msToTicksRoundUp(uint32_t ms)
{
	return (ms + TICK_MS - 1) / TICK_MS;
}

constexpr uint32_t roundMsDownToTick(uint32_t ms)
{
	return ms - (ms % TICK_MS);
}

constexpr uint32_t roundMsUpToTick(uint32_t ms)
{
	return roundMsDownToTick(ms + TICK_MS - 1);
}
