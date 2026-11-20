// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/granular.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <type_traits>

namespace grainmm {
namespace {
float limit(float value, float low, float high, float fallback) noexcept {
    return std::isfinite(value) ? std::clamp(value, low, high) : fallback;
}
} // namespace
GrainParameters bounded(GrainParameters p) noexcept {
    p.size_ms = limit(p.size_ms, 5, 250, 80);
    p.density = limit(p.density, 1, 80, 12);
    p.pitch = limit(p.pitch, 0.25F, 2, 1);
    p.position = limit(p.position, 0, 1, 0.5F);
    p.spread = limit(p.spread, 0, 1, 0.1F);
    p.gain = limit(p.gain, 0, 0.5F, 0); // Invalid gain fails silent.
    p.reverse = limit(p.reverse, 0, 1, 0);
    p.pitch_jitter = limit(p.pitch_jitter, 0, 12, 0);
    p.size_jitter = limit(p.size_jitter, 0, 1, 0);
    p.timing_jitter = limit(p.timing_jitter, 0, 1, 0);
    p.pan_spread = limit(p.pan_spread, 0, 1, 0);
    p.scan_hz = limit(p.scan_hz, -0.5F, 0.5F, 0);
    p.start = limit(p.start, 0, 0.99F, 0);
    p.end = limit(p.end, p.start + 0.01F, 1, 1);
    p.attack = limit(p.attack, 0.01F, 0.49F, 0.2F);
    p.release = limit(p.release, 0.01F, 0.49F, 0.2F);
    p.curve = limit(p.curve, 0, 1, 0);
    p.width = limit(p.width, 0, 1, 1);
    return p;
}
Granular::Granular(const Sample &sample, int output_rate, std::uint32_t seed)
    : sample_(sample), rate_(output_rate), drone_(output_rate, seed ^ 0x729a4d35U),
      seed_(seed ? seed : 1), random_(seed_) {
    if (output_rate < 8000 || output_rate > 96000)
        throw std::invalid_argument("granular output rate must be 8000..96000 Hz");
    for (std::size_t i = 0; i < envelope_.size(); ++i)
        envelope_[i] =
            static_cast<float>(0.5 - 0.5 * std::cos(6.283185307179586 * static_cast<double>(i) /
                                                    static_cast<double>(envelope_.size() - 1)));
    envelope_.front() = envelope_.back() = 0;
}
void Granular::set_parameters(GrainParameters parameters) noexcept {
    const auto next = bounded(parameters);
    countdown_ *= static_cast<double>(parameters_.density) / next.density;
    parameters_ = next;
}
void Granular::reset() noexcept {
    voices_ = {};
    harmony_.reset();
    drone_.reset();
    modulation_ = {};
    countdown_ = scan_phase_ = 0;
    gain_ = 0;
    width_ = parameters_.width;
    random_ = seed_;
    spawned_ = dropped_ = 0;
}
float Granular::random_signed() noexcept {
    random_ ^= random_ << 13;
    random_ ^= random_ >> 17;
    random_ ^= random_ << 5;
    return static_cast<float>(random_ >> 8) / 8388607.5F - 1.0F;
}
void Granular::spawn() noexcept {
    for (auto &voice : voices_)
        if (voice.age == voice.duration) {
            double center = parameters_.position;
            if (parameters_.scan_hz != 0) {
                center += scan_phase_;
                if (center >= 1)
                    center -= 1;
            }
            const double position = std::clamp(
                center + modulation_.position +
                    random_signed() * std::clamp(parameters_.spread + modulation_.spread, 0.F, 1.F),
                0.0, 1.0);
            voice.first = std::min(sample_.frames() - 1,
                                   static_cast<std::size_t>(parameters_.start *
                                                            static_cast<double>(sample_.frames())));
            voice.end = std::clamp(
                static_cast<std::size_t>(parameters_.end * static_cast<double>(sample_.frames())),
                voice.first + 1, sample_.frames());
            voice.position = static_cast<double>(voice.first) +
                             position * static_cast<double>(voice.end - voice.first - 1);
            voice.attack_scale = 1 / (parameters_.attack * 1024);
            voice.release_scale = 1 / (parameters_.release * 1024);
            voice.curve = parameters_.curve;
            voice.increment =
                static_cast<double>(sample_.rate()) / rate_ * parameters_.pitch * harmony_.next();
            if (modulation_.pitch_cents != 0) {
                const auto pitch = std::clamp(
                    parameters_.pitch * std::exp2(modulation_.pitch_cents / 1200.F), .25F, 2.F);
                voice.increment *= pitch / parameters_.pitch;
            }
            if (parameters_.pitch_jitter > 0)
                voice.increment *= std::exp2(parameters_.pitch_jitter * random_signed() / 12.0F);
            if (parameters_.reverse >= 1 ||
                (parameters_.reverse > 0 && random_signed() < 2 * parameters_.reverse - 1))
                voice.increment = -voice.increment;
            const float pan =
                parameters_.pan_spread > 0 ? random_signed() * parameters_.pan_spread : 0;
            voice.left_gain = 1 - std::max(pan, 0.0F);
            voice.right_gain = 1 + std::min(pan, 0.0F);
            voice.age = 0;
            const float size = parameters_.size_jitter > 0
                                   ? std::clamp(parameters_.size_ms *
                                                    (1 + random_signed() * parameters_.size_jitter),
                                                5.0F, 250.0F)
                                   : parameters_.size_ms;
            voice.duration = static_cast<std::uint32_t>(size * 0.001 * rate_);
            voice.window_step = 1024.0 / (voice.duration - 1);
            ++spawned_;
            return;
        }
    ++dropped_; // Fixed cap: skip new grain rather than steal a sounding voice.
}
float Granular::interpolate(const Voice &voice, int channel) const noexcept {
    const auto frame = static_cast<std::size_t>(voice.position);
    const auto next = frame + 1 == voice.end ? voice.first : frame + 1;
    const float fraction = static_cast<float>(voice.position - static_cast<double>(frame));
    const float a = sample_.value(frame, channel);
    return a + (sample_.value(next, channel) - a) * fraction;
}
void Granular::render(std::int16_t *out, std::size_t frames, int channels, bool playing) noexcept {
    render_into(out, frames, channels, playing);
}
void Granular::render_float(float *out, std::size_t frames, int channels, bool playing) noexcept {
    render_into(out, frames, channels, playing);
}
template <typename Output>
void Granular::render_into(Output *out, std::size_t frames, int channels, bool playing) noexcept {
    if (!out || (channels != 1 && channels != 2))
        return;
    const float target = playing ? parameters_.gain : 0;
    const float ramp = 0.5F / (0.02F * static_cast<float>(rate_));
    for (std::size_t frame = 0; frame < frames; ++frame) {
        modulation_ = drone_.next(playing);
        gain_ += std::clamp(target - gain_, -ramp, ramp);
        const float width_ramp = 1 / (0.02F * static_cast<float>(rate_));
        width_ += std::clamp(parameters_.width - width_, -width_ramp, width_ramp);
        if (!playing && gain_ == 0) {
            voices_ = {};
            countdown_ = 0;
            harmony_.reset();
            std::fill_n(out + frame * static_cast<unsigned>(channels),
                        (frames - frame) * static_cast<unsigned>(channels), Output{0});
            return;
        } else {
            if (countdown_ <= 0) {
                spawn();
                const float jitter =
                    parameters_.timing_jitter > 0
                        ? std::max(0.05F, 1 + random_signed() * parameters_.timing_jitter)
                        : 1;
                countdown_ += static_cast<double>(rate_) / parameters_.density * jitter;
            }
            countdown_ -= 1;
        }
        float left = 0, right = 0;
        for (auto &voice : voices_) {
            if (voice.age == voice.duration)
                continue;
            const double phase = voice.age + 1 == voice.duration
                                     ? 1024.0
                                     : static_cast<double>(voice.age) * voice.window_step;
            const auto index = static_cast<std::size_t>(phase);
            float window =
                index >= 1024
                    ? 0
                    : envelope_[index] + (envelope_[index + 1] - envelope_[index]) *
                                             static_cast<float>(phase - static_cast<double>(index));
            if (voice.curve > 0) {
                const float custom =
                    std::clamp(std::min(static_cast<float>(phase) * voice.attack_scale,
                                        (1024 - static_cast<float>(phase)) * voice.release_scale),
                               0.0F, 1.0F);
                window += voice.curve * (custom - window);
            }
            left += interpolate(voice, 0) * window * voice.left_gain;
            right += interpolate(voice, 1) * window * voice.right_gain;
            voice.position += voice.increment;
            const double first = static_cast<double>(voice.first);
            const double size = static_cast<double>(voice.end - voice.first);
            if (voice.position >= static_cast<double>(voice.end) || voice.position < first)
                voice.position -= size * std::floor((voice.position - first) / size);
            ++voice.age;
        }
        if (playing && parameters_.scan_hz != 0 && !drone_.holding()) {
            scan_phase_ += static_cast<double>(parameters_.scan_hz) / rate_;
            if (scan_phase_ >= 1)
                scan_phase_ -= 1;
            if (scan_phase_ < 0)
                scan_phase_ += 1;
        }
        constexpr float units = std::is_same_v<Output, float> ? 1.0F : 32767.0F;
        const float scale = gain_ / voice_limit * units;
        if (channels == 1)
            out[frame] = static_cast<Output>((left + right) * 0.5F * scale);
        else {
            if (width_ != 1) {
                const float mid = (left + right) * 0.5F;
                left = mid + width_ * (left - mid);
                right = mid + width_ * (right - mid);
            }
            out[frame * 2] = static_cast<Output>(left * scale);
            out[frame * 2 + 1] = static_cast<Output>(right * scale);
        }
    }
}
} // namespace grainmm

unsigned grainmm::Granular::visual_state() const noexcept {
    unsigned count = 0, position = 0;
    for (const auto &v : voices_)
        if (v.age < v.duration) {
            if (count == 0)
                position = static_cast<unsigned>(
                    std::clamp(v.position / static_cast<double>(sample_.frames()), 0.0, 1.0) *
                    65535);
            ++count;
        }
    return position | (count << 16);
}
