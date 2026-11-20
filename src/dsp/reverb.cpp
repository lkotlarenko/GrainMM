// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/reverb.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace grainmm {
namespace {
constexpr std::array<float, 4> comb_seconds{0.0297F, 0.0371F, 0.0411F, 0.0437F};
float limit(float v, float low, float high, float fallback) noexcept {
    return std::isfinite(v) ? std::clamp(v, low, high) : fallback;
}
float clean(float v) noexcept {
    return std::isfinite(v) && std::abs(v) > 1e-20F ? std::clamp(v, -4.0F, 4.0F) : 0;
}
float comb_time(unsigned index, float size) noexcept {
    return comb_seconds[index / 2] * (0.5F + size) + (index % 2 ? 0.00073F : 0);
}
void slew(float &value, float target, float amount) noexcept {
    const float next = value + amount * (target - value);
    value = next == value || std::abs(next - target) < 1e-6F ? target : next;
}
} // namespace
ReverbParameters bounded(ReverbParameters p) noexcept {
    p.mix = limit(p.mix, 0, 1, 0);
    p.size = limit(p.size, 0, 1, 0.7F);
    p.decay_s = limit(p.decay_s, 0.3F, 20, 6);
    p.damp_hz = limit(p.damp_hz, 500, 20000, 7000);
    p.predelay_ms = limit(p.predelay_ms, 0, 250, 20);
    p.width = limit(p.width, 0, 1, 1);
    return p;
}
PreparedReverb prepare_reverb(ReverbParameters p, int rate) {
    if (rate < 8000 || rate > 96000)
        throw std::invalid_argument("reverb rate must be 8000..96000 Hz");
    PreparedReverb result;
    result.parameters = bounded(p);
    for (unsigned i = 0; i < 8; ++i)
        result.feedback[i] =
            std::pow(0.001F, comb_time(i, result.parameters.size) / result.parameters.decay_s);
    const float cutoff = std::min(result.parameters.damp_hz, static_cast<float>(rate) * 0.45F);
    result.damping = 1 - std::exp(-6.28318530718F * cutoff / static_cast<float>(rate));
    return result;
}
Reverb::Reverb(int rate) : rate_(rate), current_(prepare_reverb({}, rate)), target_(current_) {
    smoothing_ = 1 - std::exp(-1.0F / (0.02F * static_cast<float>(rate)));
    for (unsigned i = 0; i < 8; ++i)
        combs_[i].data.resize(static_cast<std::size_t>(comb_time(i, 1) * static_cast<float>(rate)) +
                              4);
    for (unsigned i = 0; i < 4; ++i)
        diffusers_[i].data.resize(
            static_cast<std::size_t>((i / 2 ? 0.0017F : 0.0053F) * static_cast<float>(rate)) +
            (i % 2 ? 17U : 0U) + 4);
    for (auto &line : predelay_)
        line.data.resize(static_cast<std::size_t>(rate / 4) + 4);
}
void Reverb::apply(const PreparedReverb &p) noexcept {
    target_.parameters = bounded(p.parameters);
    target_.damping = limit(p.damping, 0, 1, 1);
    for (unsigned i = 0; i < 8; ++i)
        target_.feedback[i] = limit(p.feedback[i], 0, 0.995F, 0);
}
float Reverb::Line::read(float age) const noexcept {
    age = std::clamp(age, 1.0F, static_cast<float>(data.size() - 2));
    const auto whole = static_cast<std::size_t>(age);
    const auto get = [&](std::size_t distance) {
        if (distance > valid)
            return 0.0F;
        return data[at >= distance ? at - distance : at + data.size() - distance];
    };
    const auto a = get(whole);
    return a + (get(whole + 1) - a) * (age - static_cast<float>(whole));
}
void Reverb::Line::push(float value) noexcept {
    data[at] = clean(value);
    if (++at == data.size())
        at = 0;
    valid = std::min(valid + 1, data.size() - 1);
}
void Reverb::reset() noexcept {
    for (auto &line : combs_)
        line.clear();
    for (auto &line : diffusers_)
        line.clear();
    for (auto &line : predelay_)
        line.clear();
    filtered_ = {};
    dormant_ = true;
}
std::size_t Reverb::storage_bytes() const noexcept {
    std::size_t total = 0;
    for (const auto &line : combs_)
        total += line.data.size() * sizeof(float);
    for (const auto &line : diffusers_)
        total += line.data.size() * sizeof(float);
    for (const auto &line : predelay_)
        total += line.data.size() * sizeof(float);
    return total;
}
void Reverb::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    if (current_.parameters.mix == 0 && target_.parameters.mix == 0) {
        if (!dormant_)
            reset();
        return;
    }
    dormant_ = false;
    for (std::size_t frame = 0; frame < frames; ++frame) {
        auto &p = current_.parameters;
        const auto &t = target_.parameters;
        slew(p.mix, t.mix, smoothing_);
        slew(p.size, t.size, smoothing_);
        slew(p.predelay_ms, t.predelay_ms, smoothing_);
        slew(p.width, t.width, smoothing_);
        slew(current_.damping, target_.damping, smoothing_);
        for (unsigned i = 0; i < 8; ++i)
            slew(current_.feedback[i], target_.feedback[i], smoothing_);
        const float left = clean(samples[frame * static_cast<unsigned>(channels)]);
        const float right = channels == 2 ? clean(samples[frame * 2 + 1]) : left;
        std::array<float, 2> wet{};
        for (unsigned ch = 0; ch < 2; ++ch) {
            const float input = ch ? right : left;
            const float pre_age = p.predelay_ms * (static_cast<float>(rate_) * 0.001F);
            const float delayed = pre_age < 1 ? input : predelay_[ch].read(pre_age);
            predelay_[ch].push(input);
            float sum = 0;
            for (unsigned comb = 0; comb < 4; ++comb) {
                const unsigned i = comb * 2 + ch;
                const float tap = combs_[i].read(comb_time(i, p.size) * static_cast<float>(rate_));
                filtered_[i] = clean(filtered_[i] + current_.damping * (tap - filtered_[i]));
                combs_[i].push(delayed + filtered_[i] * current_.feedback[i]);
                sum += tap * 0.25F;
            }
            for (unsigned stage = 0; stage < 2; ++stage) {
                auto &line = diffusers_[stage * 2 + ch];
                const float tap = line.read(static_cast<float>(line.data.size() - 3));
                const float output = tap - 0.5F * sum;
                line.push(sum + 0.5F * output);
                sum = output;
            }
            wet[ch] = sum;
        }
        const float middle = (wet[0] + wet[1]) * 0.5F;
        samples[frame * static_cast<unsigned>(channels)] =
            left * (1 - p.mix) +
            (channels == 1 ? middle : middle + (wet[0] - middle) * p.width) * p.mix;
        if (channels == 2)
            samples[frame * 2 + 1] =
                right * (1 - p.mix) + (middle + (wet[1] - middle) * p.width) * p.mix;
    }
}
} // namespace grainmm
