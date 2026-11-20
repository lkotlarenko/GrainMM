// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/drone_synth.hpp"
#include "grainmm/instrument.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <vector>
namespace {
bool guard = false;
unsigned allocations = 0;
int fail(int n) {
    std::fprintf(stderr, "Drone synth regression line%d\n", n);
    return 1;
}
} // namespace
void *operator new(std::size_t n) {
    if (guard)
        ++allocations;
    if (auto *p = std::malloc(n ? n : 1))
        return p;
    throw std::bad_alloc();
}
void *operator new[](std::size_t n) { return ::operator new(n); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }
int main() {
    using namespace grainmm;
    for (int rate : {8000, 48000, 96000}) {
        DroneSynthParameters p{.1F, 48, 0, 4, .6F, .8F};
        auto prepared = prepare_drone_synth(p, rate);
        DroneSynth a(rate), b(rate);
        a.apply(prepared);
        b.apply(prepared);
        const unsigned frames = static_cast<unsigned>(rate) * 2;
        std::vector<float> one(frames * 2), split(one.size());
        guard = true;
        a.add(one.data(), frames, 2, true);
        for (unsigned at = 0; at < frames;) {
            auto n = std::min(173U, frames - at);
            b.add(split.data() + at * 2, n, 2, true);
            at += n;
        }
        guard = false;
        if (allocations || one != split)
            return fail(__LINE__);
        double power = 0, mean = 0;
        float peak = 0;
        bool stereo = false;
        for (unsigned i = 0; i < one.size(); ++i) {
            if (!std::isfinite(one[i]))
                return fail(__LINE__);
            power += one[i] * one[i];
            mean += one[i];
            peak = std::max(peak, std::abs(one[i]));
            if (i % 2 && one[i] != one[i - 1])
                stereo = true;
        }
        if (peak > .1F || power / static_cast<double>(one.size()) < .000001 ||
            std::abs(mean / static_cast<double>(one.size())) > .002 || !stereo)
            return fail(__LINE__);
        std::fill(split.begin(), split.end(), 0);
        a.reset();
        a.add(split.data(), frames, 2, true);
        if (one != split)
            return fail(__LINE__);
        std::array<float, 512> block{};
        guard = true;
        for (int i = 0; i < rate * 8 / 256 + 1; ++i) {
            block.fill(0);
            a.add(block.data(), 256, 2, true);
        }
        guard = false;
        if (allocations || a.changes() < 2 || (a.visual_state() & (1U << 24)) == 0)
            return fail(__LINE__);
        for (int i = 0; i < rate / 256 + 1; ++i) {
            block.fill(0);
            a.add(block.data(), 256, 2, false);
        }
        const auto view = a.visual_state();
        block.fill(0);
        a.add(block.data(), 256, 2, false);
        if (a.visual_state() != view ||
            std::any_of(block.begin(), block.end(), [](float x) { return x != 0; }))
            return fail(__LINE__);
        for (float mood : {1.F, 2.F, 3.F}) {
            p.mood = mood;
            p.root = 60;
            p.level = .2F;
            a.apply(prepare_drone_synth(p, rate));
            block.fill(0);
            a.add(block.data(), 256, 2, true);
            for (auto x : block)
                if (!std::isfinite(x) || std::abs(x) > .2F)
                    return fail(__LINE__);
        }
    }
    Instrument dry(nullptr, 48000), wet(nullptr, 48000);
    InstrumentParameters p;
    p.drone_synth.level = .1F;
    dry.set_parameters(p);
    p.reverb.mix = .4F;
    wet.set_parameters(p);
    std::vector<std::int16_t> plain(96000), effect(96000);
    guard = true;
    dry.render(plain.data(), 48000, 2, true);
    wet.render(effect.data(), 48000, 2, true);
    guard = false;
    if (allocations || plain == effect || dry.clipped_samples() || wet.clipped_samples())
        return fail(__LINE__);
    auto rack = reset_effects(p);
    if (rack.drone_synth.level != .1F)
        return fail(__LINE__);
    auto modulation = reset_effects(p, ResetScope::Modulation);
    if (modulation.drone_synth.movement != 0 || modulation.drone_synth.level != .1F)
        return fail(__LINE__);
    if (reset_effects(p, ResetScope::Sound).drone_synth.level != 0)
        return fail(__LINE__);
    // Usable level headroom: preserve20% scenes, allow a genuinely louder60%.
    DroneSynth quiet(48000), loud(48000);
    DroneSynthParameters level;
    level.level = .2F;
    level.event_mix = 0;
    quiet.apply(prepare_drone_synth(level, 48000));
    level.level = .6F;
    loud.apply(prepare_drone_synth(level, 48000));
    std::vector<float> q(192000), l(q.size());
    quiet.add(q.data(), 96000, 2, true);
    loud.add(l.data(), 96000, 2, true);
    double qp = 0, lp = 0;
    for (unsigned i = 96000; i < q.size(); ++i) {
        qp += q[i] * q[i];
        lp += l[i] * l[i];
    }
    if (qp <= 0 || std::abs(std::sqrt(lp / qp) - 3) > .01)
        return fail(__LINE__);
    // Every new pattern/seed/event control changes real audio-owned state.
    for (unsigned pattern = 0; pattern < 5; ++pattern) {
        DroneSynth gen(8000);
        DroneSynthParameters x;
        x.level = .8F;
        x.event_mix = 1;
        x.event_rate = 2;
        x.seed = 314;
        x.pattern = static_cast<float>(pattern);
        x.evolve_seconds = 4;
        x.stereo = 0;
        gen.apply(prepare_drone_synth(x, 8000));
        std::array<float, 512> work{};
        bool flashes = false;
        guard = true;
        for (unsigned n = 0; n < 129; ++n) {
            work.fill(0);
            gen.add(work.data(), 256, 2, true);
            flashes = flashes || gen.event_levels() != 0;
            for (unsigned i = 0; i < work.size(); i += 2)
                if (!std::isfinite(work[i]) || std::abs(work[i]) > 1 || work[i] != work[i + 1])
                    return fail(__LINE__);
        }
        guard = false;
        const unsigned degree = gen.visual_state() & 3U;
        if (allocations || !flashes || gen.event_count() == 0 || (pattern == 1 && degree != 1) ||
            (pattern == 2 && degree != 3) || (pattern == 3 && degree != 1) ||
            (pattern == 4 && degree != 0))
            return fail(__LINE__);
    }
    for (unsigned mood = 4; mood < 8; ++mood) {
        DroneSynth high(48000);
        DroneSynthParameters x;
        x.level = 1;
        x.root = 84;
        x.mood = static_cast<float>(mood);
        high.apply(prepare_drone_synth(x, 48000));
        std::array<float, 512> work{};
        high.add(work.data(), 256, 2, true);
        for (auto value : work)
            if (!std::isfinite(value) || std::abs(value) > 1)
                return fail(__LINE__);
    }
    std::puts("Drone synth: standalone stereo power/bounds, deterministic chunk/reset, autonomous "
              "changes, pause, allocation and shared FX/reset passed");
}
