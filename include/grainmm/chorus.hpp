// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
namespace grainmm {
struct ChorusParameters {
    float rate_hz = .8F, depth_ms = 3, delay_ms = 12, feedback = 0, mix = 0, width = 1;
};
ChorusParameters bounded(ChorusParameters p) noexcept;
struct PreparedChorus {
    ChorusParameters values;
    float step = 0, depth = 0, delay = 1;
};
PreparedChorus prepare_chorus(ChorusParameters p, int rate);
// Fixed32KiB stereo delay; interpolated modulated reads, no feedback gain above .7.
class Chorus {
  public:
    explicit Chorus(int rate);
    void apply(const PreparedChorus &p) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept;

  private:
    float sine(double phase) const noexcept;
    PreparedChorus target_;
    std::array<std::array<float, 4096>, 2> history_{};
    std::array<float, 257> sine_{};
    std::array<float, 2> feedback_{};
    unsigned write_ = 0;
    double phase_ = 0;
    float step_ = 0, depth_ = 0, delay_ = 1, feedback_gain_ = 0, mix_ = 0, width_ = 1, smoothing_;
};
} // namespace grainmm
