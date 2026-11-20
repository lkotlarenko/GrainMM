// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
namespace grainmm {
struct SliceStep {
    unsigned region = 0;
    bool enabled = true, reverse = false;
    float probability = 1;
};
// Independent versioned musical data, not spare scalar parameter fields.
// Positive widths define relative region lengths; preparation normalizes the
// first REGIONS entries into ordered cut boundaries outside the audio callback.
struct SlicePattern {
    bool manual = false;
    std::array<SliceStep, 16> steps = [] {
        std::array<SliceStep, 16> result{};
        for (unsigned i = 0; i < result.size(); ++i)
            result[i].region = i % 8;
        return result;
    }();
    std::array<float, 16> widths = [] {
        std::array<float, 16> result{};
        for (auto &v : result)
            v = 1;
        return result;
    }();
};
SlicePattern bounded(SlicePattern p) noexcept;
bool operator==(const SlicePattern &a, const SlicePattern &b) noexcept;
inline bool operator!=(const SlicePattern &a, const SlicePattern &b) noexcept { return !(a == b); }
std::array<float, 17> slice_cuts(const SlicePattern &p, unsigned regions) noexcept;
enum class PatternKey { Previous, Next, Up, Down, Left, Right, Rest, Mode };
// UI-only cursor/edit state; the pattern remains in the instrument snapshot.
class PatternEditor {
  public:
    bool input(PatternKey key, SlicePattern &pattern, unsigned regions, unsigned multiplier = 1);
    unsigned step() const noexcept { return step_; }
    unsigned field() const noexcept { return field_; }

  private:
    unsigned step_ = 0, field_ = 0;
};
} // namespace grainmm
