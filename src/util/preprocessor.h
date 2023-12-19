#pragma once

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)

#define FORCE_SEMICOLON static_assert(true)
