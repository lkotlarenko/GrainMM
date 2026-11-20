// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/crusher.hpp"
#include "grainmm/dsp_utils.hpp"
namespace grainmm {
CrusherParameters bounded(CrusherParameters p) noexcept {
    using dsp::limit;
    p.bits = std::round(limit(p.bits, 2, 16, 12));
    p.hold = std::round(limit(p.hold, 1, 64, 1));
    p.drive_db = limit(p.drive_db, 0, 18);
    p.tone_hz = limit(p.tone_hz, 500, 16000, 12000);
    p.mix = limit(p.mix, 0, 1);
    p.output_db = limit(p.output_db, -18, 0);
    return p;
}
PreparedCrusher prepare_crusher(CrusherParameters p, int rate) {
    dsp::checked_rate(rate);
    p = bounded(p);
    return {p, std::exp2(p.bits - 1), dsp::db_gain(p.drive_db),
            1 - std::exp(-6.283185307F * std::min(p.tone_hz, .45F * static_cast<float>(rate)) /
                         static_cast<float>(rate)),
            dsp::db_gain(p.output_db)};
}
Crusher::Crusher(int rate) : target_(prepare_crusher({}, rate)), smoothing_(dsp::smoothing(rate)) {
    tone_ = target_.tone;
}
void Crusher::apply(const PreparedCrusher &p) noexcept {
    target_ = p;
    target_.values = bounded(p.values);
    target_.levels = dsp::limit(p.levels, 2, 32768, 2048);
    target_.drive = dsp::limit(p.drive, 1, 8, 1);
    target_.tone = dsp::limit(p.tone, .001F, 1, 1);
    target_.output = dsp::limit(p.output, .125F, 1, 1);
}
void Crusher::reset() noexcept {
    held_ = {};
    low_ = {};
    countdown_ = 0;
}
void Crusher::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    if (mix_ == 0 && target_.values.mix == 0)
        return;
    for (std::size_t i = 0; i < frames; ++i) {
        dsp::smooth(mix_, target_.values.mix, smoothing_);
        dsp::smooth(drive_, target_.drive, smoothing_);
        dsp::smooth(levels_, target_.levels, smoothing_);
        dsp::smooth(tone_, target_.tone, smoothing_);
        dsp::smooth(output_, target_.output, smoothing_);
        const bool capture = countdown_ == 0;
        for (unsigned ch = 0; ch < static_cast<unsigned>(channels); ++ch) {
            auto &out = samples[i * static_cast<unsigned>(channels) + ch];
            const auto dry = dsp::limit(out, -8, 8);
            if (capture)
                held_[ch] = std::round(std::clamp(dry * drive_, -1.0F, 1.0F) * levels_) / levels_;
            low_[ch] = dsp::quiet(low_[ch] + tone_ * (held_[ch] - low_[ch]));
            out = dry + mix_ * (low_[ch] * output_ - dry);
        }
        if (capture)
            countdown_ = static_cast<unsigned>(target_.values.hold);
        --countdown_;
    }
    if (mix_ == 0)
        reset();
}
} // namespace grainmm
