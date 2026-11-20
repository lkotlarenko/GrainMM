// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace grainmm {
struct HarmonyParameters {
    // Chord: off/octaves/fifth/major/minor/sus4. Order: up/down/bounce/random.
    float transpose = 0, chord = 0, order = 0, octaves = 1, chance = 1, detune_cents = 0;
};
HarmonyParameters bounded(HarmonyParameters p) noexcept;
struct PreparedHarmony {
    HarmonyParameters values;
    std::array<float, 9> ratios{};
    unsigned count = 1;
    float root = 1, detune_low = 1, detune_high = 1;
};
// Exponential pitch design stays on the control thread.
PreparedHarmony prepare_harmony(HarmonyParameters p) noexcept;
// Called once per successful grain spawn, never per sample. No new grain voices.
class GrainHarmony {
  public:
    GrainHarmony() noexcept;
    void apply(const PreparedHarmony &p) noexcept;
    float next() noexcept;
    void reset() noexcept;

  private:
    float random_unit() noexcept;
    PreparedHarmony prepared_;
    unsigned position_ = 0;
    std::uint32_t random_ = 0x6d2b79f5U;
};
} // namespace grainmm
