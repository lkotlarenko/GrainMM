// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/resonator.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace grainmm {
namespace {
constexpr double tau = 6.2831853071795864769;
float limit(float x, float lo, float hi, float fallback) noexcept {
    return std::isfinite(x) ? std::clamp(x, lo, hi) : fallback;
}
double clean(double x) noexcept {
    return std::isfinite(x) && std::abs(x) >= 1e-20 ? std::clamp(x, -16.0, 16.0) : 0;
}
void smooth(float &x, float goal, float a) noexcept {
    const float next = x + a * (goal - x);
    x = next == x || std::abs(next - goal) < 1e-6F ? goal : next;
}
bool same(const std::array<ResonantMode, 4> &a, const std::array<ResonantMode, 4> &b) noexcept {
    for (std::size_t i = 0; i < a.size(); ++i)
        if (a[i].a != b[i].a || a[i].radius2 != b[i].radius2 || a[i].gain != b[i].gain)
            return false;
    return true;
}
} // namespace
ResonatorParameters bounded(ResonatorParameters p) noexcept {
    p.note = limit(p.note, 36, 84, 57);
    p.decay_s = limit(p.decay_s, 0.05F, 3, 0.5F);
    p.brightness = limit(p.brightness, 0, 1, 0.5F);
    p.inharmonic = limit(p.inharmonic, 0, 1, 0);
    p.width = limit(p.width, 0, 1, 1);
    p.mix = limit(p.mix, 0, 1, 0);
    return p;
}
PreparedResonator prepare_resonator(ResonatorParameters p, int rate) {
    if (rate < 8000 || rate > 96000)
        throw std::invalid_argument("resonator rate must be 8000..96000 Hz");
    p = bounded(p);
    PreparedResonator result;
    result.values = p;
    const double base = 440 * std::exp2((static_cast<double>(p.note) - 69) / 12);
    double weights = 0;
    for (unsigned i = 0; i < result.modes.size(); ++i) {
        const double harmonic = i + 1;
        const double frequency = base * harmonic * (1 + p.inharmonic * 0.12 * i * i);
        if (frequency >= 0.45 * rate)
            continue;
        const double decay = p.decay_s / (1 + (1 - p.brightness) * static_cast<double>(i));
        const double radius = std::exp(-std::log(1000.0) / (decay * rate));
        const double weight = std::pow(static_cast<double>(0.15F + 0.85F * p.brightness), i);
        result.modes[i] = {2 * radius * std::cos(tau * frequency / rate), radius * radius,
                           weight * (1 - radius * radius) * 0.5};
        weights += weight;
    }
    for (auto &mode : result.modes)
        mode.gain /= weights; // Fundamental is always below Nyquist in the supported range.
    return result;
}
Resonator::Resonator(int rate)
    : target_(prepare_resonator({}, rate)), fade_frames_(static_cast<unsigned>(rate / 50)),
      smoothing_(1 - std::exp(-1 / (0.02F * static_cast<float>(rate)))) {
    for (auto &bank : banks_)
        bank.modes = target_.modes;
}
void Resonator::apply(const PreparedResonator &p) noexcept {
    for (const auto &m : p.modes)
        if (!std::isfinite(m.a) || !std::isfinite(m.radius2) || !std::isfinite(m.gain) ||
            m.radius2 < 0 || m.radius2 >= 1 || std::abs(m.a) >= 1 + m.radius2 || m.gain < 0 ||
            m.gain > (1 - m.radius2) * 0.5 + 1e-12) {
            target_.values.mix = 0;
            return;
        }
    const bool changed = !same(target_.modes, p.modes);
    target_ = p;
    target_.values = bounded(p.values);
    if (changed)
        pending_ = true;
}
void Resonator::transition() noexcept {
    auto &next = banks_[active_ ^ 1U];
    next = {};
    next.modes = target_.modes;
    remaining_ = fade_frames_;
    pending_ = false;
}
void Resonator::reset() noexcept {
    for (auto &bank : banks_) {
        bank = {};
        bank.modes = target_.modes;
    }
    remaining_ = 0;
    pending_ = false;
    dormant_ = true;
}
double Resonator::tick(Bank &bank, double x, unsigned channel) noexcept {
    const double difference = x - bank.x2[channel];
    bank.x2[channel] = bank.x1[channel];
    bank.x1[channel] = x;
    double wet = 0;
    for (std::size_t i = 0; i < bank.modes.size(); ++i) {
        const auto &m = bank.modes[i];
        auto &h = bank.history[i][channel];
        const double y = clean(m.a * h.y1 - m.radius2 * h.y2 + m.gain * difference);
        h.y2 = h.y1;
        h.y1 = y;
        wet += y;
    }
    return wet;
}
void Resonator::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    if (mix_ == 0 && target_.values.mix == 0) {
        if (!dormant_ || pending_)
            reset();
        return;
    }
    dormant_ = false;
    for (std::size_t frame = 0; frame < frames; ++frame) {
        if (pending_ && remaining_ == 0)
            transition();
        smooth(mix_, target_.values.mix, smoothing_);
        smooth(width_, target_.values.width, smoothing_);
        const double blend =
            remaining_ ? static_cast<double>(fade_frames_ - remaining_ + 1) / fade_frames_ : 0;
        std::array<float, 2> dry{}, wet{};
        for (unsigned ch = 0; ch < static_cast<unsigned>(channels); ++ch) {
            dry[ch] =
                static_cast<float>(clean(samples[frame * static_cast<unsigned>(channels) + ch]));
            const double current = tick(banks_[active_], dry[ch], ch);
            wet[ch] = static_cast<float>(
                remaining_ ? current + blend * (tick(banks_[active_ ^ 1U], dry[ch], ch) - current)
                           : current);
        }
        const float mid = channels == 2 ? (wet[0] + wet[1]) * 0.5F : wet[0];
        for (unsigned ch = 0; ch < static_cast<unsigned>(channels); ++ch)
            samples[frame * static_cast<unsigned>(channels) + ch] =
                dry[ch] + mix_ * (mid + width_ * (wet[ch] - mid) - dry[ch]);
        if (remaining_ && --remaining_ == 0)
            active_ ^= 1U;
    }
}
} // namespace grainmm
