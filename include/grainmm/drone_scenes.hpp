// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/instrument_parameters.hpp"
#include <array>
namespace grainmm {
struct DroneScene {
    const char *name, *description;
    DroneSynthParameters sound;
};
// Original GrainMM starting points. No samples, copied presets or extra storage.
const std::array<DroneScene, 8> &drone_scenes() noexcept;
// Control-thread transaction: changes only drone timbre/generation. Gain, rack,
// other sources and transport are deliberately preserved; browsing never applies.
InstrumentParameters apply_drone_scene(InstrumentParameters current, unsigned scene, unsigned seed);
} // namespace grainmm
