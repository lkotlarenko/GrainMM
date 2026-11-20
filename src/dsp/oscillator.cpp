// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/oscillator.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace grainmm {
namespace {
constexpr double tau = 6.28318530717958647692;
float limit(float v, float low, float high, float fallback) noexcept {
    return std::isfinite(v) ? std::clamp(v, low, high) : fallback;
}
void smooth(float &value, float target, float step) noexcept {
    const float next = value + step * (target - value);
    value = next == value || std::abs(next - target) < 1e-6F ? target : next;
}
// Two-sample quadratic correction: zero value/slope at the support edges,
// opposite signed lobes around the discontinuity. No borrowed implementation.
float edge(double phase, double step) noexcept {
    if (phase < step) {
        const double u = 1 - phase / step;
        return static_cast<float>(-u * u);
    }
    if (phase > 1 - step) {
        const double u = 1 + (phase - 1) / step;
        return static_cast<float>(u * u);
    }
    return 0;
}
double wrap(double phase) noexcept { return phase - std::floor(phase); }
} // namespace
OscillatorParameters bounded(OscillatorParameters p) noexcept {
    p.level = limit(p.level, 0, 0.25F, 0);
    p.note = limit(p.note, 24, 96, 57);
    p.wave = std::round(limit(p.wave, 0, 3, 0));
    p.detune_cents = limit(p.detune_cents, 0, 30, 0);
    p.pulse_width = limit(p.pulse_width, 0.1F, 0.9F, 0.5F);
    p.vibrato_cents = limit(p.vibrato_cents, 0, 100, 0);
    return p;
}
SynthColorParameters bounded(SynthColorParameters p) noexcept {
    p.table_mix = limit(p.table_mix, 0, 1, 0);
    p.position = limit(p.position, 0, 3, 0);
    p.scan_hz = limit(p.scan_hz, 0, 2, 0);
    p.scan_depth = limit(p.scan_depth, 0, 1, 0);
    p.sub_level = limit(p.sub_level, 0, .25F, 0);
    p.sub_octave = std::round(limit(p.sub_octave, 1, 2, 1));
    return p;
}
PreparedOscillator prepare_oscillator(OscillatorParameters p, int rate,
                                      SynthColorParameters color) {
    if (rate < 8000 || rate > 96000)
        throw std::invalid_argument("oscillator rate must be 8000..96000 Hz");
    p = bounded(p);
    const double hz = 440 * std::exp2((static_cast<double>(p.note) - 69) / 12);
    const double detune = std::exp2(static_cast<double>(p.detune_cents) / 2400);
    return {p, std::clamp(hz / detune / rate, 0.00001, 0.4),
            std::clamp(hz * detune / rate, 0.00001, 0.4), std::exp2(p.vibrato_cents / 1200) - 1,
            bounded(color)};
}
Oscillator::Oscillator(int rate)
    : tables_(tables()), current_(prepare_oscillator({}, rate)), target_(current_),
      vibrato_step_(5.0 / rate),
      smoothing_(1 - std::exp(-1.0F / (0.02F * static_cast<float>(rate)))) {
    for (std::size_t i = 0; i < sine_.size(); ++i)
        sine_[i] = static_cast<float>(std::sin(tau * static_cast<double>(i) / 1024));
    sine_.back() = sine_.front();
}
void Oscillator::apply(const PreparedOscillator &p) noexcept {
    target_ = p;
    target_.values = bounded(p.values);
    target_.color = bounded(p.color);
    if (!std::isfinite(target_.left_step) || !std::isfinite(target_.right_step) ||
        target_.left_step <= 0 || target_.right_step <= 0) {
        target_.values.level = 0;
        target_.left_step = target_.right_step = 0.01;
    }
    target_.left_step = std::clamp(target_.left_step, 0.00001, 0.4);
    target_.right_step = std::clamp(target_.right_step, 0.00001, 0.4);
    target_.vibrato_amount = limit(p.vibrato_amount, 0, 0.06F, 0);
}
void Oscillator::reset() noexcept {
    left_phase_ = right_phase_ = vibrato_phase_ = 0;
    gain_ = sub_gain_ = 0;
    sub_phase_ = scan_phase_ = 0;
}
float Oscillator::sine(double phase) const noexcept {
    if (phase < 0 || phase >= 1)
        phase = wrap(phase);
    const double at = phase * 1024;
    const auto i = static_cast<std::size_t>(at);
    return sine_[i] + (sine_[i + 1] - sine_[i]) * static_cast<float>(at - static_cast<double>(i));
}
float Oscillator::waveform(unsigned kind, double phase, double step, float width) const noexcept {
    if (kind == 0)
        return sine(phase);
    if (kind == 1) {
        float sum = 0;
        for (unsigned harmonic = 1; harmonic <= 15 && harmonic * step < 0.45; harmonic += 2)
            sum += (harmonic % 4 == 1 ? 1.0F : -1.0F) * sine(phase * harmonic) /
                   static_cast<float>(harmonic * harmonic);
        return sum * 0.810569469F;
    }
    if (kind == 2)
        return static_cast<float>(2 * phase - 1) - edge(phase, step);
    const float pulse =
        (phase < width ? 1.0F : -1.0F) + edge(phase, step) - edge(wrap(phase - width), step);
    return (pulse - (2 * width - 1)) / (2 * std::max(width, 1 - width));
}
const Oscillator::Tables &Oscillator::tables() {
    static const Tables result = [] {
        Tables bank{};
        // Four original spectra: pure, hollow, bright, glass. Each band adds
        // one partial; normalization is shared across bands to avoid gain jumps.
        constexpr float spectra[4][8] = {{1, 0, 0, 0, 0, 0, 0, 0},
                                         {1, 0, .45F, 0, .2F, 0, .1F, 0},
                                         {1, .5F, .33F, .25F, .2F, .16F, .14F, .12F},
                                         {1, .1F, .45F, .05F, .3F, .05F, .2F, .1F}};
        for (unsigned shape = 0; shape < 4; ++shape) {
            float norm = 0;
            for (float weight : spectra[shape])
                norm += weight;
            for (unsigned band = 0; band < 8; ++band)
                for (unsigned i = 0; i <= 256; ++i) {
                    float value = 0;
                    for (unsigned h = 0; h <= band; ++h)
                        value += spectra[shape][h] *
                                 static_cast<float>(std::sin(tau * i * (h + 1) / 256.0));
                    bank[shape][band][i] = value / norm;
                }
            for (auto &band : bank[shape])
                band.back() = band.front();
        }
        return bank;
    }();
    return result;
}
float Oscillator::table(double phase, double step, float position) const noexcept {
    const auto shape = std::min(2U, static_cast<unsigned>(position));
    const float morph = position - static_cast<float>(shape);
    // Keep every interpolated partial below .45 cycles/frame. Fade bands as
    // pitch changes rather than switching bright tables at a hard threshold.
    const double partials = std::clamp(.45 / step - 1, 1.0, 8.0);
    const auto band = std::min(6U, static_cast<unsigned>(partials) - 1);
    const float blend = static_cast<float>(partials - (band + 1));
    const double at = phase * 256;
    const auto i = static_cast<unsigned>(at);
    const float fraction = static_cast<float>(at - i);
    const auto lookup = [&](unsigned s, unsigned b) {
        const auto &wave = tables_[s][b];
        return wave[i] + fraction * (wave[i + 1] - wave[i]);
    };
    const float a = lookup(shape, band) + blend * (lookup(shape, band + 1) - lookup(shape, band));
    const float b =
        lookup(shape + 1, band) + blend * (lookup(shape + 1, band + 1) - lookup(shape + 1, band));
    return a + morph * (b - a);
}
void Oscillator::add(float *samples, std::size_t frames, int channels, bool playing) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    const float goal = playing ? target_.values.level : 0;
    const float sub_goal = playing ? target_.color.sub_level : 0;
    if (goal == 0 && gain_ == 0 && sub_goal == 0 && sub_gain_ == 0)
        return;
    for (std::size_t frame = 0; frame < frames; ++frame) {
        smooth(gain_, goal, smoothing_);
        smooth(sub_gain_, sub_goal, smoothing_);
        smooth(current_.color.table_mix, target_.color.table_mix, smoothing_);
        smooth(current_.color.position, target_.color.position, smoothing_);
        smooth(current_.color.scan_hz, target_.color.scan_hz, smoothing_);
        smooth(current_.color.scan_depth, target_.color.scan_depth, smoothing_);
        current_.left_step += smoothing_ * (target_.left_step - current_.left_step);
        current_.right_step += smoothing_ * (target_.right_step - current_.right_step);
        smooth(current_.values.pulse_width, target_.values.pulse_width, smoothing_);
        smooth(current_.vibrato_amount, target_.vibrato_amount, smoothing_);
        for (unsigned i = 0; i < 4; ++i)
            smooth(weights_[i], target_.values.wave == static_cast<float>(i) ? 1.0F : 0.0F,
                   smoothing_);
        const double modulation =
            current_.vibrato_amount > 0 ? 1 + current_.vibrato_amount * sine(vibrato_phase_) : 1;
        const double left_step = std::min(0.45, current_.left_step * modulation);
        const double right_step = std::min(0.45, current_.right_step * modulation);
        float left = 0, right = 0;
        for (unsigned i = 0; i < 4; ++i)
            if (weights_[i] > 0) {
                left +=
                    weights_[i] * waveform(i, left_phase_, left_step, current_.values.pulse_width);
                right += weights_[i] *
                         waveform(i, right_phase_, right_step, current_.values.pulse_width);
            }
        if (current_.color.table_mix > 0) {
            const float position = std::clamp(current_.color.position +
                                                  3 * current_.color.scan_depth * sine(scan_phase_),
                                              0.0F, 3.0F);
            left += current_.color.table_mix * (table(left_phase_, left_step, position) - left);
            right += current_.color.table_mix * (table(right_phase_, right_step, position) - right);
        }
        left = std::clamp(left, -1.0F, 1.0F) * gain_;
        right = std::clamp(right, -1.0F, 1.0F) * gain_;
        const double desired_sub =
            (left_step + right_step) * .5 / (target_.color.sub_octave == 2 ? 4 : 2);
        sub_step_ += smoothing_ * (desired_sub - sub_step_);
        const float sub = sine(sub_phase_) * sub_gain_;
        left += sub;
        right += sub;
        sub_phase_ = wrap(sub_phase_ + sub_step_);
        // vibrato_step_ is 5/rate, keeping scan rate independent of block size.
        scan_phase_ = wrap(scan_phase_ + current_.color.scan_hz * vibrato_step_ / 5);
        if (channels == 1)
            samples[frame] += (left + right) * 0.5F;
        else {
            samples[frame * 2] += left;
            samples[frame * 2 + 1] += right;
        }
        left_phase_ += left_step;
        if (left_phase_ >= 1)
            left_phase_ -= 1;
        right_phase_ += right_step;
        if (right_phase_ >= 1)
            right_phase_ -= 1;
        vibrato_phase_ += vibrato_step_;
        if (vibrato_phase_ >= 1)
            vibrato_phase_ -= 1;
    }
}
} // namespace grainmm
