#pragma once

#include "util/vector.h"

struct ColorF : color_rgb_f32 {
	using color_rgb_f32::color_rgb_f32;
	constexpr ColorF(const color_rgb_f32 &color) : color_rgb_f32(color) {}
};

struct GFXColorInfoF {
	float haze;
	ColorF color;
	float alpha;
};
