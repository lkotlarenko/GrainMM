// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>
#include <vector>
namespace {
// Observable counter: optimized builds may otherwise cache it across external DSP calls.
volatile unsigned allocations = 0;
void check(bool ok, const char *message) {
    if (!ok)
        throw std::runtime_error(message);
}
std::vector<float> oscillator(grainmm::SynthColorParameters color, int rate = 48000) {
    grainmm::Oscillator synth(rate);
    grainmm::OscillatorParameters p;
    p.level = .15F;
    synth.apply(grainmm::prepare_oscillator(p, rate, color));
    std::vector<float> out(static_cast<unsigned>(rate));
    const auto before = allocations;
    synth.add(out.data(), out.size(), 1, true);
    check(allocations == before, "oscillator allocated in render");
    for (float value : out)
        check(std::isfinite(value) && std::abs(value) <= .41F, "unbounded synthesis");
    return out;
}
float energy(const std::vector<float> &v, unsigned first, unsigned last) {
    float sum = 0;
    for (unsigned i = first; i < last; ++i)
        sum += v[i] * v[i];
    return sum;
}
} // namespace
// Opaque allocation helpers keep GCC's optimizer from eliding the guard or
// misidentifying a legal malloc/free-backed replacement as mismatched new/delete.
// Attributes on the overloaded operators themselves trigger a GCC8 SDK bug.
namespace {
[[gnu::noinline]] void *counted_allocate(std::size_t n) {
    ++allocations;
    if (void *p = std::malloc(n ? n : 1))
        return p;
    throw std::bad_alloc();
}
[[gnu::noinline]] void counted_free(void *p) noexcept { std::free(p); }
} // namespace
void *operator new(std::size_t n) { return counted_allocate(n); }
void *operator new[](std::size_t n) { return counted_allocate(n); }
void operator delete(void *p) noexcept { counted_free(p); }
void operator delete[](void *p) noexcept { counted_free(p); }
void operator delete(void *p, std::size_t) noexcept { counted_free(p); }
void operator delete[](void *p, std::size_t) noexcept { counted_free(p); }
int main() {
    try {
        const auto base = oscillator({});
        for (const int rate : {8000, 44100, 48000, 96000}) {
            const auto pure = oscillator({1, 0, 0, 0, 0, 1}, rate);
            const auto bright = oscillator({1, 2, 0, 0, 0, 1}, rate);
            const auto scan = oscillator({1, 1.5F, .7F, 1, 0, 1}, rate);
            check(pure != bright && scan != bright && scan != pure, "morph/scan inaudible");
            grainmm::Oscillator whole(rate), split(rate);
            grainmm::OscillatorParameters p{.15F, 96, 2, 30, .1F, 100};
            const auto prepared = grainmm::prepare_oscillator(p, rate, {1, 3, 2, 1, .25F, 2});
            whole.apply(prepared);
            split.apply(prepared);
            std::vector<float> a(40000), b(40000);
            const auto before = allocations;
            whole.add(a.data(), 20000, 2, true);
            for (unsigned at = 0; at < 20000;) {
                const auto n = std::min(137U, 20000 - at);
                split.add(b.data() + at * 2, n, 2, true);
                at += n;
            }
            check(a == b && allocations == before, "table/sub block boundary or allocation");
            std::vector<float> tail(static_cast<unsigned>(rate) * 2);
            whole.add(tail.data(), static_cast<unsigned>(rate), 2, false);
            check(tail.back() == 0, "synth did not fade to silence");
        }
        check(energy(base, 24000, 48000) > 1, "base source silent");
        for (unsigned octave = 1; octave <= 2; ++octave) {
            grainmm::Oscillator sub(48000);
            const auto p = grainmm::prepare_oscillator({}, 48000, {0, 0, 0, 0, .1F, float(octave)});
            sub.apply(p);
            check(sub.enabled(), "sub-only source must suppress diagnostic tone");
            std::vector<float> out(96000);
            sub.add(out.data(), out.size(), 1, true);
            unsigned crossings = 0;
            for (unsigned i = 48001; i < out.size(); ++i)
                crossings += out[i - 1] <= 0 && out[i] > 0;
            check(crossings >= (octave == 1 ? 109U : 54U) &&
                      crossings <= (octave == 1 ? 111U : 56U),
                  "sub octave tuning");
        }
        grainmm::Instrument a(nullptr, 48000), b(nullptr, 48000), layered(nullptr, 48000);
        grainmm::InstrumentParameters p;
        p.oscillator = {.1F, 64, 0, 0, .5F, 0};
        a.set_parameters(p);
        p.oscillator_b = p.oscillator;
        p.oscillator.level = 0;
        b.set_parameters(p);
        p.oscillator = {.1F, 57, 0, 0, .5F, 0};
        layered.set_parameters(p);
        std::array<std::int16_t, 8192> x{}, y{}, z{};
        a.render(x.data(), 4096, 2, true);
        b.render(y.data(), 4096, 2, true);
        layered.render(z.data(), 4096, 2, true);
        check(x == y && z != y, "independent oscillator B or fallback changed");
        std::vector<float> input(32000, .7F);
        grainmm::Sample sample(8000, 1, std::move(input));
        grainmm::SlicePattern pattern;
        pattern.manual = true;
        for (unsigned i = 0; i < 16; ++i)
            pattern.steps[i].region = i;
        const grainmm::SliceParameters settings{1, 16, 2, .9F, 0, 0};
        // Verify timestamps and traversal against explicit expected results.
        for (unsigned order = 0; order < 3; ++order) {
            grainmm::SlicePlayer slices(sample, 8000);
            slices.apply(grainmm::prepare_slice(settings, 120, 8000, pattern,
                                                {.5F, 3, 1, float(order), .7F, 2}),
                         1, .5F, 0, 1);
            grainmm::FrameClock clock;
            clock.apply(grainmm::prepare_clock(120, 8000));
            constexpr unsigned expected[3][6] = {
                {1, 2, 0, 1, 2, 0}, {0, 2, 1, 0, 2, 1}, {1, 2, 0, 2, 1, 2}};
            unsigned events = 0;
            for (unsigned frame = 0; frame < 6000; ++frame) {
                float out = 0;
                const auto before = slices.triggered();
                slices.render(&out, 1, 1, true, clock);
                if (slices.triggered() != before) {
                    check(events < 6, "extra swing event");
                    const unsigned target = (events / 2) * 2000 + (events % 2 ? 1500 : 0);
                    check(frame >= target && frame <= target + 1, "swing clock drift");
                    check((slices.view() & 15U) == expected[order][events], "phrase traversal");
                    ++events;
                }
                clock.advance();
            }
            check(events == 6, "missing swing event");
        }
        grainmm::SlicePlayer straight(sample, 8000), accented(sample, 8000), split(sample, 8000);
        const auto control =
            grainmm::prepare_slice(settings, 120, 8000, pattern, {.4F, 1, 15, 2, .75F, 2});
        accented.apply(control, 1, .5F, 0, 1);
        split.apply(control, 1, .5F, 0, 1);
        auto unaccented = control;
        unaccented.groove.accent = 0;
        straight.apply(unaccented, 1, .5F, 0, 1);
        std::vector<float> plain(12000), groove(12000), chunks(12000);
        grainmm::FrameClock clock;
        clock.apply(grainmm::prepare_clock(120, 8000));
        const auto before = allocations;
        straight.render(plain.data(), plain.size(), 1, true, clock);
        accented.render(groove.data(), groove.size(), 1, true, clock);
        for (unsigned at = 0; at < chunks.size();) {
            const auto n = std::min(137U, static_cast<unsigned>(chunks.size()) - at);
            split.render(chunks.data() + at, n, 1, true, clock);
            clock.advance(n);
            at += n;
        }
        check(allocations == before && groove == chunks, "groove allocates or chunk dependent");
        check(energy(groove, 1600, 1900) < energy(plain, 1600, 1900) * .1F,
              "accent did not soften offbeats");
        const float nan = std::numeric_limits<float>::quiet_NaN();
        check(grainmm::bounded(grainmm::GrooveParameters{nan, nan, nan, nan, nan, nan}).length ==
                  16,
              "groove NaN defaults");
        p.groove = {.5F, 7, 3, 2, 1, 3};
        p.synth_color = {1, 2, 1, 1, .1F, 2};
        auto reset = grainmm::reset_effects(p, grainmm::ResetScope::Modulation);
        check(reset.groove.length == 16 && reset.synth_color.scan_depth == 0 &&
                  reset.synth_color.sub_level == .1F && reset.oscillator_b.level == .1F,
              "modulation reset lost base synthesis");
        std::puts(
            "v1: dual oscillator, tables/scan/sub, swing, phrases, accents, no allocations pass");
    } catch (const std::exception &e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
}
