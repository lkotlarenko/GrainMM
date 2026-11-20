// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstdint>
namespace grainmm {
struct DroneParameters {
    // Off / periodic wave / seeded drift / blend. Hold freezes movement, not playback.
    float mode = 0, rate_hz = .05F, position_depth = .2F, pitch_cents = 0, spread_depth = 0,
          hold = 0;
};
DroneParameters bounded(DroneParameters p) noexcept;
struct DroneFrame {
    float position = 0, pitch_cents = 0, spread = 0;
};
// Audio-owned slow modulation. Targets update every64audio frames, then slew over
// 20ms. No allocation; base musical parameters are never changed by modulation.
class DroneMotion {
  public:
    explicit DroneMotion(int rate, std::uint32_t seed = 0x729a4d35U);
    void apply(DroneParameters p) noexcept;
    void reset() noexcept;
    DroneFrame next(bool playing) noexcept;
    bool holding() const noexcept { return parameters_.mode != 0 && parameters_.hold != 0; }

  private:
    float random_signed() noexcept;
    int rate_;
    std::uint32_t seed_, random_;
    DroneParameters parameters_{};
    DroneFrame value_{}, target_{};
    double phase_ = 0, increment_ = 0;
    float from_ = 0, to_ = 0, smoothing_;
    unsigned countdown_ = 0;
};
} // namespace grainmm
