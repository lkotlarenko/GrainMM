// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
namespace grainmm {
struct FilterParameters {
    float cutoff_hz = 8000, resonance = .707F, mode = 0, drive_db = 0, mix = 0, output_db = 0;
};
FilterParameters bounded(FilterParameters p) noexcept;
struct PreparedFilter {
    FilterParameters values;
    float g = 1, k = 1.414F, drive = 1, output = 1;
};
PreparedFilter prepare_filter(FilterParameters p, int rate);
// Two-integrator topology-preserving SVF. Smooth low/high/bandpass mode changes.
class Filter {
  public:
    explicit Filter(int rate);
    void apply(const PreparedFilter &p) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept;

  private:
    PreparedFilter target_;
    std::array<double, 2> one_{}, two_{};
    std::array<float, 3> weights_{1, 0, 0};
    float g_, k_ = 1.414F, drive_ = 1, output_ = 1, mix_ = 0, smoothing_;
};
} // namespace grainmm
