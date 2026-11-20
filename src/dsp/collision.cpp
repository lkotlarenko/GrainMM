// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/collision.hpp"
#include "grainmm/dsp_utils.hpp"
namespace grainmm {
CollisionParameters bounded(CollisionParameters p) noexcept {
    p.enabled = std::round(dsp::limit(p.enabled, 0, 1));
    p.count = std::round(dsp::limit(p.count, 1, 6, 4));
    p.gravity = dsp::limit(p.gravity, 0, 2, 1);
    p.rotation = dsp::limit(p.rotation, -1, 1);
    p.bounce = dsp::limit(p.bounce, .2F, .99F, .8F);
    p.level = dsp::limit(p.level, 0, .2F);
    p.note = std::round(dsp::limit(p.note, 48, 72, 60));
    p.scale = std::round(dsp::limit(p.scale, 0, 3));
    p.octave = std::round(dsp::limit(p.octave, -1, 1));
    p.decay = dsp::limit(p.decay, .1F, 3, .8F);
    p.brightness = dsp::limit(p.brightness, 0, 1, .4F);
    p.excite = dsp::limit(p.excite, 0, 1, .7F);
    return p;
}
PreparedCollision prepare_collision(CollisionParameters p, int rate) {
    dsp::checked_rate(rate);
    p = bounded(p);
    PreparedCollision result;
    result.values = p;
    constexpr unsigned scales[4][6] = {
        {0, 2, 3, 5, 7, 10}, {0, 2, 4, 5, 7, 9}, {0, 2, 4, 7, 9, 12}, {0, 1, 2, 3, 4, 5}};
    for (unsigned n = 0; n < 6; ++n)
        for (unsigned m = 0; m < 2; ++m) {
            const double midi = p.note + p.octave * 12 +
                                static_cast<float>(scales[static_cast<unsigned>(p.scale)][n]);
            const double frequency =
                std::min(.4 * rate, 440 * std::exp2((midi - 69) / 12) * (m == 0 ? 1 : 2.73));
            const double radius =
                std::exp(-6.907755 / (static_cast<double>(p.decay) * rate) * (m == 0 ? 1 : 2));
            const double angle = 6.283185307179586 * frequency / rate;
            result.notes[n][m] = {
                static_cast<float>(2 * radius * std::cos(angle)),
                static_cast<float>(radius * radius),
                static_cast<float>(std::sin(angle) * (m == 0 ? 1 : p.brightness * .5))};
        }
    return result;
}
CollisionSynth::CollisionSynth(int rate) : rate_(rate), target_(prepare_collision({}, rate)) {
    reset_physics();
}
void CollisionSynth::reset_physics() noexcept {
    balls_ = {{{.25F, -.25F, .45F, .1F},
               {-.3F, -.08F, -.2F, -.35F},
               {.1F, .1F, .3F, -.45F},
               {-.15F, .25F, -.4F, .05F},
               {.35F, .18F, .15F, .4F},
               {-.35F, -.28F, .4F, -.15F}}};
    accumulator_ = 0;
    phase_ = 0;
}
void CollisionSynth::apply(const PreparedCollision &p) noexcept {
    const bool reset =
        p.values.enabled != 0 && (!enabled() || p.values.count != target_.values.count);
    target_ = p;
    target_.values = bounded(p.values);
    for (auto &note : target_.notes)
        for (auto &m : note) {
            m.radius2 = dsp::limit(m.radius2, 0, .999999F);
            const float stable = 1 + m.radius2 - 1e-6F;
            m.coefficient = dsp::limit(m.coefficient, -stable, stable);
            m.impulse = dsp::limit(m.impulse, 0, 1);
        }
    if (reset)
        reset_physics();
}
void CollisionSynth::strike(unsigned note, float energy) noexcept {
    const auto energy_of = [](const Voice &v) {
        return std::abs(v.a[0]) + std::abs(v.b[0]) + std::abs(v.a[1]) + std::abs(v.b[1]);
    };
    auto &v =
        *std::min_element(voices_.begin(), voices_.end(), [&](const Voice &a, const Voice &b) {
            return energy_of(a) < energy_of(b);
        });
    v = {};
    v.modes = target_.notes[note % 6];
    for (unsigned m = 0; m < 2; ++m)
        v.a[m] = std::min(energy, 3.F) * target_.values.excite * .25F * v.modes[m].impulse;
    ++hits_;
}
void CollisionSynth::physics() noexcept {
    constexpr float nx[6] = {1, .5F, -.5F, -1, -.5F, .5F};
    constexpr float ny[6] = {0, .8660254F, .8660254F, 0, -.8660254F, -.8660254F};
    phase_ += target_.values.rotation * .015F;
    if (phase_ > 6.2831853F)
        phase_ -= 6.2831853F;
    if (phase_ < 0)
        phase_ += 6.2831853F;
    const float gx = std::sin(phase_) * target_.values.gravity,
                gy = std::cos(phase_) * target_.values.gravity;
    for (unsigned i = 0; i < static_cast<unsigned>(target_.values.count); ++i) {
        auto &b = balls_[i];
        b.vx = std::clamp(b.vx + gx / 120, -3.F, 3.F);
        b.vy = std::clamp(b.vy + gy / 120, -3.F, 3.F);
        b.x += b.vx / 120;
        b.y += b.vy / 120;
        for (unsigned wall = 0; wall < 6; ++wall) {
            const float excess = b.x * nx[wall] + b.y * ny[wall] - .78F;
            if (excess > 0) {
                b.x -= excess * nx[wall];
                b.y -= excess * ny[wall];
                const float speed = b.vx * nx[wall] + b.vy * ny[wall];
                if (speed > 0) {
                    const float rebound =
                        std::max(.6F + target_.values.bounce, target_.values.bounce * speed);
                    b.vx -= (speed + rebound) * nx[wall];
                    b.vy -= (speed + rebound) * ny[wall];
                    if (speed > .015F)
                        strike(wall + i, speed);
                }
            }
        }
    }
}
void CollisionSynth::add(float *out, std::size_t frames, int channels, bool playing) noexcept {
    if (!out || (channels != 1 && channels != 2))
        return;
    if (playing && enabled() && !playing_)
        reset_physics();
    playing_ = playing && enabled();
    if (!playing_ && gain_ == 0) {
        voices_ = {};
        return;
    }
    const float ramp = .2F / (.02F * static_cast<float>(rate_));
    for (std::size_t i = 0; i < frames; ++i) {
        gain_ += std::clamp((playing_ ? target_.values.level : 0) - gain_, -ramp, ramp);
        if (playing_) {
            accumulator_ += 120;
            if (accumulator_ >= static_cast<unsigned>(rate_)) {
                accumulator_ -= static_cast<unsigned>(rate_);
                physics();
            }
        }
        float sum = 0;
        for (auto &v : voices_)
            for (unsigned m = 0; m < 2; ++m) {
                const float next =
                    dsp::quiet(v.modes[m].coefficient * v.a[m] - v.modes[m].radius2 * v.b[m]);
                v.b[m] = v.a[m];
                v.a[m] = next;
                sum += next;
            }
        const float value = sum * gain_ * .125F;
        for (unsigned ch = 0; ch < static_cast<unsigned>(channels); ++ch)
            out[i * static_cast<unsigned>(channels) + ch] += value;
        if (!playing_ && gain_ == 0) {
            voices_ = {};
            return;
        }
    }
}
std::array<unsigned, 6> CollisionSynth::positions() const noexcept {
    std::array<unsigned, 6> out{};
    if (!enabled())
        return out;
    for (unsigned i = 0; i < static_cast<unsigned>(target_.values.count); ++i) {
        const auto &b = balls_[i];
        const auto x = static_cast<unsigned>(std::clamp((b.x + 1) * 127.5F, 0.F, 255.F));
        const auto y = static_cast<unsigned>(std::clamp((b.y + 1) * 127.5F, 0.F, 255.F));
        out[i] = x | (y << 8) | (1U << 16);
    }
    return out;
}
} // namespace grainmm
