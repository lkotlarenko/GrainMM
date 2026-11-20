// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/tone.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace {
void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}
} // namespace
int main() {
    try {
        for (const int rate : {8000, 44100, 48000, 96000}) {
            for (const int channels : {1, 2}) {
                grainmm::Tone tone(rate), split(rate);
                const auto count = static_cast<std::size_t>(rate);
                std::vector<std::int16_t> pcm(count * static_cast<std::size_t>(channels), 99);
                tone.render(pcm.data(), count, channels, false);
                require(std::all_of(pcm.begin(), pcm.end(), [](auto v) { return v == 0; }),
                        "silent start");
                grainmm::Tone reference(rate);
                tone = reference;
                tone.render(pcm.data(), count, channels, true);
                double square = 0.0, dc = 0.0;
                int crossings = 0;
                const auto skip = static_cast<std::size_t>(rate / 10);
                for (std::size_t frame = skip; frame < count; ++frame) {
                    const auto index = frame * static_cast<std::size_t>(channels);
                    const int value = pcm[index];
                    require(std::abs(value) <= 1311, "safe peak");
                    require(std::abs(value - pcm[index - static_cast<std::size_t>(channels)]) < 250,
                            "waveform discontinuity");
                    if (channels == 2)
                        require(pcm[index] == pcm[index + 1], "stereo mismatch");
                    square += static_cast<double>(value) * value;
                    dc += value;
                    if (value >= 0 && pcm[index - static_cast<std::size_t>(channels)] < 0)
                        ++crossings;
                }
                const double rms = std::sqrt(square / static_cast<double>(count - skip));
                require(rms > 920 && rms < 930, "RMS regression");
                require(std::abs(dc / static_cast<double>(count - skip)) < 2.0, "DC offset");
                require(crossings >= 197 && crossings <= 199, "220 Hz frequency regression");
                std::vector<std::int16_t> chunked(pcm.size());
                for (std::size_t at = 0; at < count;) {
                    const auto frames = std::min<std::size_t>(137, count - at);
                    split.render(chunked.data() + at * static_cast<std::size_t>(channels), frames,
                                 channels, true);
                    at += frames;
                }
                require(pcm == chunked, "block-size dependent phase or ramp");
                tone.render(pcm.data(), count, channels, false);
                require(tone.gain() == 0.0 && pcm.back() == 0, "mute ramp incomplete");
                tone.render(nullptr, 0, channels, false);
                std::int16_t sentinel = 123;
                tone.render(&sentinel, 1, 3, true);
                require(sentinel == 123, "invalid channel guard");
            }
        }
        for (const int bad : {0, -1, 7999, 192001}) {
            bool rejected = false;
            try {
                grainmm::Tone tone(bad);
            } catch (const std::invalid_argument &) {
                rejected = true;
            }
            require(rejected, "invalid rate accepted");
        }
        std::puts(
            "tone: PCM bounds, RMS/DC/frequency, ramps, channels, chunking and guards passed");
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "tone test: %s\n", error.what());
        return 1;
    }
}
