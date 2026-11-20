// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstddef>
namespace grainmm {
struct CompressorParameters {
    float threshold_db = -18, ratio = 4, attack_ms = 10, release_ms = 150, makeup_db = 0, mix = 0;
};
CompressorParameters bounded(CompressorParameters p) noexcept;
struct PreparedCompressor {
    CompressorParameters values;
    float threshold = 0.12589254F, exponent = 0.75F, attack = 0, release = 0, makeup = 1;
};
PreparedCompressor prepare_compressor(CompressorParameters p, int rate);
// Stereo-linked feed-forward peak compressor. Fixed state, no allocation/I/O.
// Gain curve updates every 8 frames and is smoothed at the full audio rate.
class Compressor {
  public:
    explicit Compressor(int rate);
    void apply(const PreparedCompressor &p) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept;

  private:
    PreparedCompressor target_;
    float envelope_ = 0, gain_ = 1, target_gain_ = 1, mix_ = 0, makeup_ = 1, smoothing_;
    unsigned curve_phase_ = 0;
    bool dormant_ = true;
};
} // namespace grainmm
