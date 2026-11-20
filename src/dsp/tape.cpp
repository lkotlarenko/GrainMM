// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/tape.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace grainmm {
namespace {
constexpr float pi = 3.14159265358979323846F;
float limit(float x, float lo, float hi, float fallback) noexcept {
    return std::isfinite(x) ? std::clamp(x, lo, hi) : fallback;
}
void smooth(float &x, float goal, float a) noexcept {
    const float next = x + a * (goal - x);
    x = next == x || std::abs(next - goal) < 1e-6F ? goal : next;
}
float saturate(float x) noexcept {
    // Odd, bounded soft clipping with unity small-signal slope.
    return x / (1 + std::abs(x));
}
} // namespace
TapeParameters bounded(TapeParameters p) noexcept {
    p.drive_db = limit(p.drive_db, 0, 24, 0);
    p.tone_hz = limit(p.tone_hz, 500, 16000, 12000);
    p.wow_ms = limit(p.wow_ms, 0, 2, 0);
    p.flutter_ms = limit(p.flutter_ms, 0, 0.25F, 0);
    p.mix = limit(p.mix, 0, 1, 0);
    p.output_db = limit(p.output_db, -18, 6, 0);
    return p;
}
PreparedTape prepare_tape(TapeParameters p, int rate) {
    if (rate < 8000 || rate > 96000)
        throw std::invalid_argument("tape rate must be 8000..96000 Hz");
    p = bounded(p);
    return {p, std::pow(10.0F, p.drive_db / 20),
            1 - std::exp(-2 * pi * std::min(p.tone_hz, 0.45F * static_cast<float>(rate)) /
                         static_cast<float>(rate)),
            std::pow(10.0F, p.output_db / 20)};
}
Tape::Tape(int rate)
    : target_(prepare_tape({}, rate)), rate_(static_cast<float>(rate)),
      smoothing_(1 - std::exp(-1 / (0.03F * rate_))) {
    for (std::size_t i = 0; i < sine_.size(); ++i)
        sine_[i] = std::sin(2 * pi * static_cast<float>(i) / 256);
    sine_.back() = sine_.front();
    tone_ = target_.tone;
}
void Tape::apply(const PreparedTape &p) noexcept {
    target_ = p;
    target_.values = bounded(p.values);
    target_.drive = limit(p.drive, 1, 16, 1);
    target_.tone = limit(p.tone, 0.001F, 1, 1);
    target_.output = limit(p.output, 0.125F, 2, 1);
}
void Tape::reset() noexcept {
    for (auto &channel : history_)
        channel.fill(0);
    lowpass_.fill(0);
    write_ = 0;
    slow_phase_ = fast_phase_ = 0;
    dormant_ = true;
}
float Tape::modulation(float phase) const noexcept {
    const float position = phase * 256;
    const auto i = static_cast<std::size_t>(position);
    return sine_[i] + (sine_[i + 1] - sine_[i]) * (position - static_cast<float>(i));
}
void Tape::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    if (mix_ == 0 && target_.values.mix == 0) {
        if (!dormant_)
            reset();
        return;
    }
    dormant_ = false;
    for (std::size_t frame = 0; frame < frames; ++frame) {
        smooth(mix_, target_.values.mix, smoothing_);
        smooth(drive_, target_.drive, smoothing_);
        smooth(tone_, target_.tone, smoothing_);
        smooth(output_, target_.output, smoothing_);
        smooth(wow_, target_.values.wow_ms, smoothing_);
        smooth(flutter_, target_.values.flutter_ms, smoothing_);
        const float delay =
            rate_ * 0.001F *
            (3 + wow_ * modulation(slow_phase_) + flutter_ * modulation(fast_phase_));
        // Split the positive delay before wrapping with integer arithmetic.
        // Adding a tiny negative float position to 1024 can round to 1024.
        const auto whole = static_cast<std::size_t>(delay);
        const auto index = (write_ + 1024 - whole) % 1024;
        const auto previous = (index + 1023) % 1024;
        const float fraction = delay - static_cast<float>(whole);
        for (unsigned ch = 0; ch < static_cast<unsigned>(channels); ++ch) {
            const auto at = frame * static_cast<unsigned>(channels) + ch;
            const float dry = limit(samples[at], -8, 8, 0);
            const float colored = saturate(dry * drive_);
            lowpass_[ch] += tone_ * (colored - lowpass_[ch]);
            if (std::abs(lowpass_[ch]) < 1e-20F)
                lowpass_[ch] = 0;
            history_[ch][write_] = lowpass_[ch];
            const float wet =
                history_[ch][index] + fraction * (history_[ch][previous] - history_[ch][index]);
            samples[at] = dry + mix_ * (wet * output_ - dry);
        }
        write_ = (write_ + 1) % 1024;
        slow_phase_ += 0.6F / rate_;
        fast_phase_ += 6 / rate_;
        if (slow_phase_ >= 1)
            slow_phase_ -= 1;
        if (fast_phase_ >= 1)
            fast_phase_ -= 1;
    }
}
} // namespace grainmm
