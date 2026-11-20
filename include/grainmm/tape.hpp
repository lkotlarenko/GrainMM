// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
namespace grainmm {
struct TapeParameters {
    float drive_db = 0, tone_hz = 12000, wow_ms = 0, flutter_ms = 0, mix = 0, output_db = 0;
};
TapeParameters bounded(TapeParameters p) noexcept;
struct PreparedTape {
    TapeParameters values;
    float drive = 1, tone = 1, output = 1;
};
PreparedTape prepare_tape(TapeParameters p, int rate);
// Original tape-style coloration, not a physical tape model. Wet path: saturation,
// one-pole tone filter and 3ms variable delay (0.6Hz wow / 6Hz flutter).
// Exact dry bypass; fixed storage, no allocation or coefficient design in process.
class Tape {
  public:
    explicit Tape(int rate);
    void apply(const PreparedTape &p) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept;
    std::size_t storage_bytes() const noexcept { return sizeof(history_) + sizeof(sine_); }

  private:
    float modulation(float phase) const noexcept;
    PreparedTape target_;
    std::array<std::array<float, 1024>, 2> history_{};
    std::array<float, 257> sine_{};
    std::array<float, 2> lowpass_{};
    std::size_t write_ = 0;
    float rate_, smoothing_, drive_ = 1, tone_ = 1, output_ = 1;
    float mix_ = 0, wow_ = 0, flutter_ = 0, slow_phase_ = 0, fast_phase_ = 0;
    bool dormant_ = true;
};
} // namespace grainmm
