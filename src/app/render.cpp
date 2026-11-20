// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/granular.hpp"
#include "grainmm/wave.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
    try {
        std::string directory = ".", sample_name, output;
        unsigned seconds = 5;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--help") {
                std::puts("grainmm-render --sample-dir DIR --sample BASENAME --output NEW.wav "
                          "[--seconds 1..60]\n"
                          "Offline original granular renderer; output is exclusively created, "
                          "never overwritten.");
                return 0;
            }
            if (i + 1 >= argc)
                throw std::invalid_argument("missing argument value");
            const std::string value = argv[++i];
            if (arg == "--sample-dir")
                directory = value;
            else if (arg == "--sample")
                sample_name = value;
            else if (arg == "--output")
                output = value;
            else if (arg == "--seconds") {
                char *end = nullptr;
                const auto number = std::strtoul(value.c_str(), &end, 10);
                if (value.empty() || *end || number < 1 || number > 60)
                    throw std::invalid_argument("seconds must be 1..60");
                seconds = static_cast<unsigned>(number);
            } else
                throw std::invalid_argument("unknown argument: " + arg);
        }
        if (sample_name.empty() || output.empty())
            throw std::invalid_argument("--sample and --output are required");
        const auto sample = grainmm::load_wave(directory, sample_name);
        constexpr unsigned rate = 48000;
        const unsigned total = seconds * rate;
        grainmm::Granular engine(sample, rate);
        grainmm::WaveWriter writer(output, rate, 2, total);
        std::array<std::int16_t, 1024> pcm{};
        double square = 0, sum = 0;
        int peak = 0;
        for (unsigned at = 0; at < total;) {
            // End the audible section exactly at a block boundary, leave a 20 ms tail.
            const auto fade = total - 960;
            const auto count = std::min<unsigned>(512, at < fade ? fade - at : total - at);
            engine.render(pcm.data(), count, 2, at < fade);
            writer.write(pcm.data(), count);
            for (unsigned i = 0; i < count * 2; ++i) {
                const int value = pcm[i];
                peak = std::max(peak, std::abs(value));
                sum += value;
                square += static_cast<double>(value) * value;
            }
            at += count;
        }
        writer.finish();
        std::printf("offline rate=%u frames=%u peak=%d rms=%.3f dc=%.3f grains=%u dropped=%u\n",
                    rate, total, peak, std::sqrt(square / (total * 2.0)), sum / (total * 2.0),
                    engine.spawned(), engine.dropped());
    } catch (const std::exception &error) {
        std::fprintf(stderr, "grainmm-render: %s\n", error.what());
        return 1;
    }
}
