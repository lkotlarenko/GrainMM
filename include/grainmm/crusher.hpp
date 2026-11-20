// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
namespace grainmm {
struct CrusherParameters {
    float bits = 12, hold = 1, drive_db = 0, tone_hz = 12000, mix = 0, output_db = 0;
};
CrusherParameters bounded(CrusherParameters p) noexcept;
struct PreparedCrusher {
    CrusherParameters values;
    float levels = 2048, drive = 1, tone = 1, output = 1;
};
PreparedCrusher prepare_crusher(CrusherParameters p, int rate);
// Independent quantizer + shared-clock sample hold, post tone filter. Exact dry bypass.
class Crusher {
  public:
    explicit Crusher(int rate);
    void apply(const PreparedCrusher &p) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept;

  private:
    PreparedCrusher target_;
    std::array<float, 2> held_{}, low_{};
    unsigned countdown_ = 0;
    float mix_ = 0, drive_ = 1, levels_ = 2048, tone_ = 1, output_ = 1, smoothing_;
};
} // namespace grainmm
