// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/frame_clock.hpp"
#include "grainmm/sample.hpp"
#include "grainmm/slice_pattern.hpp"
#include <array>
#include <cstdint>
namespace grainmm {
struct SliceParameters {
    float mode = 0, regions = 8, division = 2, gate = .8F, variation = 0, reverse = 0;
};
SliceParameters bounded(SliceParameters p) noexcept;
struct GrooveParameters {
    float swing = 0, length = 16, rotate = 0, order = 0, accent = 0, accent_every = 4;
};
GrooveParameters bounded(GrooveParameters p) noexcept;
struct PreparedSlice {
    SliceParameters values;
    SlicePattern pattern;
    GrooveParameters groove;
    float frames_per_step = 6000;
    std::array<float, 17> cuts{};
};
PreparedSlice prepare_slice(SliceParameters p, float bpm, int rate,
                            const SlicePattern &pattern = {}, const GrooveParameters &groove = {});
// Immutable sample shared with Granular. Two crossfading voices; no allocations.
class SlicePlayer {
  public:
    SlicePlayer(const Sample &sample, int rate);
    void apply(const PreparedSlice &p, float pitch, float gain, float start, float end) noexcept;
    void render(float *out, std::size_t frames, int channels, bool playing,
                FrameClock clock) noexcept;
    void reset() noexcept;
    // Packed UI telemetry: step[0..3], region[4..7], valid triggered step[16].
    std::uint32_t view() const noexcept { return view_; }
    unsigned triggered() const noexcept { return triggered_; }

  private:
    struct Voice {
        double position = 0, increment = 1;
        float velocity = 1;
        std::size_t first = 0, end = 0;
        unsigned age = 0, duration = 0, fade = 0;
    };
    void trigger(float duration_scale) noexcept;
    float random() noexcept;
    const Sample &sample_;
    int rate_;
    PreparedSlice target_;
    std::array<Voice, 2> voices_{};
    float pitch_ = 1, target_gain_ = .2F, gain_ = 0, start_ = 0, end_ = 1;
    unsigned step_ = 0, next_voice_ = 0, triggered_ = 0;
    std::uint32_t random_ = 1, view_ = 0;
    bool starting_ = true;
    std::uint64_t last_tick_ = 0;
};
} // namespace grainmm
