// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/filter.hpp"
#include "grainmm/dsp_utils.hpp"
namespace grainmm {
FilterParameters bounded(FilterParameters p) noexcept {
    p.cutoff_hz = dsp::limit(p.cutoff_hz, 20, 16000, 8000);
    p.resonance = dsp::limit(p.resonance, .5F, 4, .707F);
    p.mode = std::round(dsp::limit(p.mode, 0, 2));
    p.drive_db = dsp::limit(p.drive_db, 0, 18);
    p.mix = dsp::limit(p.mix, 0, 1);
    p.output_db = dsp::limit(p.output_db, -18, 0);
    return p;
}
PreparedFilter prepare_filter(FilterParameters p, int rate) {
    dsp::checked_rate(rate);
    p = bounded(p);
    return {p,
            std::tan(3.141592654F * std::min(p.cutoff_hz, .4F * static_cast<float>(rate)) /
                     static_cast<float>(rate)),
            1 / p.resonance, dsp::db_gain(p.drive_db), dsp::db_gain(p.output_db)};
}
Filter::Filter(int rate)
    : target_(prepare_filter({}, rate)), g_(target_.g), smoothing_(dsp::smoothing(rate)) {}
void Filter::apply(const PreparedFilter &p) noexcept {
    target_ = p;
    target_.values = bounded(p.values);
    target_.g = dsp::limit(p.g, .0001F, 3.1F, 1);
    target_.k = dsp::limit(p.k, .25F, 2, 1.414F);
    target_.drive = dsp::limit(p.drive, 1, 8, 1);
    target_.output = dsp::limit(p.output, .125F, 1, 1);
}
void Filter::reset() noexcept {
    one_ = {};
    two_ = {};
}
void Filter::process(float *samples, std::size_t frames, int channels) noexcept {
    if (!samples || (channels != 1 && channels != 2))
        return;
    if (mix_ == 0 && target_.values.mix == 0)
        return;
    for (std::size_t i = 0; i < frames; ++i) {
        dsp::smooth(g_, target_.g, smoothing_);
        dsp::smooth(k_, target_.k, smoothing_);
        dsp::smooth(drive_, target_.drive, smoothing_);
        dsp::smooth(output_, target_.output, smoothing_);
        dsp::smooth(mix_, target_.values.mix, smoothing_);
        for (unsigned m = 0; m < 3; ++m)
            dsp::smooth(weights_[m], target_.values.mode == static_cast<float>(m) ? 1.0F : 0.0F,
                        smoothing_);
        const double a1 = 1 / (1 + static_cast<double>(g_) * (g_ + k_)), a2 = g_ * a1, a3 = g_ * a2;
        for (unsigned ch = 0; ch < static_cast<unsigned>(channels); ++ch) {
            auto &out = samples[i * static_cast<unsigned>(channels) + ch];
            const auto dry = dsp::limit(out, -8, 8);
            double input = dry * drive_;
            if (drive_ > 1.0001F)
                input /= 1 + std::abs(input) * (drive_ - 1) / drive_;
            const double v3 = input - two_[ch], band = a1 * one_[ch] + a2 * v3,
                         low = two_[ch] + a2 * one_[ch] + a3 * v3;
            one_[ch] = 2 * band - one_[ch];
            two_[ch] = 2 * low - two_[ch];
            if (std::abs(one_[ch]) < 1e-20)
                one_[ch] = 0;
            if (std::abs(two_[ch]) < 1e-20)
                two_[ch] = 0;
            const double high = input - k_ * band - low;
            const auto wet = static_cast<float>(weights_[0] * low + weights_[1] * high +
                                                weights_[2] * k_ * band);
            out = dry + mix_ * (wet * output_ - dry);
        }
    }
    if (mix_ == 0)
        reset();
}
} // namespace grainmm
