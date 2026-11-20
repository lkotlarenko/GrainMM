// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/slice_pattern.hpp"
#include "grainmm/dsp_utils.hpp"
namespace grainmm {
SlicePattern bounded(SlicePattern p) noexcept {
    for (auto &s : p.steps) {
        s.region = std::min(s.region, 15U);
        s.probability = dsp::limit(s.probability, 0, 1, 1);
    }
    for (auto &w : p.widths)
        w = dsp::limit(w, .001F, 16, 1);
    return p;
}
bool operator==(const SlicePattern &a, const SlicePattern &b) noexcept {
    if (a.manual != b.manual || a.widths != b.widths)
        return false;
    for (unsigned i = 0; i < a.steps.size(); ++i) {
        const auto &x = a.steps[i], &y = b.steps[i];
        if (x.region != y.region || x.enabled != y.enabled || x.reverse != y.reverse ||
            x.probability != y.probability)
            return false;
    }
    return true;
}
std::array<float, 17> slice_cuts(const SlicePattern &input, unsigned regions) noexcept {
    const auto p = bounded(input);
    regions = std::clamp(regions, 2U, 16U);
    float total = 0, at = 0;
    for (unsigned i = 0; i < regions; ++i)
        total += p.widths[i];
    std::array<float, 17> cuts{};
    for (unsigned i = 0; i < regions; ++i) {
        at += p.widths[i];
        cuts[i + 1] = at / total;
    }
    cuts[regions] = 1;
    return cuts;
}
bool PatternEditor::input(PatternKey key, SlicePattern &p, unsigned regions, unsigned multiplier) {
    regions = std::clamp(regions, 2U, 16U);
    multiplier = std::clamp(multiplier, 1U, 8U);
    if (key == PatternKey::Previous || key == PatternKey::Next) {
        step_ = (step_ + (key == PatternKey::Previous ? 15U : 1U)) % 16;
        return false;
    }
    if (key == PatternKey::Up || key == PatternKey::Down) {
        field_ = (field_ + (key == PatternKey::Up ? 4U : 1U)) % 5;
        return false;
    }
    const auto before = p;
    p = bounded(p);
    auto &s = p.steps[step_];
    if (key == PatternKey::Mode)
        p.manual = !p.manual;
    else if (key == PatternKey::Rest) {
        s.enabled = !s.enabled;
        p.manual = true;
    } else if (key == PatternKey::Left || key == PatternKey::Right) {
        const int sign = key == PatternKey::Left ? -1 : 1;
        if (field_ == 0)
            s.region = static_cast<unsigned>(
                std::clamp(static_cast<int>(std::min(s.region, regions - 1)) + sign, 0,
                           static_cast<int>(regions) - 1));
        if (field_ == 1)
            s.enabled = sign > 0;
        if (field_ == 2)
            s.reverse = sign > 0;
        if (field_ == 3)
            s.probability = std::clamp(s.probability + static_cast<float>(sign) * .05F *
                                                           static_cast<float>(multiplier),
                                       0.F, 1.F);
        if (field_ == 4) {
            const auto region = std::min(s.region, regions - 1);
            if (region + 1 < regions) {
                float total = 0;
                for (unsigned i = 0; i < regions; ++i)
                    total += p.widths[i];
                const float pair = p.widths[region] + p.widths[region + 1];
                const float edge = std::min(pair * .5F, std::max(.001F, total * .005F));
                const float low = std::max(edge, pair - 16.F);
                const float high = std::min(16.F, pair - edge);
                p.widths[region] =
                    std::clamp(p.widths[region] + static_cast<float>(sign) * total * .01F *
                                                      static_cast<float>(multiplier),
                               low, high);
                p.widths[region + 1] = pair - p.widths[region];
            }
        }
        p.manual = true;
    }
    p = bounded(p);
    return p != before;
}
} // namespace grainmm
