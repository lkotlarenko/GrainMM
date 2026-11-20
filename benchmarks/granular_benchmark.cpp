// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/granular.hpp"
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
    unsigned seconds = 60;
    if (argc != 1) {
        char *end = nullptr;
        if (argc != 3 || std::strcmp(argv[1], "--seconds") != 0) {
            std::fprintf(stderr, "Usage: granular_benchmark [--seconds 1..60]\n");
            return 1;
        }
        const auto value = std::strtoul(argv[2], &end, 10);
        if (!*argv[2] || *end || value < 1 || value > 60)
            return 1;
        seconds = static_cast<unsigned>(value);
    }
    const unsigned blocks = seconds * rate / frames;
#ifdef __arm__
    constexpr const char *platform = "ARM";
#else
    constexpr const char *platform = "HOST";
#endif
    std::array<std::int16_t, frames * 2> pcm{};
    for (const int channels : {1, 2}) {
        std::vector<float> values(131072 * static_cast<unsigned>(channels));
        for (std::size_t i = 0; i < values.size(); ++i)
            values[i] = static_cast<float>(std::sin(static_cast<double>(i) * 0.013)) * 0.8F;
        const grainmm::Sample sample(rate, channels, std::move(values));
        for (const float density : {12.0F, 80.0F}) {
            for (const float size : {80.0F, 250.0F}) {
                grainmm::Granular engine(sample, rate);
                engine.set_parameters({size, density, 1.5F, 0.5F, 0.25F, 0.5F});
                std::vector<double> timings(blocks);
                long checksum = 0;
                for (unsigned i = 0; i < blocks; ++i) {
                    const auto start = std::chrono::steady_clock::now();
                    engine.render(pcm.data(), frames, channels, true);
                    timings[i] = std::chrono::duration<double, std::micro>(
                                     std::chrono::steady_clock::now() - start)
                                     .count();
                    checksum += pcm[11];
                }
                std::sort(timings.begin(), timings.end());
                std::printf(
                    "%s OFFLINE channels=%d density=%.0f size_ms=%.0f "
                    "p95_us=%.3f p99_us=%.3f max_us=%.3f spawned=%u dropped=%u checksum=%ld\n",
                    platform, channels, static_cast<double>(density), static_cast<double>(size),
                    timings[blocks * 95 / 100], timings[blocks * 99 / 100], timings.back(),
                    engine.spawned(), engine.dropped(), checksum);
            }
        }
    }
}
