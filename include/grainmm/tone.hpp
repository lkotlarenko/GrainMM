// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstddef>
#include <cstdint>

namespace grainmm {
// Audio-thread-owned oscillator. UI sends only a bounded on/off command.
// 220 Hz, peak <= 0.04 full scale; 20 ms linear ramp for click suppression.
class Tone {
  public:
    explicit Tone(int sample_rate);
    void render(std::int16_t *out, std::size_t frames, int channels, bool audible) noexcept;
    double gain() const noexcept { return gain_; }

  private:
    double phase_ = 0.0;
    double gain_ = 0.0;
    double phase_step_;
    double gain_step_;
};
} // namespace grainmm
