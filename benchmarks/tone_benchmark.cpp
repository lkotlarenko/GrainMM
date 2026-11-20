// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/tone.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <vector>

int main() {
    constexpr int rate = 48000;
    constexpr std::size_t frames = 512;
    constexpr std::size_t blocks = 5625; // 60 seconds of stereo PCM, processed offline.
    std::array<std::int16_t, frames * 2> pcm{};
    std::vector<double> elapsed(blocks);
    grainmm::Tone tone(rate);
    double checksum = 0;
    for (std::size_t i = 0; i < blocks; ++i) {
        const auto start = std::chrono::steady_clock::now();
        tone.render(pcm.data(), frames, 2, true);
        elapsed[i] =
            std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start)
                .count();
        checksum += pcm[frames];
    }
    std::sort(elapsed.begin(), elapsed.end());
    std::printf("HOST ONLY: 60s offline tone, 48000Hz stereo, frames=512 blocks=%zu\n"
                "p95_us=%.3f p99_us=%.3f max_us=%.3f deadline_us=10666.667 checksum=%.0f\n",
                blocks, elapsed[blocks * 95 / 100], elapsed[blocks * 99 / 100], elapsed.back(),
                checksum);
}
