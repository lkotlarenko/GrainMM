// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstdint>
namespace grainmm {
struct PreparedClock {
    std::uint64_t increment = 0;
};
PreparedClock prepare_clock(float bpm, int rate);
// Audio-owned Q48 beat clock. Copies are immutable block anchors for consumers.
// Integer phase makes boundaries independent of render chunk size. No wall clock.
class FrameClock {
  public:
    static constexpr std::uint64_t unit = std::uint64_t{1} << 48;
    void apply(PreparedClock p) noexcept { increment_ = p.increment; }
    void reset() noexcept {
        phase_ = previous_ = beats_ = 0;
        first_ = true;
    }
    bool boundary(unsigned division) const noexcept {
        return first_ || previous_ > phase_ ||
               ((previous_ ^ phase_) >> (48U - (division < 4 ? division : 3))) != 0;
    }
    void advance() noexcept {
        previous_ = phase_;
        const auto next = phase_ + increment_;
        beats_ += static_cast<std::uint32_t>(next >> 48);
        phase_ = next & (unit - 1);
        first_ = false;
    }
    void advance(unsigned frames) noexcept {
        for (unsigned i = 0; i < frames; ++i)
            advance();
    }
    std::uint64_t phase() const noexcept { return phase_; }
    std::uint32_t beats() const noexcept { return beats_; }
    bool first() const noexcept { return first_; }

  private:
    std::uint64_t phase_ = 0, previous_ = 0, increment_ = 0;
    std::uint32_t beats_ = 0;
    bool first_ = true;
};
} // namespace grainmm
