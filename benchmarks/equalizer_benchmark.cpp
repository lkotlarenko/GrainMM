// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/equalizer.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char **argv) {
    try {
        unsigned seconds = 3;
        if (argc == 3 && std::string(argv[1]) == "--seconds") {
            char *end = nullptr;
            const auto value = std::strtoul(argv[2], &end, 10);
            if (!*argv[2] || *end || value < 1 || value > 5)
                throw std::invalid_argument("seconds must be 1..5");
            seconds = static_cast<unsigned>(value);
        } else if (argc != 1)
            throw std::invalid_argument("Usage: equalizer_benchmark [--seconds 1..5]");
        constexpr unsigned rate = 48000, frames = 512;
        const unsigned blocks = seconds * rate / frames;
        std::array<float, frames * 2> input{}, pcm{};
        for (std::size_t i = 0; i < frames; ++i) {
            input[i * 2] = static_cast<float>(
                0.1 * std::sin(6.28318530718 * 440 * static_cast<double>(i) / rate));
            input[i * 2 + 1] = static_cast<float>(
                0.1 * std::sin(6.28318530718 * 660 * static_cast<double>(i) / rate));
        }
#ifdef __arm__
        const char *platform = "ARM native only if executed on hardware";
#else
        const char *platform = "HOST";
#endif
        std::printf("EQ %s rate=%u frames=%u sizeof_eq=%zu simulated_seconds=%u\n", platform, rate,
                    frames, sizeof(grainmm::Equalizer), seconds);
        for (const char *name : {"flat", "six_bells", "changing"}) {
            auto bands = grainmm::default_eq_bands();
            if (std::string(name) != "flat")
                for (std::size_t i = 0; i < bands.size(); ++i)
                    bands[i].gain_db = i % 2 ? -12.F : 12.F;
            const bool changing = std::string(name) == "changing";
            const auto first = grainmm::design_equalizer(bands, rate);
            for (auto &band : bands) {
                band.gain_db = -band.gain_db;
                band.q = 2.F;
            }
            const auto second = grainmm::design_equalizer(bands, rate);
            grainmm::Equalizer eq(rate);
            if (!eq.set_coefficients(first))
                throw std::runtime_error("initial EQ rejected");
            for (unsigned i = 0; i < 4; ++i) {
                pcm.fill(0);
                eq.process(pcm.data(), frames, 2);
            }
            std::vector<double> timings;
            timings.reserve(blocks);
            double checksum = 0;
            for (unsigned block = 0; block < blocks; ++block) {
                pcm = input;
                const auto begin = std::chrono::steady_clock::now();
                if (changing && block % 8 == 0)
                    if (!eq.set_coefficients((block / 8) % 2 ? first : second))
                        throw std::runtime_error("EQ transition rejected");
                eq.process(pcm.data(), frames, 2);
                const auto end = std::chrono::steady_clock::now();
                timings.push_back(std::chrono::duration<double, std::micro>(end - begin).count());
                for (const float value : pcm)
                    checksum += value;
            }
            std::sort(timings.begin(), timings.end());
            std::printf("case=%s blocks=%u p95_us=%.3f p99_us=%.3f max_us=%.3f checksum=%.9f\n",
                        name, blocks, timings[timings.size() * 95 / 100],
                        timings[timings.size() * 99 / 100], timings.back(), checksum);
        }
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
