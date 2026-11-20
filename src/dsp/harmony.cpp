// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/harmony.hpp"
#include <algorithm>
#include <cmath>
namespace grainmm {
namespace {
float limit(float x, float low, float high, float fallback) noexcept {
    return std::isfinite(x) ? std::clamp(x, low, high) : fallback;
}
} // namespace
HarmonyParameters bounded(HarmonyParameters p) noexcept {
    p.transpose = std::round(limit(p.transpose, -24, 24, 0));
    p.chord = std::round(limit(p.chord, 0, 5, 0));
    p.order = std::round(limit(p.order, 0, 3, 0));
    p.octaves = std::round(limit(p.octaves, 1, 3, 1));
    p.chance = limit(p.chance, 0, 1, 1);
    p.detune_cents = limit(p.detune_cents, 0, 50, 0);
    return p;
}
PreparedHarmony prepare_harmony(HarmonyParameters p) noexcept {
    p = bounded(p);
    PreparedHarmony result;
    result.values = p;
    result.root = std::exp2(p.transpose / 12);
    result.ratios.fill(result.root);
    constexpr std::array<std::array<int, 3>, 6> intervals{
        {{0, 0, 0}, {0, 0, 0}, {0, 7, 0}, {0, 4, 7}, {0, 3, 7}, {0, 5, 7}}};
    const auto chord = static_cast<unsigned>(p.chord);
    const unsigned notes = chord < 2 ? 1U : chord == 2 ? 2U : 3U;
    const auto octaves = chord == 0 ? 1U : static_cast<unsigned>(p.octaves);
    result.count = notes * octaves;
    for (unsigned octave = 0; octave < octaves; ++octave)
        for (unsigned note = 0; note < notes; ++note)
            result.ratios[octave * notes + note] = std::exp2(
                (p.transpose + static_cast<float>(intervals[chord][note] + 12 * octave)) / 12);
    result.detune_low = std::exp2(-p.detune_cents / 1200);
    result.detune_high = std::exp2(p.detune_cents / 1200);
    return result;
}
GrainHarmony::GrainHarmony() noexcept : prepared_(prepare_harmony({})) {}
void GrainHarmony::apply(const PreparedHarmony &p) noexcept {
    const auto before = prepared_.values;
    prepared_ = p;
    prepared_.values = bounded(p.values);
    prepared_.count = std::clamp(p.count, 1U, 9U);
    prepared_.root = limit(p.root, 0.25F, 4, 1);
    for (auto &ratio : prepared_.ratios)
        ratio = limit(ratio, 0.25F, 32, prepared_.root);
    prepared_.detune_low = limit(p.detune_low, 0.97F, 1, 1);
    prepared_.detune_high = limit(p.detune_high, 1, 1.03F, 1);
    if (before.chord != prepared_.values.chord || before.order != prepared_.values.order ||
        before.octaves != prepared_.values.octaves)
        position_ = 0;
}
void GrainHarmony::reset() noexcept {
    position_ = 0;
    random_ = 0x6d2b79f5U;
}
float GrainHarmony::random_unit() noexcept {
    random_ ^= random_ << 13;
    random_ ^= random_ >> 17;
    random_ ^= random_ << 5;
    return static_cast<float>(random_ >> 8) / 16777216.0F;
}
float GrainHarmony::next() noexcept {
    const auto count = prepared_.count;
    unsigned index = position_ % count;
    const auto order = static_cast<unsigned>(prepared_.values.order);
    const unsigned period = order == 2 && count > 1 ? 2 * (count - 1) : count;
    if (order == 1)
        index = count - 1 - index;
    else if (order == 2)
        index = position_ < count ? position_ : period - position_;
    else if (order == 3)
        index = static_cast<unsigned>(random_unit() * static_cast<float>(count));
    position_ = (position_ + 1) % period;
    float ratio = prepared_.ratios[index];
    if (prepared_.values.chance < 1 && random_unit() >= prepared_.values.chance)
        ratio = prepared_.root;
    if (prepared_.values.detune_cents > 0)
        ratio *=
            prepared_.detune_low + random_unit() * (prepared_.detune_high - prepared_.detune_low);
    return ratio;
}
} // namespace grainmm
