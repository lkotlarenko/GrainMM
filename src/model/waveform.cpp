// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/waveform.hpp"
#include <algorithm>
namespace grainmm {
Waveform::Waveform(const Sample &sample) noexcept
    : seconds_(static_cast<double>(sample.frames()) / sample.rate()) {
    const auto frames = sample.frames();
    for (std::size_t column = 0; column < columns; ++column) {
        const auto first = frames * column / columns;
        const auto end = std::min(frames, std::max(first + 1, frames * (column + 1) / columns));
        auto &peak = peaks_[column];
        peak = {1, -1};
        for (auto frame = first; frame < end; ++frame)
            for (int channel = 0; channel < sample.channels(); ++channel) {
                const float x = sample.value(frame, channel);
                peak.low = std::min(peak.low, x);
                peak.high = std::max(peak.high, x);
            }
    }
}
} // namespace grainmm
