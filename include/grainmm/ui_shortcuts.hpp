// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstdint>
namespace grainmm {
// Deferred short taps avoid opening dialogs while the reset chord is being held.
// Both buttons released rearms the chord. No SDL dependency; uint32 tick-wrap safe.
class UiShortcuts {
  public:
    enum class Button { Start, Select };
    enum class Tap { None, Start, Select };
    Tap key(Button button, bool down, std::uint32_t now) noexcept {
        bool &held = button == Button::Start ? start_ : select_;
        if (down) {
            if (held)
                return Tap::None;
            held = true;
            if (start_ && select_) {
                combined_ = true;
                if (!fired_)
                    since_ = now;
            }
            return Tap::None;
        }
        const bool was_held = held;
        held = false;
        const bool single = was_held && !combined_ && !cancelled_;
        if (!start_ && !select_)
            combined_ = fired_ = cancelled_ = false;
        return single ? (button == Button::Start ? Tap::Start : Tap::Select) : Tap::None;
    }
    bool poll(std::uint32_t now, bool eligible) noexcept {
        if (!start_ || !select_)
            return false;
        if (!eligible) {
            fired_ = true;
            return false;
        }
        if (combined_ && !fired_ && !cancelled_ && now - since_ >= 800) {
            fired_ = true;
            return true;
        }
        return false;
    }
    // Only SELECT-first gestures qualify. Consume its deferred help tap and
    // reset chord until both system buttons have been released.
    bool consume_select_modifier() noexcept {
        if (!select_ || start_ || combined_ || cancelled_)
            return false;
        cancelled_ = true;
        return true;
    }
    void cancel() noexcept { cancelled_ = fired_ = start_ || select_; }

  private:
    bool start_ = false, select_ = false, combined_ = false, fired_ = false, cancelled_ = false;
    std::uint32_t since_ = 0;
};
} // namespace grainmm
