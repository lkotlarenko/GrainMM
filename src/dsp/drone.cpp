// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/drone.hpp"
#include "grainmm/dsp_utils.hpp"
namespace grainmm {
DroneParameters bounded(DroneParameters p) noexcept {
    p.mode = std::round(dsp::limit(p.mode, 0, 3));
    p.rate_hz = dsp::limit(p.rate_hz, .01F, .5F, .05F);
    p.position_depth = dsp::limit(p.position_depth, 0, 1, .2F);
    p.pitch_cents = dsp::limit(p.pitch_cents, 0, 1200);
    p.spread_depth = dsp::limit(p.spread_depth, 0, 1);
    p.hold = std::round(dsp::limit(p.hold, 0, 1));
    return p;
}
DroneMotion::DroneMotion(int rate, std::uint32_t seed)
    : rate_(dsp::checked_rate(rate)), seed_(seed ? seed : 1), random_(seed_),
      smoothing_(1.F / (.02F * static_cast<float>(rate))) {
    reset();
}
float DroneMotion::random_signed() noexcept {
    random_ ^= random_ << 13;
    random_ ^= random_ >> 17;
    random_ ^= random_ << 5;
    return static_cast<float>(random_ >> 8) / 8388607.5F - 1;
}
void DroneMotion::reset() noexcept {
    phase_ = 0;
    random_ = seed_;
    from_ = 0;
    to_ = random_signed();
    value_ = target_ = {};
    countdown_ = 0;
}
void DroneMotion::apply(DroneParameters p) noexcept {
    parameters_ = bounded(p);
    increment_ = static_cast<double>(parameters_.rate_hz) / rate_;
    countdown_ = 0; // Refresh targets without resetting musical phase or drift seed.
}
DroneFrame DroneMotion::next(bool playing) noexcept {
    if (!playing)
        return value_;
    if (parameters_.mode == 0 && value_.position == 0 && value_.pitch_cents == 0 &&
        value_.spread == 0)
        return {};
    if (parameters_.mode != 0 && !holding()) {
        phase_ += increment_;
        if (phase_ >= 1) {
            phase_ -= 1;
            from_ = to_;
            to_ = random_signed();
        }
    }
    if (countdown_ == 0) {
        target_ = {};
        if (parameters_.mode != 0) {
            const auto x = static_cast<float>(phase_);
            const float wave = std::sin(6.283185307179586F * x);
            const float drift = from_ + (to_ - from_) * x * x * (3 - 2 * x);
            const float source = parameters_.mode == 1   ? wave
                                 : parameters_.mode == 2 ? drift
                                                         : .65F * wave + .35F * drift;
            target_ = {source * parameters_.position_depth, source * parameters_.pitch_cents,
                       source * parameters_.spread_depth};
        }
        countdown_ = 64;
    }
    --countdown_;
    const auto slew = [&](float now, float wanted, float range) {
        const float step = range * smoothing_;
        return now + std::clamp(wanted - now, -step, step);
    };
    value_.position = slew(value_.position, target_.position, 1);
    value_.pitch_cents = slew(value_.pitch_cents, target_.pitch_cents, 1200);
    value_.spread = slew(value_.spread, target_.spread, 1);
    return value_;
}
} // namespace grainmm
