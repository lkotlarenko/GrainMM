// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument_parameters.hpp"
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <vector>
namespace {
void check(bool ok) {
    if (!ok)
        throw std::runtime_error("manual pattern regression");
}
} // namespace
int main() {
    try {
        grainmm::SlicePattern p;
        grainmm::PatternEditor editor;
        check(!editor.input(grainmm::PatternKey::Next, p, 8) && editor.step() == 1);
        check(editor.input(grainmm::PatternKey::Rest, p, 8));
        check(p.manual && !p.steps[1].enabled);
        editor.input(grainmm::PatternKey::Down, p, 8);
        editor.input(grainmm::PatternKey::Down, p, 8);
        check(editor.input(grainmm::PatternKey::Right, p, 8) && p.steps[1].reverse);
        editor.input(grainmm::PatternKey::Down, p, 8);
        check(editor.input(grainmm::PatternKey::Left, p, 8) && p.steps[1].probability == .95F);
        editor.input(grainmm::PatternKey::Down, p, 8);
        const auto before = grainmm::slice_cuts(p, 8);
        check(editor.input(grainmm::PatternKey::Left, p, 8));
        const auto after = grainmm::slice_cuts(p, 8);
        check(after[2] < before[2] && std::abs(after[3] - before[3]) < 1e-6F);
        // Moving an extreme-width pair must not change unrelated cuts when
        // bounding the resulting stored widths, including the legal endpoints.
        for (float width : {.001F, 16.F}) {
            grainmm::SlicePattern edge;
            edge.manual = true;
            edge.widths[0] = edge.widths[1] = width;
            grainmm::PatternEditor cut_editor;
            for (unsigned i = 0; i < 4; ++i)
                cut_editor.input(grainmm::PatternKey::Down, edge, 8);
            const auto original = grainmm::slice_cuts(edge, 8);
            cut_editor.input(grainmm::PatternKey::Left, edge, 8, 8);
            const auto edited = grainmm::slice_cuts(edge, 8);
            for (unsigned i = 2; i <= 8; ++i)
                check(std::abs(original[i] - edited[i]) < 1e-6F);
        }
        for (unsigned n = 2; n <= 16; ++n) {
            const auto cuts = grainmm::slice_cuts(p, n);
            check(cuts[0] == 0 && cuts[n] == 1);
            for (unsigned i = 1; i <= n; ++i)
                check(cuts[i] > cuts[i - 1]);
        }
        check(editor.input(grainmm::PatternKey::Mode, p, 8) && !p.manual);
        const auto stored = p.steps;
        check(editor.input(grainmm::PatternKey::Mode, p, 8) && p.manual);
        check(p.steps[1].enabled == stored[1].enabled);
        p.widths[0] = std::numeric_limits<float>::quiet_NaN();
        p.steps[0].region = 999;
        p.steps[0].probability = -2;
        p = grainmm::bounded(p);
        check(p.widths[0] == 1 && p.steps[0].region == 15 && p.steps[0].probability == 0);
        p = {};
        p.manual = true;
        for (unsigned i = 0; i < 16; ++i) {
            p.steps[i].region = (3 - i % 4);
            p.steps[i].enabled = i % 3 != 1;
            p.steps[i].reverse = i % 2 != 0;
            p.steps[i].probability = i % 5 == 0 ? .5F : 1.F;
        }
        p.widths[0] = .5F;
        p.widths[1] = 1.5F;
        grainmm::SliceParameters settings{1, 4, 2, 1, 0, 0};
        const auto prepared = grainmm::prepare_slice(settings, 120, 8000, p);
        std::vector<float> input(4000);
        for (unsigned i = 0; i < input.size(); ++i)
            input[i] = .1F * std::sin(static_cast<float>(i) * .1F);
        grainmm::Sample sample(8000, 1, std::move(input));
        grainmm::SlicePlayer whole(sample, 8000), split(sample, 8000);
        whole.apply(prepared, 1, .5F, .1F, .9F);
        split.apply(prepared, 1, .5F, .1F, .9F);
        grainmm::FrameClock clock;
        clock.apply(grainmm::prepare_clock(120, 8000));
        std::vector<float> a(40000), b(40000);
        whole.render(a.data(), 20000, 2, true, clock);
        for (unsigned at = 0; at < 20000;) {
            const auto frames = std::min(137U, 20000 - at);
            split.render(b.data() + at * 2, frames, 2, true, clock);
            clock.advance(frames);
            at += frames;
        }
        check(a == b && whole.triggered() == split.triggered());
        check(whole.triggered() > 0 && whole.triggered() < 20);
        grainmm::InstrumentParameters sound;
        sound.slice_pattern = p;
        const auto rack = grainmm::reset_effects(sound);
        check(rack.slice_pattern == p);
        const auto mod = grainmm::reset_effects(sound, grainmm::ResetScope::Modulation);
        check(mod.slice_pattern.manual && mod.slice_pattern.widths == p.widths);
        for (const auto &step : mod.slice_pattern.steps)
            check(!step.reverse && step.probability == 1);
        check(!grainmm::reset_effects(sound, grainmm::ResetScope::Sound).slice_pattern.manual);
        std::puts("manual pattern: edits, cuts, rests/reverse/chance and block determinism pass");
    } catch (const std::exception &e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
}
