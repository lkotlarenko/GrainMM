// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstddef>
#include <vector>

namespace grainmm {
// Immutable, normalized, interleaved audio. At most 16 MiB decoded storage.
class Sample {
  public:
    static constexpr std::size_t max_values = 4 * 1024 * 1024;
    Sample(int rate, int channels, std::vector<float> values);
    int rate() const noexcept { return rate_; }
    int channels() const noexcept { return channels_; }
    std::size_t frames() const noexcept {
        return values_.size() / static_cast<unsigned>(channels_);
    }
    float value(std::size_t frame, int channel) const noexcept {
        // Keep bounds checks visible to the optimizer in the per-voice interpolation path.
        if (frame >= frames() || channel < 0 || channel > 1)
            return 0.0F;
        return values_[frame * static_cast<unsigned>(channels_) +
                       static_cast<unsigned>(channels_ == 1 ? 0 : channel)];
    }

  private:
    int rate_, channels_;
    std::vector<float> values_;
};
} // namespace grainmm
