// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/delay.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace grainmm {
namespace {
float limit(float value, float low, float high, float fallback) noexcept {
    return std::isfinite(value) ? std::clamp(value, low, high) : fallback;
}
float finite_input(float value) noexcept {
    return std::isfinite(value) ? std::clamp(value, -4.0F, 4.0F) : 0;
}
void approach(float &value, float target, float amount) noexcept {
    value += (target - value) * amount;
}
} // namespace
DelayParameters bounded(DelayParameters p) noexcept {
    p.time_ms = limit(p.time_ms, 10, 2000, 450);
    p.feedback = limit(p.feedback, 0, 0.95F, 0);
    p.mix = limit(p.mix, 0, 1, 0);
    p.ping_pong = limit(p.ping_pong, 0, 1, 0);
    p.spread = limit(p.spread, 0, 1, 1);
    p.lowpass_hz = limit(p.lowpass_hz, 500, 20000, 9000);
    return p;
}
Delay::Delay(int rate, DelayParameters initial) : rate_(rate) {
    if (rate < 8000 || rate > 96000)
        throw std::invalid_argument("delay rate must be 8000..96000 Hz");
    ring_.resize(static_cast<std::size_t>(rate) * 2 + 2);
    current_ = target_ = bounded(initial);
    smoothing_ = 1 - std::exp(-1.0F / (0.02F * static_cast<float>(rate)));
    lowpass_ = target_lowpass_ = coefficient(current_.lowpass_hz);
}
float Delay::coefficient(float hz) const noexcept {
    // Bypass feedback filtering near Nyquist; lower cutoffs use a stable one-pole.
    if (hz >= 0.45F * static_cast<float>(rate_))
        return 1;
    return 1 - std::exp(-6.28318530718F * hz / static_cast<float>(rate_));
}
void Delay::set_parameters(DelayParameters parameters) noexcept {
    target_ = bounded(parameters);
    target_lowpass_ = coefficient(target_.lowpass_hz);
}
void Delay::reset() noexcept {
    write_ = valid_ = 0;
    filtered_ = {};
    // Keep smoothed controls; only the delay history is cleared.
}
float Delay::read(std::size_t age, unsigned channel) const noexcept {
    if (age == 0 || age > valid_)
        return 0;
    const auto index = write_ >= age ? write_ - age : write_ + ring_.size() - age;
    return ring_[index][channel];
}
void Delay::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    for (std::size_t frame = 0; frame < frames; ++frame) {
        approach(current_.time_ms, target_.time_ms, smoothing_);
        approach(current_.feedback, target_.feedback, smoothing_);
        approach(current_.mix, target_.mix, smoothing_);
        approach(current_.ping_pong, target_.ping_pong, smoothing_);
        approach(current_.spread, target_.spread, smoothing_);
        approach(lowpass_, target_lowpass_, smoothing_);
        const float age = current_.time_ms * (static_cast<float>(rate_) * 0.001F);
        const auto whole = static_cast<std::size_t>(age);
        const float fraction = age - static_cast<float>(whole);
        const float left = finite_input(samples[frame * static_cast<unsigned>(channels)]);
        const float right = channels == 2 ? finite_input(samples[frame * 2 + 1]) : left;
        std::array<float, 2> wet{};
        for (unsigned channel = 0; channel < 2; ++channel) {
            const float a = read(whole, channel);
            wet[channel] = a + (read(whole + 1, channel) - a) * fraction;
            filtered_[channel] += lowpass_ * (wet[channel] - filtered_[channel]);
            // Kill subnormal feedback once inaudible on CPUs without flush-to-zero.
            if (std::abs(filtered_[channel]) < 1e-20F)
                filtered_[channel] = 0;
        }
        const float cross = current_.ping_pong;
        ring_[write_][0] = finite_input(
            left + current_.feedback * (filtered_[0] + cross * (filtered_[1] - filtered_[0])));
        ring_[write_][1] = finite_input(
            right + current_.feedback * (filtered_[1] + cross * (filtered_[0] - filtered_[1])));
        if (++write_ == ring_.size())
            write_ = 0;
        valid_ = std::min(valid_ + 1, ring_.size() - 1);
        const float middle = (wet[0] + wet[1]) * 0.5F;
        const float wet_left = middle + (wet[0] - middle) * current_.spread;
        const float wet_right = middle + (wet[1] - middle) * current_.spread;
        const float dry = 1 - current_.mix;
        samples[frame * static_cast<unsigned>(channels)] =
            left * dry + (channels == 1 ? middle : wet_left) * current_.mix;
        if (channels == 2)
            samples[frame * 2 + 1] = right * dry + wet_right * current_.mix;
    }
}
} // namespace grainmm
