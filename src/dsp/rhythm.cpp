// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/rhythm.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace grainmm {
namespace {
float limit(float x, float lo, float hi, float fallback) noexcept {
    return std::isfinite(x) ? std::clamp(x, lo, hi) : fallback;
}
void smooth(float &x, float goal, float a) noexcept {
    const float next = x + a * (goal - x);
    x = next == x || std::abs(next - goal) < 1e-7F ? goal : next;
}
} // namespace
RhythmParameters bounded(RhythmParameters p) noexcept {
    p.bpm = limit(p.bpm, 30, 240, 120);
    p.division = std::round(limit(p.division, 0, 3, 1));
    p.tremolo = limit(p.tremolo, 0, 1, 0);
    p.pan = limit(p.pan, 0, 1, 0);
    p.shape = std::round(limit(p.shape, 0, 1, 0));
    p.stereo_degrees = limit(p.stereo_degrees, 0, 180, 0);
    return p;
}
PreparedRhythm prepare_rhythm(RhythmParameters p, int rate) {
    if (rate < 8000 || rate > 96000)
        throw std::invalid_argument("rhythm rate must be 8000..96000 Hz");
    p = bounded(p);
    constexpr std::array<float, 4> divisions{0.5F, 1, 2, 4};
    return {p, p.bpm / 60 * divisions[static_cast<std::size_t>(p.division)] /
                   static_cast<float>(rate)};
}
Rhythm::Rhythm(int rate)
    : target_(prepare_rhythm({}, rate)),
      smoothing_(1 - std::exp(-1 / (0.02F * static_cast<float>(rate)))), step_(target_.step) {
    for (std::size_t i = 0; i < sine_.size(); ++i)
        sine_[i] = std::sin(6.28318530718F * static_cast<float>(i) / 256);
    sine_.back() = sine_.front();
}
void Rhythm::apply(const PreparedRhythm &p) noexcept {
    target_ = p;
    target_.values = bounded(p.values);
    target_.step = limit(p.step, 0.000001F, 0.002F, step_);
}
float Rhythm::wave(double phase) const noexcept {
    if (phase >= 1)
        phase -= 1;
    const double at = phase * 256;
    const auto i = static_cast<std::size_t>(at);
    const float sine =
        sine_[i] + (sine_[i + 1] - sine_[i]) * static_cast<float>(at - static_cast<double>(i));
    // Triangle aligned to sine's zero crossing, continuous at wrap.
    const float triangle = static_cast<float>(phase < 0.25   ? 4 * phase
                                              : phase < 0.75 ? 2 - 4 * phase
                                                             : 4 * phase - 4);
    return sine + shape_ * (triangle - sine);
}
void Rhythm::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    if (tremolo_ == 0 && pan_ == 0 && target_.values.tremolo == 0 && target_.values.pan == 0)
        return;
    for (std::size_t frame = 0; frame < frames; ++frame) {
        smooth(step_, target_.step, smoothing_);
        smooth(tremolo_, target_.values.tremolo, smoothing_);
        smooth(pan_, target_.values.pan, smoothing_);
        smooth(shape_, target_.values.shape, smoothing_);
        smooth(offset_, target_.values.stereo_degrees / 360, smoothing_);
        const float left = wave(phase_), right = wave(phase_ + offset_);
        const float pan = left * pan_;
        const float left_gain = (1 - tremolo_ * (0.5F + 0.5F * left)) * (1 - std::max(0.0F, pan));
        const float right_gain = (1 - tremolo_ * (0.5F + 0.5F * right)) * (1 + std::min(0.0F, pan));
        for (unsigned ch = 0; ch < static_cast<unsigned>(channels); ++ch) {
            auto &x = samples[frame * static_cast<unsigned>(channels) + ch];
            x = std::isfinite(x) ? x * (channels == 1 ? (left_gain + right_gain) * 0.5F
                                        : ch          ? right_gain
                                                      : left_gain)
                                 : 0;
        }
        phase_ += step_;
        if (phase_ >= 1)
            phase_ -= 1;
    }
}
} // namespace grainmm
