// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
namespace grainmm {
// UI-only grouping. Codec/page IDs remain unchanged and append-only.
class PageNavigator {
  public:
    static constexpr unsigned groups = 4;
    static constexpr unsigned style_page = 25;
    static constexpr unsigned export_page = 26, patches_page = 27, pattern_page = 28;
    static constexpr unsigned destinations = 29;
    static constexpr std::array<unsigned, 14> sources{0,  4,  5,  10, 11, 15, pattern_page,
                                                      16, 17, 18, 19, 20, 22, 23};
    static constexpr std::array<unsigned, 9> effects{13, 8, 2, 7, 12, 14, 6, 1, 3};
    static constexpr std::array<unsigned, 4> tools{export_page, patches_page, 21, style_page};
    static unsigned group_for(unsigned page) noexcept {
        for (auto p : sources)
            if (page == p)
                return 0;
        for (auto p : tools)
            if (page == p)
                return 3;
        return page == 9 || page == 24 ? 2U : 1U;
    }
    static const char *group_label(unsigned group) noexcept {
        return group == 0 ? "SOURCE" : group == 1 ? "FX" : group == 2 ? "TEMPO" : "TOOLS";
    }
    static unsigned adjacent(unsigned page, int direction) noexcept {
        // Shoulder browsing visits sound pages only; actions require explicit A
        // in the navigator so passing a tab never pauses playback for export.
        constexpr std::array<unsigned, 26> order{0,  4,  5,  10, 11, 15, 16, 17,        18,
                                                 19, 20, 22, 23, 13, 8,  2,  7,         12,
                                                 14, 6,  1,  3,  9,  24, 21, style_page};
        for (unsigned i = 0; i < order.size(); ++i)
            if (order[i] == page)
                return order[(i + (direction < 0 ? order.size() - 1 : 1U)) % order.size()];
        return 0;
    }
    void open(unsigned page) noexcept {
        group_ = group_for(page);
        selected_ = 0;
        for (unsigned i = 0; i < count(); ++i)
            if (page_at(i) == page)
                selected_ = i;
    }
    void group(int direction) noexcept {
        if (direction) {
            group_ = (group_ + (direction < 0 ? 3U : 1U)) % groups;
            selected_ = 0;
        }
    }
    void move(int direction) noexcept {
        if (direction)
            selected_ = (selected_ + (direction < 0 ? count() - 1 : 1U)) % count();
    }
    unsigned count() const noexcept {
        return group_ == 0 ? 14U : group_ == 1 ? 9U : group_ == 3 ? 4U : 2U;
    }
    unsigned page_at(unsigned index) const noexcept {
        if (index >= count())
            return 0;
        return group_ == 0   ? sources[index]
               : group_ == 1 ? effects[index]
               : group_ == 2 ? (index == 0 ? 9U : 24U)
                             : tools[index];
    }
    static constexpr unsigned bank_size = 12;
    unsigned bank_start() const noexcept { return selected_ / bank_size * bank_size; }
    unsigned visible_count() const noexcept {
        const auto remaining = count() - bank_start();
        return remaining < bank_size ? remaining : bank_size;
    }
    unsigned page() const noexcept { return page_at(selected_); }
    unsigned selection() const noexcept { return selected_; }
    unsigned section() const noexcept { return group_; }

  private:
    unsigned group_ = 0, selected_ = 0;
};
} // namespace grainmm
