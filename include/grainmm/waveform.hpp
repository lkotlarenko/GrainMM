// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/sample.hpp"
#include <array>
namespace grainmm {
struct WavePeak {
    float low = 0, high = 0;
};
// UI-only cache built once per source replacement. Never scans/allocates in audio.
class Waveform {
  public:
    static constexpr std::size_t columns = 256;
    explicit Waveform(const Sample &sample) noexcept;
    const std::array<WavePeak, columns> &peaks() const noexcept { return peaks_; }
    double seconds() const noexcept { return seconds_; }

  private:
    std::array<WavePeak, columns> peaks_{};
    double seconds_ = 0;
};
} // namespace grainmm
