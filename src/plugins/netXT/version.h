#pragma once

#include "tribes/version.h"

namespace Netcode::XT {
// XT versions
constexpr auto v100 = NetcodeVersion{1, 2};
constexpr auto Latest = v100;

// Feature tests
constexpr auto Subtick = v100;
}
