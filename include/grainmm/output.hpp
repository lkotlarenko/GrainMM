// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstddef>
namespace grainmm {
struct OutputParameters {
    float gain_db = 0, limiter = 0, ceiling_db = -1, release_ms = 200, balance = 0, mono = 0;
};
OutputParameters bounded(OutputParameters p) noexcept;
struct PreparedOutput {
    OutputParameters values;
    float gain = 1, ceiling = .89125094F, release = 0;
};
PreparedOutput prepare_output(OutputParameters p, int rate);
// Post-rack trim and stereo-linked sample-peak limiter. No lookahead or true-peak
// guarantee. Fixed state, no allocation/design/I/O in process. Neutral is bit-exact.
class Output {
  public:
    explicit Output(int rate);
    void apply(const PreparedOutput &p) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept {
        limiting_ = 1;
        reduction_ = 0;
    }
    unsigned reduction() const noexcept { return reduction_; }

  private:
    PreparedOutput target_;
    float gain_ = 1, ceiling_ = .89125094F, balance_ = 0, mono_ = 0, limiting_ = 1, smoothing_;
    unsigned reduction_ = 0;
};
} // namespace grainmm
