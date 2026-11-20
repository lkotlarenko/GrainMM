// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/parameter_schema.hpp"
#include <limits>
#include <utility>
namespace grainmm {
// UI-only editor. Labels/accessors/steps and codec order share the typed schema.
class ParameterEditor {
  public:
    static constexpr unsigned page_count = parameter_page_count;
    static constexpr std::size_t field_count = 6;
    void restore(const InstrumentParameters &p) noexcept { values_ = bounded(p); }
    InstrumentParameters snapshot() const noexcept { return values_; }
    void switch_page(int direction = 1) noexcept {
        if (direction)
            page_ = (page_ + (direction < 0 ? page_count - 1 : 1U)) % page_count;
    }
    void set_page(unsigned page) noexcept {
        if (page < page_count)
            page_ = page;
    }
    void select(int direction) noexcept {
        if (direction)
            selected_ = (selected_ + (direction < 0 ? field_count - 1 : 1)) % field_count;
    }
    void show_oscillator() noexcept {
        page_ = 5;
        selected_ = 0;
    }
    void show_sample() noexcept {
        page_ = 10;
        selected_ = 0;
    }
    bool requires_sample() const noexcept { return parameter_pages()[page_].sample_required; }
    const char *page_label() const noexcept { return parameter_pages()[page_].label; }
    bool effect_active() const noexcept {
        switch (page_) {
        case 1:
            return values_.delay.mix > 0;
        case 2:
            return std::any_of(values_.eq_db.begin(), values_.eq_db.end(),
                               [](float x) { return x != 0; });
        case 3:
            return values_.reverb.mix > 0;
        case 6:
            return values_.compressor.mix > 0;
        case 7:
            return values_.tape.mix > 0;
        case 8:
            return values_.resonator.mix > 0;
        case 9:
            return values_.rhythm.tremolo > 0 || values_.rhythm.pan > 0;
        case 12:
            return values_.crusher.mix > 0;
        case 13:
            return values_.filter.mix > 0;
        case 14:
            return values_.chorus.mix > 0;
        case 21:
            return values_.output.gain_db != 0 || values_.output.limiter != 0 ||
                   values_.output.balance != 0 || values_.output.mono != 0;
        default:
            return false;
        }
    }
    unsigned page_index() const noexcept { return page_; }
    std::pair<float, float> range_at(std::size_t i) const noexcept {
        if (i >= field_count)
            return {0, 0};
        const auto &d = definition(i);
        if (d.range)
            return d.range(values_);
        auto lo = values_, hi = values_;
        d.write(lo) = -std::numeric_limits<float>::max();
        d.write(hi) = std::numeric_limits<float>::max();
        lo = bounded(lo);
        hi = bounded(hi);
        return {d.read(lo), d.read(hi)};
    }
    std::size_t selection() const noexcept { return selected_; }
    const GrainParameters &parameters() const noexcept { return values_.grain; }
    const char *label_at(std::size_t i) const noexcept {
        return i < field_count ? definition(i).label : "";
    }
    unsigned decimals_at(std::size_t i) const noexcept {
        if (i >= field_count)
            return 0;
        const auto &d = definition(i);
        const auto step = d.step * d.display_scale;
        return step >= 1 ? 0U : step >= .1F ? 1U : 2U;
    }
    float value_at(std::size_t i) const noexcept {
        return i < field_count ? definition(i).read(values_) : 0;
    }
    float display_value_at(std::size_t i) const noexcept {
        return i < field_count ? value_at(i) * definition(i).display_scale : 0;
    }
    const char *choice_at(std::size_t i) const noexcept {
        if (i >= field_count)
            return nullptr;
        const auto &d = definition(i);
        if (!d.choices)
            return nullptr;
        const auto v = static_cast<std::size_t>(d.read(values_));
        return d.choices && v < d.choice_count ? d.choices[v] : nullptr;
    }
    const char *label() const noexcept { return label_at(selected_); }
    float value() const noexcept { return value_at(selected_); }
    void adjust(int direction, unsigned multiplier = 1) noexcept {
        if (!direction)
            return;
        const auto &d = definition(selected_);
        d.write(values_) += (direction < 0 ? -1.0F : 1.0F) * d.step *
                            static_cast<float>(d.choices ? 1U : std::clamp(multiplier, 1U, 10U));
        values_ = bounded(values_);
        // Avoid a displayed zero keeping a whole effect/source running after
        // repeated floating-point decrements (for example0.30 minus6×0.05).
        if (std::abs(d.read(values_)) < d.step * .0001F) {
            d.write(values_) = 0;
            values_ = bounded(values_);
        }
    }
    void reset_selected() noexcept {
        const InstrumentParameters defaults;
        const auto &d = definition(selected_);
        d.write(values_) = d.read(defaults);
        values_ = bounded(values_);
    }
    void reset_all_effects(ResetScope scope = ResetScope::Rack) noexcept {
        values_ = grainmm::reset_effects(values_, scope);
    }

  private:
    const ParameterDefinition &definition(std::size_t i) const noexcept {
        const auto &page = parameter_pages()[page_];
        return page.fields[page.display_order[i]];
    }
    InstrumentParameters values_{};
    unsigned page_ = 0;
    std::size_t selected_ = 0;
};
} // namespace grainmm
