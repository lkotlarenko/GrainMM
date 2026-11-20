// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
int main(int argc, char **argv) {
    constexpr int rate = 48000;
    constexpr unsigned frames = 512;
    unsigned seconds = 5;
    const bool synthesis = argc == 4 && std::strcmp(argv[3], "--synthesis") == 0;
    const bool drone = argc == 4 && std::strcmp(argv[3], "--evolve") == 0;
    const bool output = argc == 4 && std::strcmp(argv[3], "--output") == 0;
    const bool ambient = argc == 4 && std::strcmp(argv[3], "--drone") == 0;
    const bool collision = argc == 4 && std::strcmp(argv[3], "--collision") == 0;
    const bool slicing = argc == 4 && std::strcmp(argv[3], "--slice") == 0;
    const bool texture = synthesis || output || ambient || drone || collision || slicing ||
                         (argc == 4 && std::strcmp(argv[3], "--texture") == 0);
    const bool harmony = texture || (argc == 4 && std::strcmp(argv[3], "--harmony") == 0);
    if (argc != 1) {
        char *end = nullptr;
        if ((argc != 3 && !harmony) || std::strcmp(argv[1], "--seconds") != 0)
            return 1;
        const auto value = std::strtoul(argv[2], &end, 10);
        if (!*argv[2] || *end || value < 1 || value > 10)
            return 1;
        seconds = static_cast<unsigned>(value);
    }
    const unsigned blocks = seconds * rate / frames;
    std::vector<float> source(131072 * 2);
    for (std::size_t i = 0; i < source.size(); ++i)
        source[i] = static_cast<float>(std::sin(static_cast<double>(i) * 0.013)) * 0.8F;
    const grainmm::Sample sample(rate, 2, std::move(source));
    std::array<std::int16_t, frames * 2> pcm{};
    for (const bool dense : {false, true}) {
        for (const bool full : {false, true}) {
            for (const float mix : {0.0F, 0.65F}) {
                if (full && mix == 0)
                    continue;
                for (const bool colored : {false, true}) {
                    if (colored && !full)
                        continue;
                    for (const bool expanded : {false, true}) {
                        if (expanded && !(full && colored))
                            continue;
                        grainmm::Instrument engine(sample, rate);
                        grainmm::InstrumentParameters p;
                        p.grain = {
                            dense ? 250.0F : 80.0F, dense ? 80.0F : 12.0F, 1.5F, 0.5F, 0.25F, 0.5F};
                        if (synthesis) {
                            p.oscillator = {.12F, 57, 2, 30, .1F, 100};
                            p.oscillator_b = {.1F, 64, 1, 30, .9F, 100};
                            p.synth_color = {1, 1.5F, 2, 1, .08F, 2};
                        }
                        if (output)
                            p.output = {6, 1, -1, 200, 0, 0};
                        if (ambient)
                            p.drone_synth = {.6F, 60, 4, 4, 1, 1, 1, 2, .05F, 1, 314, 0};
                        if (drone)
                            p.drone = {3, .5F, .5F, 400, .3F, 0};
                        if (collision)
                            p.collision = {1, 6, 2, 1, .98F, .1F, 60, 0, 0, 3, 1, 1};
                        if (slicing)
                            p.slice = {1, 16, 3, .9F, 2, .5F};
                        if (harmony)
                            p.harmony = {0, 3, 2, 2, 1, 12};
                        if (texture) {
                            p.crusher = {8, 4, 3, 9000, .3F, -3};
                            p.filter = {4000, .707F, 0, 3, .5F, -3};
                            p.chorus = {.8F, 3, 12, .2F, .3F, 1};
                        }
                        p.delay = {450.5F, 0.95F, mix, 1, 1, 800};
                        if (full) {
                            p.eq_db = {6, -6, 6, -6, 6, -6};
                            p.reverb.mix = 0.5F;
                            p.reverb.decay_s = 12;
                            p.grain.reverse = 0.5F;
                            p.grain.pitch_jitter = 7;
                            p.grain.size_jitter = 0.5F;
                            p.grain.timing_jitter = 0.5F;
                            p.grain.pan_spread = 1;
                            p.grain.scan_hz = 0.1F;
                            p.oscillator = {0.08F, 57, 2, 8, 0.5F, 10};
                            if (colored)
                                p.tape = {12, 5000, 1.5F, 0.2F, 0.7F, -6};
                            p.compressor = {-24, 4, 10, 150, 0, 0.7F};
                        }
                        if (expanded) {
                            p.resonator = {57, .5F, .7F, .4F, 1, .5F};
                            p.rhythm = {135, 2, .7F, .5F, 1, 90};
                            p.grain.start = .15F;
                            p.grain.end = .85F;
                            p.grain.curve = .7F;
                        }
                        engine.set_parameters(p);
                        if (collision || ambient)
                            for (unsigned warm = 0; warm < 2U * rate / frames; ++warm)
                                engine.render(pcm.data(), frames, 2, true);
                        std::vector<double> timings(blocks);
                        long checksum = 0;
                        for (unsigned i = 0; i < blocks; ++i) {
                            const auto start = std::chrono::steady_clock::now();
                            engine.render(pcm.data(), frames, 2, true);
                            timings[i] = std::chrono::duration<double, std::micro>(
                                             std::chrono::steady_clock::now() - start)
                                             .count();
                            checksum += pcm[11];
                        }
                        std::sort(timings.begin(), timings.end());
                        std::printf(
                            "OFFLINE collision=%u slice=%u texture=%u harmony=%u full=%u tape=%u "
                            "rack=%u "
                            "dense=%u "
                            "mix=%.2f "
                            "blocks=%u "
                            "p95_us=%.3f "
                            "p99_us=%.3f "
                            "max_us=%.3f "
                            "delay_bytes=%zu reverb_bytes=%zu clipped=%u grains=%u dropped=%u "
                            "slices=%u hits=%u checksum=%ld\n",
                            collision ? 1U : 0U, slicing ? 1U : 0U, texture ? 1U : 0U,
                            harmony ? 1U : 0U, full ? 1U : 0U, colored ? 1U : 0U,
                            expanded ? 1U : 0U, dense ? 1U : 0U, static_cast<double>(mix), blocks,
                            timings[blocks * 95 / 100], timings[blocks * 99 / 100], timings.back(),
                            engine.delay_storage_bytes(), engine.reverb_storage_bytes(),
                            engine.clipped_samples(), engine.spawned(), engine.dropped(),
                            engine.slices_triggered(), engine.visuals().hits, checksum);
                    }
                }
            }
        }
    }
    grainmm::Instrument synth(static_cast<const grainmm::Sample *>(nullptr), rate);
    grainmm::InstrumentParameters p;
    p.oscillator = {0.15F, 69, 3, 12, 0.25F, 20};
    if (synthesis) {
        p.oscillator_b = {.1F, 76, 1, 30, .5F, 100};
        p.synth_color = {1, 1.5F, 2, 1, .08F, 2};
    }
    p.delay.mix = 0.3F;
    p.reverb.mix = 0.3F;
    p.compressor.mix = 0.6F;
    p.tape = {12, 5000, 1.5F, 0.2F, 0.7F, -6};
    p.resonator = {57, .5F, .7F, .4F, 1, .5F};
    p.rhythm = {135, 2, .7F, .5F, 1, 90};
    synth.set_parameters(p);
    std::vector<double> times(blocks);
    long checksum = 0;
    for (unsigned i = 0; i < blocks; ++i) {
        const auto start = std::chrono::steady_clock::now();
        synth.render(pcm.data(), frames, 2, true);
        times[i] =
            std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start)
                .count();
        checksum += pcm[11];
    }
    std::sort(times.begin(), times.end());
    std::printf("OFFLINE standalone_synth=1 rack=1 blocks=%u p95_us=%.3f p99_us=%.3f max_us=%.3f "
                "clipped=%u checksum=%ld\n",
                blocks, times[blocks * 95 / 100], times[blocks * 99 / 100], times.back(),
                synth.clipped_samples(), checksum);
}
