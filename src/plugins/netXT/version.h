#pragma once

#include "tribes/version.h"

namespace Netcode::XT {
// XT versions
constexpr auto v100 = NetcodeVersion{1, 2};
constexpr auto Latest = v100;

// Feature tests
constexpr auto Subtick = v100;
constexpr auto TracerInheritance = v100;
constexpr auto ClockSync = v100;
constexpr auto LagCompensation = v100;
constexpr auto PrefDamageFlash = v100;
constexpr auto ProjectileDataSendInheritance = v100;
constexpr auto ItemImageDataSendAccuFire = v100;
constexpr auto ClientPredictedProjectiles = v100;
}
