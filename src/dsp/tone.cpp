// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/tone.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace grainmm {
namespace {
constexpr double tau = 6.2831853071795864769;
constexpr double peak = 0.04;
} // namespace
Tone::Tone(int sample_rate) {
    if (sample_rate < 8000 || sample_rate > 192000)
        throw std::invalid_argument("unsupported sample rate");
    phase_step_ = tau * 220.0 / sample_rate;
    gain_step_ = peak / (0.02 * sample_rate);
}
void Tone::render(std::int16_t *out, std::size_t frames, int channels, bool audible) noexcept {
    if (!out || (channels != 1 && channels != 2))
        return;
    const double target = audible ? peak : 0.0;
    for (std::size_t frame = 0; frame < frames; ++frame) {
        gain_ += std::clamp(target - gain_, -gain_step_, gain_step_);
        const auto value = static_cast<std::int16_t>(std::sin(phase_) * gain_ * 32767.0);
        for (int channel = 0; channel < channels; ++channel)
            out[frame * static_cast<std::size_t>(channels) + static_cast<std::size_t>(channel)] =
                value;
        phase_ += phase_step_;
        if (phase_ >= tau)
            phase_ -= tau;
    }
}
} // namespace grainmm
