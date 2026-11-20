// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/equalizer.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace grainmm {
namespace {
constexpr double pi = 3.14159265358979323846;
void check_rate(int rate) {
    if (rate < 8000 || rate > 96000)
        throw std::invalid_argument("equalizer rate must be 8000..96000 Hz");
}
float limit(float value, float low, float high, float fallback) noexcept {
    return std::isfinite(value) ? std::clamp(value, low, high) : fallback;
}
bool valid(const BiquadCoefficients &c) noexcept {
    // Schur stability of z^2+a1*z+a2. Double precision retains the
    // low-frequency/high-Q response when poles approach unity.
    const double a1 = c.a1, a2 = c.a2;
    return std::isfinite(c.b0) && std::isfinite(c.b1) && std::isfinite(c.b2) && std::isfinite(a1) &&
           std::isfinite(a2) && std::abs(a2) < 1 && 1 + a1 + a2 > 0 && 1 - a1 + a2 > 0;
}
BiquadCoefficients design(EqBand band, int rate) {
    const double frequency =
        limit(band.frequency, 25, std::min(18000.0F, 0.45F * static_cast<float>(rate)), 1000);
    const double q = limit(band.q, 0.35F, 5, 0.9F);
    const double gain = limit(band.gain_db, -20, 20, 0);
    if (band.kind != EqKind::Bell && band.kind != EqKind::LowShelf &&
        band.kind != EqKind::HighShelf && band.kind != EqKind::LowPass &&
        band.kind != EqKind::HighPass)
        throw std::invalid_argument("unsupported equalizer band kind");
    if (gain == 0 && band.kind != EqKind::LowPass && band.kind != EqKind::HighPass)
        return {};
    // Independently implemented equations described by Robert Bristow-Johnson:
    // https://www.w3.org/TR/audio-eq-cookbook/ (W3C Working Group Note,2021-06-08).
    const double omega = 2 * pi * frequency / rate;
    const double cosine = std::cos(omega), alpha = std::sin(omega) / (2 * q);
    const double amplitude = std::pow(10.0, gain / 40.0);
    double b0, b1, b2, a0, a1, a2;
    switch (band.kind) {
    case EqKind::Bell:
        b0 = 1 + alpha * amplitude;
        b1 = -2 * cosine;
        b2 = 1 - alpha * amplitude;
        a0 = 1 + alpha / amplitude;
        a1 = -2 * cosine;
        a2 = 1 - alpha / amplitude;
        break;
    case EqKind::LowPass:
    case EqKind::HighPass: {
        const bool high = band.kind == EqKind::HighPass;
        b0 = (1 + (high ? cosine : -cosine)) / 2;
        b1 = (high ? -2 : 2) * b0;
        b2 = b0;
        a0 = 1 + alpha;
        a1 = -2 * cosine;
        a2 = 1 - alpha;
        break;
    }
    case EqKind::LowShelf: {
        const double beta = 2 * std::sqrt(amplitude) * alpha;
        b0 = amplitude * (amplitude + 1 - (amplitude - 1) * cosine + beta);
        b1 = 2 * amplitude * (amplitude - 1 - (amplitude + 1) * cosine);
        b2 = amplitude * (amplitude + 1 - (amplitude - 1) * cosine - beta);
        a0 = amplitude + 1 + (amplitude - 1) * cosine + beta;
        a1 = -2 * (amplitude - 1 + (amplitude + 1) * cosine);
        a2 = amplitude + 1 + (amplitude - 1) * cosine - beta;
        break;
    }
    case EqKind::HighShelf: {
        const double beta = 2 * std::sqrt(amplitude) * alpha;
        b0 = amplitude * (amplitude + 1 + (amplitude - 1) * cosine + beta);
        b1 = -2 * amplitude * (amplitude - 1 + (amplitude + 1) * cosine);
        b2 = amplitude * (amplitude + 1 + (amplitude - 1) * cosine - beta);
        a0 = amplitude + 1 - (amplitude - 1) * cosine + beta;
        a1 = 2 * (amplitude - 1 - (amplitude + 1) * cosine);
        a2 = amplitude + 1 - (amplitude - 1) * cosine - beta;
        break;
    }
    default:
        throw std::invalid_argument("unsupported equalizer band kind");
    }
    const BiquadCoefficients result{b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
    if (!valid(result))
        throw std::runtime_error("equalizer coefficient precision is unstable");
    return result;
}
double clean(double value) noexcept {
    return std::isfinite(value) && std::abs(value) >= 1e-20 ? value : 0;
}
} // namespace
EqBands default_eq_bands() noexcept {
    return {{{80, 0.55F}, {250, 0.9F}, {700, 0.9F}, {2000, 0.9F}, {6000, 0.9F}, {12000, 0.65F}}};
}
EqCoefficients design_equalizer(EqBands bands, int rate) {
    check_rate(rate);
    EqCoefficients result;
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = design(bands[i], rate);
    return result;
}
Equalizer::Equalizer(int rate) {
    check_rate(rate);
    for (auto &bank : banks_)
        bank.bypass.fill(true);
    fade_frames_ = static_cast<unsigned>(rate / 50); // 20ms output crossfade.
}
void Equalizer::begin_transition(const EqCoefficients &coefficients) noexcept {
    auto &next = banks_[active_ ^ 1U];
    next.coefficients = coefficients;
    next.all_bypassed = true;
    for (std::size_t i = 0; i < coefficients.size(); ++i) {
        const auto &c = coefficients[i];
        next.bypass[i] = c.b0 == 1 && c.b1 == 0 && c.b2 == 0 && c.a1 == 0 && c.a2 == 0;
        next.all_bypassed = next.all_bypassed && next.bypass[i];
    }
    next.history = {};
    remaining_ = fade_frames_;
    pending_valid_ = false;
}
bool Equalizer::set_coefficients(const EqCoefficients &coefficients) noexcept {
    for (const auto &c : coefficients)
        if (!valid(c))
            return false;
    bool same = true;
    for (std::size_t i = 0; i < coefficients.size(); ++i) {
        const auto &a = coefficients[i], &b = requested_[i];
        same = same && a.b0 == b.b0 && a.b1 == b.b1 && a.b2 == b.b2 && a.a1 == b.a1 && a.a2 == b.a2;
    }
    if (same)
        return true;
    requested_ = coefficients;
    if (remaining_ == 0)
        begin_transition(coefficients);
    else {
        pending_ = coefficients;
        pending_valid_ = true;
    }
    return true;
}
void Equalizer::reset() noexcept {
    for (auto &bank : banks_)
        bank.history = {};
}
double Equalizer::tick(Bank &bank, double input, unsigned channel) noexcept {
    double value = clean(input);
    if (bank.all_bypassed)
        return value;
    for (std::size_t i = 0; i < bank.coefficients.size(); ++i) {
        if (bank.bypass[i])
            continue;
        const auto &c = bank.coefficients[i];
        auto &h = bank.history[i][channel];
        const auto next =
            clean(c.b0 * value + c.b1 * h.x1 + c.b2 * h.x2 - c.a1 * h.y1 - c.a2 * h.y2);
        h.x2 = h.x1;
        h.x1 = value;
        h.y2 = h.y1;
        h.y1 = next;
        value = next;
    }
    return value;
}
void Equalizer::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    for (std::size_t frame = 0; frame < frames; ++frame) {
        if (remaining_ == 0 && pending_valid_) {
            begin_transition(pending_);
            pending_valid_ = false;
        }
        const float blend = remaining_ > 0 ? static_cast<float>(fade_frames_ - remaining_ + 1) /
                                                 static_cast<float>(fade_frames_)
                                           : 0;
        for (unsigned channel = 0; channel < static_cast<unsigned>(channels); ++channel) {
            const auto at = frame * static_cast<unsigned>(channels) + channel;
            const auto input = samples[at];
            const auto current = tick(banks_[active_], input, channel);
            const double output =
                remaining_ > 0
                    ? current + blend * (tick(banks_[active_ ^ 1U], input, channel) - current)
                    : current;
            samples[at] = std::abs(output) <= static_cast<double>(std::numeric_limits<float>::max())
                              ? static_cast<float>(output)
                              : 0;
        }
        if (remaining_ > 0 && --remaining_ == 0)
            active_ ^= 1U;
    }
}
} // namespace grainmm
