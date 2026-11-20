// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/sample.hpp"
#include <cmath>
#include <stdexcept>
#include <utility>

namespace grainmm {
Sample::Sample(int rate, int channels, std::vector<float> values)
    : rate_(rate), channels_(channels), values_(std::move(values)) {
    if (rate < 8000 || rate > 96000 || (channels != 1 && channels != 2) || values_.empty() ||
        values_.size() > max_values || values_.size() % static_cast<unsigned>(channels) != 0)
        throw std::invalid_argument("sample must be bounded mono/stereo PCM at 8000..96000 Hz");
    for (const float value : values_)
        if (!std::isfinite(value) || value < -1.0F || value > 1.0F)
            throw std::invalid_argument("sample values must be finite and normalized");
}
} // namespace grainmm
