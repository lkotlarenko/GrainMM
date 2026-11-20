// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace grainmm {
class DirectionalRepeat {
  public:
    enum class Direction : unsigned { Up, Down, Left, Right, None };
    struct Step {
        Direction direction = Direction::None;
        unsigned multiplier = 1;
    };
    bool press(Direction d, std::uint32_t now) noexcept {
        if (d == Direction::None)
            return false;
        auto &held = held_[static_cast<unsigned>(d)];
        if (held)
            return false;
        held = true;
        active_ = d;
        since_ = now;
        next_ = now + 320;
        return true;
    }
    void release(Direction d) noexcept {
        if (d == Direction::None)
            return;
        held_[static_cast<unsigned>(d)] = false;
        if (active_ == d)
            active_ = Direction::None;
    }
    Step poll(std::uint32_t now) noexcept {
        if (active_ == Direction::None || now - next_ > 0x7fffffffU)
            return {};
        const bool edit = active_ == Direction::Left || active_ == Direction::Right;
        next_ =
            now + (edit ? 60U : 140U); // At most one step per UI frame; never catch up in a burst.
        const auto duration = now - since_;
        return {active_, edit ? (duration >= 3000 ? 10U : duration >= 1500 ? 5U : 1U) : 1U};
    }
    void cancel() noexcept {
        active_ = Direction::None;
    } // Held keys must release before editing another view.
    void focus_lost() noexcept {
        active_ = Direction::None;
        held_ = {};
    }

  private:
    std::array<bool, 4> held_{};
    Direction active_ = Direction::None;
    std::uint32_t since_ = 0, next_ = 0;
};
} // namespace grainmm
