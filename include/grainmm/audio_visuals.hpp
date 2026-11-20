// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
namespace grainmm {
// Value-only callback telemetry. UI never reads mutable DSP state.
struct AudioVisuals {
    unsigned peaks = 0, grain = 0, hits = 0;
    std::array<unsigned, 6> balls{};
    unsigned drone = 0, drone_voices = 0, drone_events = 0, drone_count = 0, output_reduction = 0;
};
} // namespace grainmm
