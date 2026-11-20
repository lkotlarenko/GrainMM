// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/frame_clock.hpp"
#include "grainmm/dsp_utils.hpp"
namespace grainmm {
PreparedClock prepare_clock(float bpm, int rate) {
    dsp::checked_rate(rate);
    bpm = dsp::limit(bpm, 20, 300, 120);
    return {static_cast<std::uint64_t>(std::llround(
        static_cast<double>(bpm) * static_cast<double>(FrameClock::unit) / (60.0 * rate)))};
}
} // namespace grainmm
