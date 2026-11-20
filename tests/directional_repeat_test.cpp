// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/directional_repeat.hpp"
int main() {
    using R = grainmm::DirectionalRepeat;
    using D = R::Direction;
    R r;
    if (!r.press(D::Right, 100) || r.press(D::Right, 200) || r.poll(419).direction != D::None)
        return 1;
    if (r.poll(420).direction != D::Right || r.poll(421).direction != D::None)
        return 1;
    if (r.poll(1600).multiplier != 5 || r.poll(3100).multiplier != 10)
        return 1;
    r.release(D::Right);
    if (r.poll(9000).direction != D::None)
        return 1;
    r.press(D::Left, 10000);
    r.cancel();
    if (r.poll(11000).direction != D::None || r.press(D::Left, 11000))
        return 1;
    r.release(D::Left);
    if (!r.press(D::Left, 11001))
        return 1;
    r.press(D::Right, 11002);
    r.release(D::Right);
    if (r.poll(12000).direction != D::None)
        return 1;
    r.focus_lost();
    r.press(D::Up, 0xffffff00U);
    if (r.poll(0x40U).direction != D::Up)
        return 1;
    if (r.poll(5000).multiplier != 1)
        return 1; // Navigation never accelerates across cards/dialogs.
    if (r.poll(5001).direction != D::None)
        return 1; // No delayed-frame catch-up.
}
