// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
namespace grainmm {
struct RhythmParameters {
    float bpm = 120, division = 1, tremolo = 0, pan = 0, shape = 0, stereo_degrees = 0;
};
RhythmParameters bounded(RhythmParameters p) noexcept;
struct PreparedRhythm {
    RhythmParameters values;
    float step = 0;
};
PreparedRhythm prepare_rhythm(RhythmParameters p, int rate);
// Tempo-based attenuating tremolo / linear autopan. Division0..3: half,
// quarter, eighth, sixteenth. Shape0/1: sine/triangle; fixed prebuilt LFO table.
class Rhythm {
  public:
    explicit Rhythm(int rate);
    void apply(const PreparedRhythm &p) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept { phase_ = 0; }

  private:
    float wave(double phase) const noexcept;
    std::array<float, 257> sine_{};
    PreparedRhythm target_;
    double phase_ = 0;
    float smoothing_, step_ = 0, tremolo_ = 0, pan_ = 0, shape_ = 0, offset_ = 0;
};
} // namespace grainmm
