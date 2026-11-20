// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>

namespace grainmm {
enum class EqKind { Bell, LowShelf, HighShelf, LowPass, HighPass };
struct EqBand {
    float frequency, q, gain_db = 0;
    EqKind kind = EqKind::Bell;
};
using EqBands = std::array<EqBand, 6>;
EqBands default_eq_bands() noexcept;
struct BiquadCoefficients {
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
};
using EqCoefficients = std::array<BiquadCoefficients, 6>;
// Control-thread design from standard RBJ transfer functions, not copied DSP.
// Clamps finite frequency/Q/gain to 25..min(18000,0.45*rate),0.35..5,-20..20.
// Shelves use Q damping, not the separate cookbook shelf-slope S parameter.
EqCoefficients design_equalizer(EqBands bands, int rate);
// Audio-owner processor. Fixed storage; no allocation, I/O or transcendental
// functions in set_coefficients/process/reset. Source/target coefficients are
// independently validated; invalid updates leave the previous target untouched.
// Crossfades two stable banks over 20ms instead of varying IIR denominators.
// During a transition, only the latest pending target is retained.
class Equalizer {
  public:
    explicit Equalizer(int rate);
    bool set_coefficients(const EqCoefficients &coefficients) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept; // Clears history, preserving current controls/transition.

  private:
    struct History {
        double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    };
    struct Bank {
        EqCoefficients coefficients{};
        std::array<bool, 6> bypass{};
        bool all_bypassed = true;
        std::array<std::array<History, 2>, 6> history{};
    };
    void begin_transition(const EqCoefficients &coefficients) noexcept;
    double tick(Bank &bank, double input, unsigned channel) noexcept;
    std::array<Bank, 2> banks_{};
    EqCoefficients pending_{}, requested_{};
    unsigned active_ = 0, fade_frames_, remaining_ = 0;
    bool pending_valid_ = false;
};
} // namespace grainmm
