// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/drone.hpp"
#include "grainmm/granular.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>
#include <vector>
namespace {
unsigned allocations = 0;
bool guarded = false;
int fail(int line) {
    std::fprintf(stderr, "Drone regression line%d\n", line);
    return 1;
}
} // namespace
void *operator new(std::size_t n) {
    if (guarded)
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
    DroneParameters p;
    p.mode = 1;
    p.rate_hz = .5F;
    p.position_depth = 1;
    DroneMotion wave(48000);
    wave.apply(p);
    DroneFrame frame;
    for (unsigned i = 0; i < 24000; ++i)
        frame = wave.next(true);
    if (std::abs(frame.position - 1) > .001F)
        return fail(__LINE__);
    p.hold = 1;
    wave.apply(p);
    for (unsigned i = 0; i < 2000; ++i)
        frame = wave.next(true);
    const auto held = frame.position;
    for (unsigned i = 0; i < 96000; ++i)
        if (wave.next(true).position != held || wave.next(false).position != held)
            return fail(__LINE__);
    p.mode = 0;
    wave.apply(p);
    for (unsigned i = 0; i < 2000; ++i)
        frame = wave.next(true);
    if (frame.position != 0 || frame.pitch_cents != 0 || frame.spread != 0)
        return fail(__LINE__);
    for (int rate : {8000, 48000, 96000}) {
        for (unsigned mode = 1; mode <= 3; ++mode) {
            p = {};
            p.mode = static_cast<float>(mode);
            p.rate_hz = .5F;
            p.pitch_cents = 400;
            p.spread_depth = .3F;
            p.position_depth = .4F;
            DroneMotion a(rate), b(rate);
            a.apply(p);
            b.apply(p);
            guarded = true;
            for (int i = 0; i < rate * 4; ++i) {
                auto x = a.next(true), y = b.next(true);
                if (x.position != y.position || x.pitch_cents != y.pitch_cents ||
                    x.spread != y.spread || !std::isfinite(x.position) ||
                    std::abs(x.position) > .4F || std::abs(x.pitch_cents) > 400 ||
                    std::abs(x.spread) > .3F)
                    return fail(__LINE__);
            }
            guarded = false;
            if (allocations)
                return fail(__LINE__);
        }
        std::vector<float> values(8192);
        for (unsigned i = 0; i < values.size(); ++i)
            values[i] =
                .7F * std::sin(static_cast<float>(i) * (.01F + static_cast<float>(i) * .00001F));
        Sample sample(rate, 1, std::move(values));
        Granular whole(sample, rate), chunks(sample, rate), dry(sample, rate), bypass(sample, rate);
        p.mode = 3;
        whole.set_drone(p);
        chunks.set_drone(p);
        auto off = p;
        off.mode = 0;
        off.hold = 1;
        bypass.set_drone(off);
        std::vector<std::int16_t> one(static_cast<unsigned>(rate) * 4), split(one.size()),
            base(one.size()), off_pcm(one.size());
        guarded = true;
        whole.render(one.data(), static_cast<unsigned>(rate) * 2, 2, true);
        for (unsigned at = 0; at < static_cast<unsigned>(rate) * 2;) {
            auto count = std::min(137U, static_cast<unsigned>(rate) * 2 - at);
            chunks.render(split.data() + at * 2, count, 2, true);
            at += count;
        }
        dry.render(base.data(), static_cast<unsigned>(rate) * 2, 2, true);
        bypass.render(off_pcm.data(), static_cast<unsigned>(rate) * 2, 2, true);
        guarded = false;
        if (allocations || one != split || base != off_pcm || base == one ||
            whole.spawned() != dry.spawned() || whole.dropped() != dry.dropped())
            return fail(__LINE__);
        whole.reset();
        whole.render(split.data(), static_cast<unsigned>(rate) * 2, 2, true);
        if (one != split)
            return fail(__LINE__);
    }
    p.mode = std::numeric_limits<float>::infinity();
    p.pitch_cents = std::numeric_limits<float>::quiet_NaN();
    auto safe = bounded(p);
    if (safe.mode != 0 || safe.pitch_cents != 0)
        return fail(__LINE__);
    std::puts("Drone: wave reference, seeded drift, hold/pause/bypass, ramps, PCM chunk/reset and "
              "allocation guards passed");
}
