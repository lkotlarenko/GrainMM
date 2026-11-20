// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
#include <vector>
namespace grainmm {
struct ReverbParameters {
    float mix = 0, size = 0.7F, decay_s = 6, damp_hz = 7000, predelay_ms = 20, width = 1;
};
ReverbParameters bounded(ReverbParameters p) noexcept;
struct PreparedReverb {
    ReverbParameters parameters;
    std::array<float, 8> feedback{};
    float damping = 1;
};
// Control thread only; all transcendental calculations happen here.
PreparedReverb prepare_reverb(ReverbParameters p, int rate);
// Original parallel-comb/allpass stereo reverb, not bit-identical upstream DSP.
// Construction allocates bounded rings. Audio-owner apply/process/reset never allocate.
class Reverb {
  public:
    explicit Reverb(int rate);
    void apply(const PreparedReverb &parameters) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept;
    std::size_t storage_bytes() const noexcept;

  private:
    struct Line {
        std::vector<float> data;
        std::size_t at = 0, valid = 0;
        float read(float age) const noexcept;
        void push(float value) noexcept;
        void clear() noexcept { at = valid = 0; }
    };
    int rate_;
    float smoothing_;
    PreparedReverb current_, target_;
    std::array<Line, 8> combs_;
    std::array<Line, 4> diffusers_;
    std::array<Line, 2> predelay_;
    std::array<float, 8> filtered_{};
    bool dormant_ = true;
};
} // namespace grainmm
