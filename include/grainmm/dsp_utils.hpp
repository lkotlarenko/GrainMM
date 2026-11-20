// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace grainmm::dsp {
inline float limit(float x, float lo, float hi, float fallback = 0) noexcept {
    return std::isfinite(x) ? std::clamp(x, lo, hi) : fallback;
}
inline void smooth(float &x, float goal, float a) noexcept {
    const auto next = x + a * (goal - x);
    x = next == x || std::abs(next - goal) < 1e-6F ? goal : next;
}
inline int checked_rate(int rate) {
    if (rate < 8000 || rate > 96000)
        throw std::invalid_argument("DSP rate must be8000..96000Hz");
    return rate;
}
inline float smoothing(int rate) {
    return 1 - std::exp(-1 / (.02F * static_cast<float>(checked_rate(rate))));
}
inline float db_gain(float db) noexcept { return std::pow(10.0F, db / 20); }
inline float quiet(float x) noexcept { return std::isfinite(x) && std::abs(x) > 1e-20F ? x : 0; }
} // namespace grainmm::dsp
