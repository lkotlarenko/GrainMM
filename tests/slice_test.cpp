// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument.hpp"
#include "grainmm/slice.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <vector>
namespace {
bool guard = false;
unsigned allocations = 0;
void check(bool ok) {
    if (!ok)
        throw std::runtime_error("slice/clock regression");
}
} // namespace
void *operator new(std::size_t size) {
    if (guard)
        ++allocations;
    if (auto *p = std::malloc(size ? size : 1))
        return p;
    throw std::bad_alloc();
}
void *operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }
int main() {
    try {
        for (const int rate : {8000, 48000, 96000})
            for (const float bpm : {20.F, 123.F, 300.F}) {
                grainmm::FrameClock clock;
                clock.apply(grainmm::prepare_clock(bpm, rate));
                unsigned ticks = 0;
                for (unsigned f = 0; f < static_cast<unsigned>(rate) * 4; ++f) {
                    if (clock.boundary(2)) {
                        const double expected = static_cast<double>(ticks) * rate * 60 / (bpm * 4);
                        check(std::abs(static_cast<double>(f) - expected) < 1.01);
                        ++ticks;
                    }
                    clock.advance();
                }
                check(ticks >= 5);
                auto copy = clock;
                auto phase = clock.phase();
                clock.apply(grainmm::prepare_clock(200, rate));
                check(clock.phase() == phase);
                copy.reset();
                check(copy.phase() == 0 && copy.boundary(0));
            }
        // Integer phase gives the same result for differently grouped virtual time.
        grainmm::FrameClock a, b;
        a.apply(grainmm::prepare_clock(123.456F, 48000));
        b = a;
        for (unsigned i = 0; i < 1000000; ++i)
            a.advance();
        for (unsigned i = 0; i < 1000; ++i)
            b.advance(1000);
        check(a.phase() == b.phase() && a.beats() == b.beats());
        // Worst-case Q48 rounding drift over one virtual hour stays well below a frame.
        for (const int rate : {8000, 48000, 96000}) {
            const auto prepared_clock = grainmm::prepare_clock(123.456F, rate);
            const long double actual = static_cast<long double>(prepared_clock.increment) * rate *
                                       3600 / grainmm::FrameClock::unit;
            const long double ideal = static_cast<long double>(123.456F) * 60;
            check(std::abs(actual - ideal) < 1e-5L);
        }
        bool invalid = false;
        try {
            (void)grainmm::prepare_clock(120, 0);
        } catch (const std::invalid_argument &) {
            invalid = true;
        }
        check(invalid);
        grainmm::SliceParameters p;
        p.mode = 1;
        p.regions = 4;
        p.division = 2;
        p.gate = .8F;
        auto prepared = grainmm::prepare_slice(p, 120, 8000);
        for (unsigned i = 0; i < 16; ++i)
            check(prepared.pattern.steps[i].region == i % 4);
        p.mode = 2;
        auto backward = grainmm::prepare_slice(p, 120, 8000);
        for (unsigned i = 0; i < 16; ++i)
            check(backward.pattern.steps[i].region == 3 - i % 4);
        p.mode = 3;
        auto bounce = grainmm::prepare_slice(p, 120, 8000);
        const unsigned expected[] = {0, 1, 2, 3, 2, 1, 0, 1};
        for (unsigned i = 0; i < 8; ++i)
            check(bounce.pattern.steps[i].region == expected[i]);
        p.mode = 4;
        auto shuffled = grainmm::prepare_slice(p, 120, 8000);
        for (const auto &s : shuffled.pattern.steps)
            check(s.region < 4);
        std::vector<float> values(4000);
        for (unsigned i = 0; i < 4000; ++i)
            values[i] = .1F * static_cast<float>(i / 1000 + 1);
        grainmm::Sample sample(8000, 1, std::move(values));
        grainmm::SlicePlayer whole(sample, 8000), split(sample, 8000);
        whole.apply(prepared, 1, .5F, 0, 1);
        split.apply(prepared, 1, .5F, 0, 1);
        grainmm::FrameClock clock;
        clock.apply(grainmm::prepare_clock(120, 8000));
        std::vector<float> output(8000), chunks(8000);
        guard = true;
        whole.render(output.data(), 4000, 2, true, clock);
        guard = false;
        check(allocations == 0 && whole.triggered() == 4);
        for (unsigned region = 0; region < 4; ++region)
            check(std::abs(output[(region * 1000 + 300) * 2] -
                           .0125F * static_cast<float>(region + 1)) < 1e-5F);
        for (unsigned at = 0; at < 4000;) {
            const auto n = std::min(137U, 4000 - at);
            split.render(chunks.data() + at * 2, n, 2, true, clock);
            clock.advance(n);
            at += n;
        }
        check(output == chunks && split.triggered() == whole.triggered());
        split.render(chunks.data(), 1000, 2, false, clock);
        check(std::all_of(chunks.begin() + 160 * 2, chunks.begin() + 2000,
                          [](float v) { return v == 0; }));
        for (const auto mode : {1.F, 2.F, 3.F, 4.F})
            for (const auto pitch : {.25F, 2.F}) {
                p.mode = mode;
                p.variation = 2;
                p.reverse = 1;
                split.reset();
                split.apply(grainmm::prepare_slice(p, 300, 8000), pitch, .5F, .99F, 1);
                clock.reset();
                clock.apply(grainmm::prepare_clock(300, 8000));
                split.render(chunks.data(), 4000, 2, true, clock);
                for (float value : chunks)
                    check(std::isfinite(value) && std::abs(value) <= .1F);
            }
        grainmm::InstrumentParameters reset_source;
        reset_source.rhythm.bpm = 175;
        reset_source.slice = {2, 9, 3, .5F, 2, .7F};
        const auto rack = grainmm::reset_effects(reset_source);
        const auto modulation =
            grainmm::reset_effects(reset_source, grainmm::ResetScope::Modulation);
        check(rack.rhythm.bpm == 175 && rack.slice.variation == 2 && rack.slice.reverse == .7F);
        check(modulation.rhythm.bpm == 175 && modulation.slice.mode == 2 &&
              modulation.slice.regions == 9 && modulation.slice.variation == 0 &&
              modulation.slice.reverse == 0);
        // Integrated mode transition has fixed storage and remains finite/bounded.
        grainmm::Instrument instrument(sample, 8000);
        grainmm::InstrumentParameters params;
        std::vector<std::int16_t> pcm(4000);
        instrument.set_parameters(params);
        instrument.render(pcm.data(), 2000, 2, true);
        params.slice.mode = 1;
        instrument.set_parameters(params);
        guard = true;
        instrument.render(pcm.data(), 2000, 2, true);
        guard = false;
        check(allocations == 0 && instrument.slices_triggered() > 0 &&
              (instrument.sequence_view() & (1U << 16)) != 0);
        params.slice.mode = 0;
        instrument.set_parameters(params);
        instrument.render(pcm.data(), 2000, 2, true);
        check(instrument.sequence_view() == 0);
        grainmm::Instrument contiguous(sample, 8000), fragmented(sample, 8000);
        std::vector<std::int16_t> reference(12000), divided(12000);
        grainmm::InstrumentParameters transition;
        for (unsigned stage = 0; stage < 3; ++stage) {
            transition.slice.mode = stage == 1 ? 1.F : 0.F;
            contiguous.set_parameters(transition);
            fragmented.set_parameters(transition);
            contiguous.render(reference.data() + stage * 4000, 2000, 2, true);
            for (unsigned at = 0; at < 2000;) {
                const auto n = std::min(71U, 2000 - at);
                fragmented.render(divided.data() + stage * 4000 + at * 2, n, 2, true);
                at += n;
            }
        }
        check(reference == divided);
        params.slice.mode = 1;
        params.slice_pattern.manual = true;
        params.slice_pattern.steps[1].enabled = false;
        params.slice_pattern.steps[2].probability = .5F;
        const auto manual = grainmm::prepare_parameters(params, 8000);
        guard = true;
        instrument.apply(manual);
        instrument.render(pcm.data(), 2000, 2, true);
        guard = false;
        check(allocations == 0);
        std::puts("clock boundaries/phase, patterns, numeric regions, chunking, reverse, pause and "
                  "allocation checks passed");
        return 0;
    } catch (const std::exception &e) {
        guard = false;
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
}
