// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
namespace grainmm {
struct ThemeProfile {
    const char *name;
    std::array<unsigned char, 3> accent, signal;
};
inline constexpr std::array<ThemeProfile, 6> themes{{{"BURGUNDY", {218, 84, 118}, {106, 218, 206}},
                                                     {"AMBER", {255, 166, 75}, {110, 216, 177}},
                                                     {"MINT", {102, 220, 180}, {238, 198, 126}},
                                                     {"ICE", {114, 190, 244}, {146, 224, 218}},
                                                     {"VIOLET", {187, 145, 244}, {239, 172, 208}},
                                                     {"MONO", {224, 225, 215}, {169, 180, 184}}}};
unsigned load_theme(const char *directory);
void save_theme(const char *directory, unsigned theme);
} // namespace grainmm
