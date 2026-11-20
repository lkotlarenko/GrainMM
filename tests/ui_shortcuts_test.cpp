// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/ui_shortcuts.hpp"
#include <cstdio>
int main() {
    using U = grainmm::UiShortcuts;
    using B = U::Button;
    using T = U::Tap;
    U u;
    if (u.key(B::Select, true, 0) != T::None || u.key(B::Select, false, 10) != T::Select)
        return 1;
    if (u.key(B::Start, true, 20) != T::None || u.key(B::Start, false, 30) != T::Start)
        return 1;
    u.key(B::Start, true, 100);
    u.key(B::Select, true, 200);
    u.key(B::Select, true, 250);
    if (u.poll(999, true) || !u.poll(1000, true) || u.poll(3000, true))
        return 1;
    if (u.key(B::Select, false, 3001) != T::None || u.key(B::Start, false, 3002) != T::None)
        return 1;
    u.key(B::Select, true, 0xffffff00U);
    u.key(B::Start, true, 0xffffff00U);
    if (!u.poll(0x300U, true))
        return 1;
    u.key(B::Start, false, 0x301U);
    u.key(B::Select, false, 0x302U);
    u.key(B::Start, true, 4000);
    u.cancel();
    if (u.key(B::Start, false, 5000) != T::None)
        return 1;
    u.key(B::Start, true, 6000);
    u.key(B::Select, true, 6001);
    if (u.poll(7000, false) || u.poll(8000, true))
        return 1;
    u.key(B::Start, false, 8001);
    u.key(B::Select, false, 8002);
    u.key(B::Start, true, 9000);
    u.key(B::Select, true, 9001);
    if (u.key(B::Start, false, 9010) != T::None || u.key(B::Select, false, 9020) != T::None)
        return 1;
    u.key(B::Start, true, 10000);
    u.key(B::Select, true, 10001);
    u.key(B::Select, false, 10100);
    u.key(B::Select, true, 10800);
    if (u.poll(10801, true) || u.poll(11599, true) || !u.poll(11600, true))
        return 1; // Releasing either key restarts the continuous overlap interval.
    U modifier;
    if (modifier.consume_select_modifier())
        return 1;
    modifier.key(B::Select, true, 0);
    if (!modifier.consume_select_modifier() || modifier.consume_select_modifier())
        return 1;
    modifier.key(B::Start, true, 1);
    if (modifier.poll(1000, true) || modifier.key(B::Select, false, 1001) != T::None ||
        modifier.key(B::Start, false, 1002) != T::None)
        return 1;
    modifier.key(B::Select, true, 1100);
    if (modifier.key(B::Select, false, 1200) != T::Select)
        return 1;
    modifier.key(B::Start, true, 1300);
    modifier.key(B::Select, true, 1400);
    if (modifier.consume_select_modifier() || !modifier.poll(2200, true))
        return 1;
    modifier.key(B::Select, false, 2201);
    modifier.key(B::Start, false, 2202);
    modifier.key(B::Select, true, 2300);
    modifier.cancel(); // Focus loss must not leave an active modifier.
    if (modifier.consume_select_modifier() || modifier.key(B::Select, false, 2400) != T::None)
        return 1;
    std::puts("UI shortcuts: taps, overlapping hold, repeats, wrap, cancellation and rearm passed");
}
