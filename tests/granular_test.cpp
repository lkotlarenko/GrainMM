// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/granular.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>
#include <vector>

namespace {
std::size_t allocations = 0;
void require(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
template <typename F> void rejects(F action, const char *message) {
    bool rejected = false;
    try {
        action();
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    require(rejected, message);
}
} // namespace
void *operator new(std::size_t size) {
    ++allocations;
    if (void *p = std::malloc(size ? size : 1))
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
        rejects([] { grainmm::Sample sample(48000, 1, {}); }, "empty sample accepted");
        rejects([] { grainmm::Sample sample(0, 1, {0}); }, "bad sample rate accepted");
        rejects([] { grainmm::Sample sample(48000, 2, {0}); }, "unaligned stereo accepted");
        rejects([] { grainmm::Sample sample(48000, 1, {2}); }, "unbounded float accepted");
        rejects([] { grainmm::Sample sample(48000, 1, {std::numeric_limits<float>::infinity()}); },
                "infinite sample accepted");
        const float nan = std::numeric_limits<float>::quiet_NaN();
        rejects([&] { grainmm::Sample sample(48000, 1, {nan}); }, "NaN sample accepted");
        const auto invalid =
            grainmm::bounded(grainmm::GrainParameters{nan, nan, nan, nan, nan, nan});
        require(invalid.gain == 0 && invalid.density == 12, "nonfinite parameters not guarded");
        const auto bounds = grainmm::bounded(grainmm::GrainParameters{-1, 10000, -1, 2, -1, 100});
        require(bounds.size_ms == 5 && bounds.density == 80 && bounds.pitch == 0.25F &&
                    bounds.position == 1 && bounds.spread == 0 && bounds.gain == 0.5F,
                "parameter bounds");
        // Independent cosine envelope reference guards inner-loop optimization.
        for (const int rate : {8000, 44100, 48000, 96000}) {
            for (const float size_ms : {5.0F, 80.0F, 250.0F}) {
                const grainmm::Sample constant(rate, 1, {1});
                grainmm::Granular envelope(constant, rate);
                grainmm::GrainParameters p;
                p.size_ms = size_ms;
                p.density = 1;
                envelope.set_parameters(p);
                const auto duration = static_cast<std::size_t>(size_ms * 0.001 * rate);
                std::vector<float> actual(duration);
                envelope.render_float(actual.data(), duration, 1, true);
                for (std::size_t i = 0; i < duration; ++i) {
                    const double gain =
                        std::min(0.2, static_cast<double>(i + 1) * 0.5 / (0.02 * rate));
                    const double hann =
                        0.5 - 0.5 * std::cos(6.283185307179586 * static_cast<double>(i) /
                                             static_cast<double>(duration - 1));
                    require(std::abs(actual[i] - hann * gain / 16) < 3e-7,
                            "grain envelope differs from analytical Hann reference");
                }
                require(actual.front() == 0 && actual.back() == 0, "envelope endpoint");
            }
        }
        for (const int output_rate : {8000, 44100, 48000, 96000}) {
            std::vector<float> values(1001 * 2);
            for (std::size_t i = 0; i < 1001; ++i) {
                values[i * 2] = static_cast<float>(std::sin(static_cast<double>(i) * 0.03));
                values[i * 2 + 1] = -values[i * 2];
            }
            const grainmm::Sample sample(44100, 2, std::move(values));
            grainmm::Granular whole(sample, output_rate, 42), chunked(sample, output_rate, 42);
            grainmm::GrainParameters parameters;
            parameters.density = 7;
            parameters.pitch = 1.7F;
            whole.set_parameters(parameters);
            chunked.set_parameters(parameters);
            const auto frames = static_cast<std::size_t>(output_rate) * 2;
            std::vector<std::int16_t> a(frames * 2, 99), b(a.size(), 99);
            whole.render(a.data(), frames, 2, false);
            require(std::all_of(a.begin(), a.end(), [](auto v) { return v == 0; }), "silent start");
            const auto before = allocations;
            whole.render(a.data(), frames, 2, true);
            for (std::size_t at = 0; at < frames;) {
                const auto count = std::min<std::size_t>(137, frames - at);
                chunked.render(b.data() + at * 2, count, 2, true);
                at += count;
            }
            require(allocations == before, "allocation in grain renderer");
            require(a == b, "block-dependent scheduling/randomness");
            require(whole.spawned() == 14 && whole.dropped() == 0, "sample clock schedule");
            require(std::any_of(a.begin(), a.end(), [](auto v) { return v != 0; }),
                    "inaudible renderer");
            for (std::size_t i = 0; i < frames; ++i) {
                require(std::abs(static_cast<int>(a[i * 2])) <= 16384,
                        "output exceeds safe gain bound");
                require(a[i * 2] == -a[i * 2 + 1], "stereo channel identity lost");
            }
            whole.render(a.data(), frames, 2, false);
            require(a.back() == 0, "mute did not settle");
            whole.reset();
            whole.render(a.data(), frames, 2, true);
            require(a == b, "reset does not reproduce seeded output");
            whole.render(nullptr, 0, 2, false);
            std::int16_t guard = 111;
            whole.render(&guard, 1, 3, true);
            require(guard == 111, "invalid channel guard overwritten");
        }
        // Backward grains must wrap before the next sample read, including
        // one-frame buffers and source/output-rate extremes.
        for (const int source_rate : {8000, 96000}) {
            const grainmm::Sample ramp(source_rate, 1, {0, 0.3F, 0.6F, -0.3F, -0.6F});
            grainmm::Granular forward(ramp, source_rate), reverse(ramp, source_rate);
            grainmm::GrainParameters motion{5, 1, 1, 0, 0, 0.2F};
            forward.set_parameters(motion);
            motion.reverse = 1;
            reverse.set_parameters(motion);
            std::vector<float> a(512), b(512);
            forward.render_float(a.data(), 256, 2, true);
            reverse.render_float(b.data(), 256, 2, true);
            require(a != b, "reverse did not change playback");
            motion.reverse = 0.5F;
            motion.pitch_jitter = 12;
            motion.size_jitter = 1;
            motion.timing_jitter = 1;
            motion.pan_spread = 1;
            motion.scan_hz = -0.5F;
            motion.density = 80;
            motion.size_ms = 250;
            motion.pitch = 2;
            grainmm::Granular whole(ramp, 8000, 123), chunked(ramp, 8000, 123);
            whole.set_parameters(motion);
            chunked.set_parameters(motion);
            a.resize(32000);
            b.resize(32000);
            const auto before = allocations;
            whole.render_float(a.data(), 16000, 2, true);
            for (std::size_t at = 0; at < 16000;) {
                const auto count = std::min<std::size_t>(71, 16000 - at);
                chunked.render_float(b.data() + at * 2, count, 2, true);
                at += count;
            }
            require(allocations == before && a == b, "motion allocated or depends on blocks");
            require(std::all_of(a.begin(), a.end(),
                                [](float v) { return std::isfinite(v) && std::abs(v) <= 0.5F; }),
                    "motion exceeded output bound");
            bool stereo = false;
            for (std::size_t i = 0; i < a.size(); i += 2)
                stereo = stereo || a[i] != a[i + 1];
            require(stereo, "pan spread did not create stereo variation");
            whole.reset();
            whole.render_float(b.data(), 16000, 2, true);
            require(a == b, "motion seed/reset failed");
            const grainmm::Sample one(source_rate, 1, {0.5F});
            grainmm::Granular tiny(one, 8000);
            tiny.set_parameters(motion);
            tiny.render_float(b.data(), 16000, 2, true);
        }
        // Trim bounds apply to reads/interpolation in both directions, including
        // extreme increments. Outside-region sentinels must never leak in.
        for (int rate : {8000, 96000}) {
            std::vector<float> values(64, 1);
            std::fill(values.begin() + 16, values.begin() + 48, 0);
            const grainmm::Sample region(rate, 2, std::move(values));
            for (float reverse : {0.0F, 1.0F}) {
                grainmm::Granular g(region, 8000);
                grainmm::GrainParameters p;
                p.start = .25F;
                p.end = .75F;
                p.reverse = reverse;
                p.pitch = 2;
                p.pitch_jitter = 12;
                p.density = 80;
                p.size_ms = 250;
                p.curve = 1;
                p.attack = .01F;
                p.release = .49F;
                g.set_parameters(p);
                std::vector<float> x(16000);
                const auto before_trim = allocations;
                g.render_float(x.data(), 8000, 2, true);
                require(before_trim == allocations, "trim path allocated");
                require(std::all_of(x.begin(), x.end(), [](float v) { return v == 0; }),
                        "trim leaked outside region");
            }
        }
        const grainmm::Sample ones(48000, 1, {.5F});
        grainmm::Granular latch_a(ones, 48000), latch_b(ones, 48000);
        grainmm::GrainParameters latching;
        latching.size_ms = 250;
        latching.density = 1;
        latch_a.set_parameters(latching);
        latch_b.set_parameters(latching);
        std::vector<float> latch_x(24000), latch_y(24000);
        latch_a.render_float(latch_x.data(), 480, 2, true);
        latch_b.render_float(latch_y.data(), 480, 2, true);
        latching.curve = 1;
        latching.attack = .01F;
        latching.release = .49F;
        latch_b.set_parameters(latching);
        latch_a.render_float(latch_x.data(), 11520, 2, true);
        latch_b.render_float(latch_y.data(), 11520, 2, true);
        require(latch_x == latch_y, "new envelope changed an already sounding grain");
        grainmm::Granular shaped(ones, 48000);
        shaped.set_parameters(latching);
        shaped.render_float(latch_y.data(), 12000, 2, true);
        require(latch_y.front() == 0 && latch_y.back() == 0 && latch_y[2000] > 0,
                "custom envelope endpoints");
        require(latch_x != latch_y, "envelope controls have no effect");
        const grainmm::Sample singleton(8000, 1, {1});
        grainmm::Granular engine(singleton, 8000);
        rejects([&] { grainmm::Granular bad(singleton, 0); }, "bad output rate accepted");
        grainmm::GrainParameters parameters{5, 1, 2, 1, 1, 0.5F};
        engine.set_parameters(parameters);
        std::vector<std::int16_t> pcm(8000);
        engine.render(pcm.data(), pcm.size(), 1, true);
        require(pcm[0] == 0 && pcm[39] == 0 && pcm[20] > 0 && pcm[40] == 0,
                "Hann endpoint/one-frame wrapping regression");
        engine.reset();
        engine.set_parameters({250, 80, 2, 1, 1, 0.5F});
        engine.render(pcm.data(), pcm.size(), 1, true);
        require(engine.dropped() > 0 && engine.dropped() + engine.spawned() == 80,
                "voice saturation must skip on schedule");
        const auto harmony = grainmm::prepare_harmony({12, 3, 3, 3, .8F, 50});
        const auto before_harmony = allocations;
        engine.set_harmony(harmony);
        engine.render(pcm.data(), pcm.size(), 1, true);
        engine.reset();
        engine.render(pcm.data(), pcm.size(), 1, true);
        require(before_harmony == allocations, "harmony apply/render/reset allocated");
        const grainmm::Sample silence(48000, 1, {0});
        grainmm::Granular silent_engine(silence, 48000);
        silent_engine.render(pcm.data(), pcm.size(), 1, true);
        require(std::all_of(pcm.begin(), pcm.end(), [](auto v) { return v == 0; }), "silent input");
        std::puts("granular: bounds, scheduling, seeded chunking, stereo, ramps, cap and no "
                  "allocations passed");
    } catch (const std::exception &error) {
        std::fprintf(stderr, "granular test: %s\n", error.what());
        return 1;
    }
}
