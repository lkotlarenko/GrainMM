// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/page_navigator.hpp"
int main() {
    using grainmm::PageNavigator;
    bool seen[PageNavigator::destinations]{};
    PageNavigator nav;
    for (unsigned group = 0; group < 4; ++group) {
        for (unsigned i = 0; i < nav.count(); ++i) {
            auto page = nav.page_at(i);
            if (page >= PageNavigator::destinations || seen[page])
                return 1;
            seen[page] = true;
            nav.open(page);
            if (nav.page() != page || nav.section() != group)
                return 1;
        }
        nav.group(1);
    }
    for (bool value : seen)
        if (!value)
            return 1;
    nav.open(16);
    nav.move(1);
    if (nav.page() != 17)
        return 1;
    nav.move(1);
    if (nav.page() != 18)
        return 1;
    nav.move(1);
    if (nav.page() != 19)
        return 1;
    nav.move(1);
    if (nav.page() != 20)
        return 1;
    nav.move(1);
    if (nav.page() != 22)
        return 1;
    if (nav.bank_start() != 12 || nav.visible_count() != 2)
        return 1;
    nav.move(1);
    if (nav.page() != 23)
        return 1;
    nav.move(1);
    if (nav.page() != 0 || nav.bank_start() != 0)
        return 1;
    nav.open(20);
    if (nav.page() != 20)
        return 1;
    nav.group(-1);
    if (nav.section() != 3 || nav.page() != PageNavigator::export_page)
        return 1;
    if (PageNavigator::group_for(21) != 3 ||
        PageNavigator::group_for(PageNavigator::pattern_page) != 0)
        return 1;
    // Main shoulders visit contiguous SOURCE, FX, TEMPO groups exactly once.
    unsigned page = 0, previous_group = 0, changes = 0;
    bool visited[26]{};
    for (unsigned i = 0; i < 26; ++i) {
        if (page >= 26 || visited[page])
            return 1;
        visited[page] = true;
        const auto group = PageNavigator::group_for(page);
        if (group != previous_group) {
            if (group != previous_group + 1)
                return 1;
            ++changes;
            previous_group = group;
        }
        const auto next = PageNavigator::adjacent(page, 1);
        if (PageNavigator::adjacent(next, -1) != page)
            return 1;
        page = next;
    }
    if (page != 0 || changes != 3)
        return 1;
}
