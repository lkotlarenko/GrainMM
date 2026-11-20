// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
#include <vector>

namespace grainmm {
struct DelayParameters {
    float time_ms = 450, feedback = 0.35F, mix = 0, ping_pong = 0, spread = 1, lowpass_hz = 9000;
};
DelayParameters bounded(DelayParameters parameters) noexcept;
// Original stereo delay. Allocate once at construction; process/set/reset do not
// allocate. Caller owns buffers; samples are interleaved normalized floats.
class Delay {
  public:
    explicit Delay(int rate, DelayParameters initial = {});
    void set_parameters(DelayParameters parameters) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept; // O(1) logical clear; stale ring contents cannot leak.
    std::size_t storage_bytes() const noexcept { return ring_.size() * sizeof(ring_[0]); }

  private:
    float read(std::size_t age, unsigned channel) const noexcept;
    float coefficient(float hz) const noexcept;
    int rate_;
    std::vector<std::array<float, 2>> ring_;
    std::size_t write_ = 0, valid_ = 0;
    DelayParameters target_, current_;
    float smoothing_, lowpass_, target_lowpass_;
    std::array<float, 2> filtered_{};
};
} // namespace grainmm
