// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/collision.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>
#include <vector>
namespace {
bool guard = false;
unsigned allocations = 0;
int fail(int line) {
    std::fprintf(stderr, "collision regression at line%d\n", line);
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
    for (int rate : {8000, 48000, 96000}) {
        grainmm::CollisionParameters p;
        p.enabled = 1;
        p.count = 6;
        p.rotation = .5F;
        p.level = .08F;
        const auto prepared = grainmm::prepare_collision(p, rate);
        grainmm::CollisionSynth a(rate), b(rate);
        a.apply(prepared);
        b.apply(prepared);
        std::vector<float> whole(static_cast<unsigned>(rate) * 4), split(whole.size());
        guard = true;
        a.add(whole.data(), static_cast<unsigned>(rate) * 2, 2, true);
        guard = false;
        if (allocations)
            return fail(__LINE__);
        for (unsigned at = 0; at < static_cast<unsigned>(rate) * 2;) {
            auto n = std::min(127U, static_cast<unsigned>(rate) * 2 - at);
            b.add(split.data() + at * 2, n, 2, true);
            at += n;
        }
        if (whole != split || a.hits() == 0 || a.hits() != b.hits() ||
            a.positions() != b.positions())
            return fail(__LINE__);
        float max = 0;
        for (float value : whole) {
            if (!std::isfinite(value) || std::abs(value) > 1)
                return fail(__LINE__);
            max = std::max(max, std::abs(value));
        }
        if (max < .00001F)
            return fail(__LINE__);
        for (auto pos : a.positions())
            if ((pos & (1U << 16)) == 0)
                return fail(__LINE__);
        std::fill(whole.begin(), whole.end(), 0);
        a.add(whole.data(), static_cast<unsigned>(rate), 2, false);
        if (std::any_of(whole.begin() + static_cast<unsigned>(rate) / 10, whole.begin() + rate * 2,
                        [](float v) { return v != 0; }))
            return fail(__LINE__);
        p.enabled = 0;
        a.apply(grainmm::prepare_collision(p, rate));
        if (a.enabled() || a.positions() != std::array<unsigned, 6>{})
            return fail(__LINE__);
    }
    // Sustained musical rebounds must keep spending time away from the walls.
    grainmm::CollisionParameters moving;
    moving.enabled = 1;
    moving.count = 1;
    grainmm::CollisionSynth world(8000);
    world.apply(grainmm::prepare_collision(moving, 8000));
    std::vector<float> buffer(1600);
    unsigned interior = 0, changed = 0, previous = 0;
    for (unsigned i = 0; i < 200; ++i) {
        std::fill(buffer.begin(), buffer.end(), 0);
        world.add(buffer.data(), 800, 2, true);
        const auto pos = world.positions()[0];
        const float x = static_cast<float>(pos & 255U) / 127.5F - 1,
                    y = static_cast<float>((pos >> 8) & 255U) / 127.5F - 1;
        if (i > 20 && std::abs(x) < .55F && std::abs(y) < .55F)
            ++interior;
        if (pos != previous)
            ++changed;
        previous = pos;
    }
    if (interior < 5 || changed < 100)
        return fail(__LINE__);
    std::puts("Collision physics/audio chunking, bounds, telemetry, real hits and pause passed");
}
