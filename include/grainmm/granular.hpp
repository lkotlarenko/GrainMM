// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/drone.hpp"
#include "grainmm/harmony.hpp"
#include "grainmm/sample.hpp"
#include <array>
#include <cstdint>

namespace grainmm {
struct GrainParameters {
    float size_ms = 80, density = 12, pitch = 1, position = 0.5F, spread = 0.1F, gain = 0.2F;
    float reverse = 0, pitch_jitter = 0, size_jitter = 0, timing_jitter = 0, pan_spread = 0,
          scan_hz = 0;
    // Non-destructive region and voice-latched envelope. Curve0 = original Hann;
    // curve1 = attack/hold/release window. Width0 = mono, width1 = original stereo.
    float start = 0, end = 1, attack = 0.2F, release = 0.2F, curve = 0, width = 1;
};
GrainParameters bounded(GrainParameters parameters) noexcept;

// Audio-thread-owned. Sample must outlive the engine. No allocation in set/render/reset.
class Granular {
  public:
    static constexpr unsigned voice_limit = 16;
    Granular(const Sample &sample, int output_rate, std::uint32_t seed = 1);
    void set_parameters(GrainParameters parameters) noexcept;
    void set_harmony(const PreparedHarmony &p) noexcept { harmony_.apply(p); }
    void render(std::int16_t *out, std::size_t frames, int channels, bool playing) noexcept;
    void render_float(float *out, std::size_t frames, int channels, bool playing) noexcept;
    void set_drone(DroneParameters p) noexcept { drone_.apply(p); }
    void reset() noexcept;
    unsigned visual_state() const noexcept;
    std::uint32_t spawned() const noexcept { return spawned_; }
    std::uint32_t dropped() const noexcept { return dropped_; }

  private:
    template <typename Output>
    void render_into(Output *out, std::size_t frames, int channels, bool playing) noexcept;
    struct Voice {
        double position = 0, increment = 1, window_step = 0;
        std::uint32_t age = 0, duration = 0;
        float left_gain = 1, right_gain = 1;
        std::size_t first = 0, end = 0;
        float attack_scale = 0, release_scale = 0, curve = 0;
    };
    float random_signed() noexcept;
    void spawn() noexcept;
    float interpolate(const Voice &voice, int channel) const noexcept;
    const Sample &sample_;
    int rate_;
    GrainParameters parameters_{};
    GrainHarmony harmony_;
    DroneMotion drone_;
    DroneFrame modulation_{};
    std::array<Voice, voice_limit> voices_{};
    std::array<float, 1025> envelope_{};
    double countdown_ = 0, scan_phase_ = 0;
    float gain_ = 0, width_ = 1;
    std::uint32_t seed_, random_, spawned_ = 0, dropped_ = 0;
};
} // namespace grainmm
