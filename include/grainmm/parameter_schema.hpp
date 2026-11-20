// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/instrument_parameters.hpp"
#include <array>
#include <cstddef>
#include <utility>
namespace grainmm {
// Control-thread metadata; typed accessors, no offsets/type punning or audio use.
struct ParameterDefinition {
    const char *label;
    float step;
    float (*read)(const InstrumentParameters &) noexcept;
    float &(*write)(InstrumentParameters &) noexcept;
    const char *const *choices = nullptr;
    std::size_t choice_count = 0;
    float display_scale = 1;
    std::pair<float, float> (*range)(const InstrumentParameters &) noexcept = nullptr;
};
struct PageDefinition {
    const char *label;
    bool sample_required;
    // Storage order is immutable. UI positions are an independent permutation.
    std::array<ParameterDefinition, 6> fields;
    std::array<std::size_t, 6> display_order{0, 1, 2, 3, 4, 5};
};
inline constexpr unsigned parameter_page_count = 25;
inline constexpr std::size_t parameter_count = 6 * parameter_page_count;
const std::array<PageDefinition, parameter_page_count> &parameter_pages() noexcept;
const ParameterDefinition &parameter_definition(std::size_t index);
// Historical schemas are append-only prefixes. Source selection is stored separately.
std::size_t parameter_count_for_version(int version);
} // namespace grainmm
