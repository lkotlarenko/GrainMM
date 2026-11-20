// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/output.hpp"
#include "grainmm/dsp_utils.hpp"
namespace grainmm {
OutputParameters bounded(OutputParameters p) noexcept {
    p.gain_db = dsp::limit(p.gain_db, -24, 18);
    p.limiter = std::round(dsp::limit(p.limiter, 0, 1));
    p.ceiling_db = dsp::limit(p.ceiling_db, -12, 0, -1);
    p.release_ms = dsp::limit(p.release_ms, 20, 1000, 200);
    p.balance = dsp::limit(p.balance, -1, 1);
    p.mono = std::round(dsp::limit(p.mono, 0, 1));
    return p;
}
PreparedOutput prepare_output(OutputParameters p, int rate) {
    dsp::checked_rate(rate);
    p = bounded(p);
    return {p, dsp::db_gain(p.gain_db), dsp::db_gain(p.ceiling_db),
            1 - std::exp(-1.F / (.001F * p.release_ms * static_cast<float>(rate)))};
}
Output::Output(int rate) : target_(prepare_output({}, rate)), smoothing_(dsp::smoothing(rate)) {}
void Output::apply(const PreparedOutput &p) noexcept {
    target_ = p;
    target_.values = bounded(p.values);
    target_.gain = dsp::limit(p.gain, .063F, 8, 1);
    target_.ceiling = dsp::limit(p.ceiling, .25F, 1, .89125094F);
    target_.release = dsp::limit(p.release, .000001F, .01F, .0001F);
}
void Output::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    reduction_ = 0;
    if (gain_ == 1 && target_.gain == 1 && balance_ == 0 && target_.values.balance == 0 &&
        mono_ == 0 && target_.values.mono == 0 && target_.values.limiter == 0 && limiting_ == 1)
        return;
    float minimum = 1;
    for (std::size_t i = 0; i < frames; ++i) {
        dsp::smooth(gain_, target_.gain, smoothing_);
        dsp::smooth(ceiling_, target_.ceiling, smoothing_);
        dsp::smooth(balance_, target_.values.balance, smoothing_);
        dsp::smooth(mono_, target_.values.mono, smoothing_);
        auto *frame = samples + i * static_cast<unsigned>(channels);
        // Keep exceptional finite inputs from overflowing the matrix or gain.
        // This is 240 dB above full scale, far beyond any useful audio signal.
        constexpr float input_limit = 1e12F;
        float left = dsp::quiet(dsp::limit(frame[0], -input_limit, input_limit)),
              right = channels == 2 ? dsp::quiet(dsp::limit(frame[1], -input_limit, input_limit))
                                    : left;
        const float middle = .5F * left + .5F * right;
        if (mono_ == 1)
            left = right = middle;
        else {
            left += mono_ * (middle - left);
            right += mono_ * (middle - right);
        }
        left *= gain_ * (channels == 2 ? 1 - std::max(balance_, 0.F) : 1);
        right *= gain_ * (channels == 2 ? 1 + std::min(balance_, 0.F) : 1);
        const float peak = std::max(std::abs(left), std::abs(right));
        const float needed = target_.values.limiter > 0 && peak > ceiling_ ? ceiling_ / peak : 1;
        float released = limiting_;
        dsp::smooth(released, 1, target_.release);
        limiting_ = std::min(needed, released);
        minimum = std::min(minimum, limiting_);
        frame[0] = left * limiting_;
        if (channels == 2)
            frame[1] = right * limiting_;
    }
    reduction_ = static_cast<unsigned>((1 - minimum) * 65535);
}
} // namespace grainmm
