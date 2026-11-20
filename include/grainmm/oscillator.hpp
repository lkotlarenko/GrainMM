// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
namespace grainmm {
struct OscillatorParameters {
    float level = 0, note = 57, wave = 0, detune_cents = 0, pulse_width = 0.5F, vibrato_cents = 0;
};
OscillatorParameters bounded(OscillatorParameters p) noexcept;
// Color applies to oscillator A; no table assets or runtime file loading.
struct SynthColorParameters {
    float table_mix = 0, position = 0, scan_hz = 0, scan_depth = 0;
    float sub_level = 0, sub_octave = 1;
};
SynthColorParameters bounded(SynthColorParameters p) noexcept;
struct PreparedOscillator {
    OscillatorParameters values;
    double left_step = 0, right_step = 0;
    float vibrato_amount = 0;
    SynthColorParameters color;
};
PreparedOscillator prepare_oscillator(OscillatorParameters p, int rate,
                                      SynthColorParameters color = {});
// Fixed-storage stereo oscillator. Parameter design is control-thread work.
// Sine lookup, finite additive triangle, polynomial edge-corrected saw/pulse.
class Oscillator {
  public:
    explicit Oscillator(int rate);
    void apply(const PreparedOscillator &p) noexcept;
    void add(float *samples, std::size_t frames, int channels, bool playing) noexcept;
    void reset() noexcept;
    bool enabled() const noexcept {
        return target_.values.level > 0 || target_.color.sub_level > 0;
    }

  private:
    float sine(double phase) const noexcept;
    float waveform(unsigned kind, double phase, double step, float width) const noexcept;
    float table(double phase, double step, float position) const noexcept;
    // Shared generated spectra, initialized during construction, before audio.
    using Tables = std::array<std::array<std::array<float, 257>, 8>, 4>;
    static const Tables &tables();
    const Tables &tables_;
    PreparedOscillator current_, target_;
    std::array<float, 1025> sine_{};
    std::array<float, 4> weights_{1, 0, 0, 0};
    double left_phase_ = 0, right_phase_ = 0, vibrato_phase_ = 0, vibrato_step_;
    float gain_ = 0, sub_gain_ = 0, smoothing_;
    double sub_phase_ = 0, scan_phase_ = 0, sub_step_ = 0;
};
} // namespace grainmm
