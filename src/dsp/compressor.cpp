// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/compressor.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace grainmm {
namespace {
float limit(float v, float low, float high, float fallback) noexcept {
    return std::isfinite(v) ? std::clamp(v, low, high) : fallback;
}
void smooth(float &v, float target, float a) noexcept {
    const float next = v + a * (target - v);
    v = next == v || std::abs(next - target) < 1e-6F ? target : next;
}
float input(float v) noexcept { return std::isfinite(v) ? std::clamp(v, -4.0F, 4.0F) : 0; }
} // namespace
CompressorParameters bounded(CompressorParameters p) noexcept {
    p.threshold_db = limit(p.threshold_db, -48, 0, -18);
    p.ratio = limit(p.ratio, 1, 20, 4);
    p.attack_ms = limit(p.attack_ms, 1, 100, 10);
    p.release_ms = limit(p.release_ms, 10, 2000, 150);
    p.makeup_db = limit(p.makeup_db, 0, 12, 0);
    p.mix = limit(p.mix, 0, 1, 0);
    return p;
}
PreparedCompressor prepare_compressor(CompressorParameters p, int rate) {
    if (rate < 8000 || rate > 96000)
        throw std::invalid_argument("compressor rate must be 8000..96000 Hz");
    p = bounded(p);
    return {p,
            std::pow(10.0F, p.threshold_db / 20),
            1 - 1 / p.ratio,
            1 - std::exp(-1000.0F / (p.attack_ms * static_cast<float>(rate))),
            1 - std::exp(-1000.0F / (p.release_ms * static_cast<float>(rate))),
            std::pow(10.0F, p.makeup_db / 20)};
}
Compressor::Compressor(int rate)
    : target_(prepare_compressor({}, rate)),
      smoothing_(1 - std::exp(-1.0F / (0.02F * static_cast<float>(rate)))) {}
void Compressor::apply(const PreparedCompressor &p) noexcept {
    target_ = p;
    target_.values = bounded(p.values);
    target_.threshold = limit(p.threshold, 0.003F, 1, 0.12589254F);
    target_.exponent = limit(p.exponent, 0, 0.95F, 0);
    target_.attack = limit(p.attack, 0, 1, 1);
    target_.release = limit(p.release, 0, 1, 1);
    target_.makeup = limit(p.makeup, 1, 4, 1);
}
void Compressor::reset() noexcept {
    envelope_ = 0;
    gain_ = target_gain_ = 1;
    curve_phase_ = 0;
    dormant_ = true;
}
void Compressor::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    if (mix_ == 0 && target_.values.mix == 0) {
        if (!dormant_)
            reset();
        return;
    }
    dormant_ = false;
    for (std::size_t frame = 0; frame < frames; ++frame) {
        const float left = input(samples[frame * static_cast<unsigned>(channels)]);
        const float right = channels == 2 ? input(samples[frame * 2 + 1]) : left;
        const float peak = std::max(std::abs(left), std::abs(right));
        envelope_ += (peak > envelope_ ? target_.attack : target_.release) * (peak - envelope_);
        if (envelope_ < 1e-20F)
            envelope_ = 0;
        if (curve_phase_++ == 0)
            target_gain_ = envelope_ > target_.threshold
                               ? std::pow(target_.threshold / envelope_, target_.exponent)
                               : 1;
        curve_phase_ &= 7;
        gain_ += (target_gain_ < gain_ ? target_.attack : target_.release) * (target_gain_ - gain_);
        smooth(mix_, target_.values.mix, smoothing_);
        smooth(makeup_, target_.makeup, smoothing_);
        const float scale = 1 + mix_ * (gain_ * makeup_ - 1);
        samples[frame * static_cast<unsigned>(channels)] = left * scale;
        if (channels == 2)
            samples[frame * 2 + 1] = right * scale;
    }
}
} // namespace grainmm
