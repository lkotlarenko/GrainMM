// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
namespace grainmm {
struct ResonatorParameters {
    float note = 57, decay_s = 0.5F, brightness = 0.5F, inharmonic = 0, width = 1, mix = 0;
};
ResonatorParameters bounded(ResonatorParameters p) noexcept;
struct ResonantMode {
    double a = 0, radius2 = 0, gain = 0;
};
struct PreparedResonator {
    ResonatorParameters values;
    std::array<ResonantMode, 4> modes{};
};
PreparedResonator prepare_resonator(ResonatorParameters p, int rate);
// Four parallel constant-peak-gain modes. Prepared coefficients, bounded banks,
// no allocation. Retuning crossfades banks rather than moving sounding poles.
class Resonator {
  public:
    explicit Resonator(int rate);
    void apply(const PreparedResonator &p) noexcept;
    void process(float *samples, std::size_t frames, int channels) noexcept;
    void reset() noexcept;

  private:
    struct History {
        double y1 = 0, y2 = 0;
    };
    struct Bank {
        std::array<ResonantMode, 4> modes{};
        std::array<std::array<History, 2>, 4> history{};
        std::array<double, 2> x1{}, x2{};
    };
    void transition() noexcept;
    static double tick(Bank &bank, double x, unsigned channel) noexcept;
    std::array<Bank, 2> banks_{};
    PreparedResonator target_;
    unsigned active_ = 0, remaining_ = 0, fade_frames_;
    float smoothing_, mix_ = 0, width_ = 1;
    bool pending_ = false, dormant_ = true;
};
} // namespace grainmm
