// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/wave.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace {
struct Temporary {
    char path[40] = "/tmp/grainmm-wave-values-XXXXXX";
    Temporary() {
        if (!::mkdtemp(path))
            throw std::runtime_error("temporary directory failed");
    }
    ~Temporary() {
        ::unlink((std::string(path) + "/values.wav").c_str());
        ::unlink((std::string(path) + "/short.wav").c_str());
        ::rmdir(path);
    }
};
} // namespace
int main() {
    try {
        Temporary temporary;
        constexpr std::array<std::int16_t, 6> values{-32768, -1, 0, 1, 32766, 32767};
        grainmm::WaveWriter writer(std::string(temporary.path) + "/values.wav", 96000, 2, 3);
        writer.write(values.data(), 3);
        writer.finish();
        const auto sample = grainmm::load_wave(temporary.path, "values.wav");
        if (sample.frames() != 3 || sample.channels() != 2 || sample.rate() != 96000)
            return 1;
        for (unsigned frame = 0; frame < 3; ++frame)
            for (int channel = 0; channel < 2; ++channel)
                if (sample.value(frame, channel) !=
                    static_cast<float>(values[frame * 2 + static_cast<unsigned>(channel)]) /
                        32768.0F)
                    return 1;
        if (sample.value(3, 0) != 0 || sample.value(0, -1) != 0 || sample.value(0, 2) != 0)
            return 1;
        grainmm::WaveWriter short_writer(std::string(temporary.path) + "/short.wav", 8000, 1, 3);
        bool overflow_rejected = false, short_rejected = false;
        try {
            short_writer.write(values.data(), 4);
        } catch (const std::invalid_argument &) {
            overflow_rejected = true;
        }
        try {
            short_writer.finish();
        } catch (const std::runtime_error &) {
            short_rejected = true;
        }
        if (!overflow_rejected || !short_rejected)
            return 1;
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "wave values test: %s\n", error.what());
        return 1;
    }
}
