// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/chorus.hpp"
#include "grainmm/dsp_utils.hpp"
namespace grainmm {
ChorusParameters bounded(ChorusParameters p) noexcept {
    p.rate_hz = dsp::limit(p.rate_hz, .05F, 5, .8F);
    p.depth_ms = dsp::limit(p.depth_ms, 0, 10, 3);
    p.delay_ms = dsp::limit(p.delay_ms, 2, 25, 12);
    p.feedback = dsp::limit(p.feedback, -.7F, .7F);
    p.mix = dsp::limit(p.mix, 0, 1);
    p.width = dsp::limit(p.width, 0, 1, 1);
    return p;
}
PreparedChorus prepare_chorus(ChorusParameters p, int rate) {
    dsp::checked_rate(rate);
    p = bounded(p);
    return {p, p.rate_hz / static_cast<float>(rate), p.depth_ms * .001F * static_cast<float>(rate),
            p.delay_ms * .001F * static_cast<float>(rate)};
}
Chorus::Chorus(int rate) : target_(prepare_chorus({}, rate)), smoothing_(dsp::smoothing(rate)) {
    delay_ = target_.delay;
    for (unsigned i = 0; i < 257; ++i)
        sine_[i] = static_cast<float>(std::sin(6.283185307179586 * static_cast<double>(i) / 256));
    sine_.back() = sine_.front();
}
void Chorus::apply(const PreparedChorus &p) noexcept {
    target_ = p;
    target_.values = bounded(p.values);
    target_.step = dsp::limit(p.step, 0, .001F);
    target_.depth = dsp::limit(p.depth, 0, 960);
    target_.delay = dsp::limit(p.delay, 1, 2400, 1);
}
float Chorus::sine(double phase) const noexcept {
    if (phase >= 1)
        phase -= 1;
    const auto at = phase * 256;
    const auto i = static_cast<unsigned>(at);
    return sine_[i] + (sine_[i + 1] - sine_[i]) * static_cast<float>(at - i);
}
void Chorus::reset() noexcept {
    for (auto &c : history_)
        c.fill(0);
    feedback_ = {};
    write_ = 0;
    phase_ = 0;
}
void Chorus::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    if (mix_ == 0 && target_.values.mix == 0)
        return;
    for (std::size_t i = 0; i < frames; ++i) {
        dsp::smooth(step_, target_.step, smoothing_);
        dsp::smooth(depth_, target_.depth, smoothing_);
        dsp::smooth(delay_, target_.delay, smoothing_);
        dsp::smooth(feedback_gain_, target_.values.feedback, smoothing_);
        dsp::smooth(mix_, target_.values.mix, smoothing_);
        dsp::smooth(width_, target_.values.width, smoothing_);
        for (unsigned ch = 0; ch < static_cast<unsigned>(channels); ++ch) {
            auto &out = samples[i * static_cast<unsigned>(channels) + ch];
            const auto dry = dsp::limit(out, -8, 8);
            const auto delay = std::clamp(
                delay_ + depth_ * sine(phase_ + (ch == 1 ? .25 * width_ : 0)), 1.0F, 4094.0F);
            const auto whole = static_cast<unsigned>(delay);
            const auto at = (write_ + 4096 - whole) & 4095U;
            const auto prev = (at + 4095) & 4095U;
            const auto fraction = delay - static_cast<float>(whole);
            const float wet = history_[ch][at] + fraction * (history_[ch][prev] - history_[ch][at]);
            history_[ch][write_] = dsp::quiet(std::clamp(dry + feedback_gain_ * wet, -4.0F, 4.0F));
            feedback_[ch] = dsp::quiet(wet);
            out = dry + mix_ * (wet - dry);
        }
        write_ = (write_ + 1) & 4095U;
        phase_ += step_;
        if (phase_ >= 1)
            phase_ -= 1;
    }
    if (mix_ == 0)
        reset();
}
} // namespace grainmm
