// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/parameter_editor.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
namespace {
int fail(int line) {
    std::fprintf(stderr, "parameter editor regression at line%d\n", line);
    return 1;
}
} // namespace

int main() {
    grainmm::ParameterEditor editor;
    if (editor.selection() != 0 || std::strcmp(editor.label_at(0), "SIZE MS") ||
        editor.value_at(0) != 80 || std::strcmp(editor.label_at(6), "") ||
        editor.value_at(6) != 0 || std::strcmp(editor.label(), "SIZE MS") || editor.value() != 80)
        return fail(__LINE__);
    editor.adjust(1);
    if (editor.value() != 85)
        return fail(__LINE__);
    editor.select(-1);
    if (std::strcmp(editor.label(), "GAIN"))
        return fail(__LINE__);
    for (int field = 0; field < 6; ++field) {
        for (int i = 0; i < 1000; ++i)
            editor.adjust(1);
        const auto high = editor.value();
        editor.adjust(1);
        if (!std::isfinite(high) || editor.value() != high)
            return fail(__LINE__);
        for (int i = 0; i < 1000; ++i)
            editor.adjust(-1);
        const auto low = editor.value();
        editor.adjust(-1);
        if (!std::isfinite(low) || editor.value() != low || low > high)
            return fail(__LINE__);
        editor.select(1);
    }
    if (std::strcmp(editor.label(), "GAIN") || editor.parameters().gain != 0)
        return fail(__LINE__);
    editor.select(0);
    editor.adjust(0);
    if (editor.value() != 0)
        return fail(__LINE__);
    editor.switch_page();
    if (std::strcmp(editor.page_label(), "DELAY") || std::strcmp(editor.label(), "MIX %"))
        return fail(__LINE__);
    editor.select(1);
    if (std::strcmp(editor.label(), "TIME MS") || editor.value() != 450)
        return fail(__LINE__);
    editor.select(-1);
    editor.adjust(1);
    if (std::abs(editor.snapshot().delay.mix - 0.05F) > 1e-6F)
        return fail(__LINE__);
    const auto saved = editor.snapshot();
    grainmm::ParameterEditor restored;
    restored.restore(saved);
    restored.switch_page();
    restored.select(-1);
    if (restored.value() != editor.value())
        return fail(__LINE__);
    for (int field = 0; field < 6; ++field) {
        for (int i = 0; i < 4000; ++i)
            restored.adjust(1);
        const auto high = restored.value();
        restored.adjust(1);
        if (!std::isfinite(high) || restored.value() != high)
            return fail(__LINE__);
        for (int i = 0; i < 4000; ++i)
            restored.adjust(-1);
        const auto low = restored.value();
        restored.adjust(-1);
        if (!std::isfinite(low) || restored.value() != low || low > high)
            return fail(__LINE__);
        restored.select(1);
    }
    while (restored.selection() != 0)
        restored.select(1);
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "EQ"))
        return fail(__LINE__);
    restored.adjust(1);
    if (restored.value() != 1)
        return fail(__LINE__);
    for (int i = 0; i < 100; ++i)
        restored.adjust(1);
    if (restored.value() != 20)
        return fail(__LINE__);
    for (int i = 0; i < 100; ++i)
        restored.adjust(-1);
    if (restored.value() != -20)
        return fail(__LINE__);
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "REVERB") || std::strcmp(restored.label(), "ROOM SIZE"))
        return fail(__LINE__);
    restored.select(-1);
    restored.adjust(1);
    if (std::abs(restored.snapshot().reverb.mix - 0.05F) > 1e-6F)
        return fail(__LINE__);
    restored.select(1);
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "MOTION") ||
        std::strcmp(restored.label(), "REVERSE CHANCE"))
        return fail(__LINE__);
    restored.adjust(1);
    if (std::abs(restored.snapshot().grain.reverse - 0.1F) > 1e-6F)
        return fail(__LINE__);
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "OSC") || restored.requires_sample())
        return fail(__LINE__);
    restored.adjust(1);
    if (std::abs(restored.snapshot().oscillator.level - 0.01F) > 1e-6F)
        return fail(__LINE__);
    restored.select(1);
    restored.select(1);
    restored.adjust(1);
    if (std::strcmp(restored.choice_at(2), "TRIANGLE"))
        return fail(__LINE__);
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "COMP"))
        return fail(__LINE__);
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "TAPE") || restored.requires_sample())
        return fail(__LINE__);
    for (int field = 0; field < 6; ++field) {
        for (int i = 0; i < 4000; ++i)
            restored.adjust(1);
        const auto high = restored.value();
        restored.adjust(1);
        if (restored.value() != high)
            return fail(__LINE__);
        for (int i = 0; i < 4000; ++i)
            restored.adjust(-1);
        const auto low = restored.value();
        restored.adjust(-1);
        if (!std::isfinite(low) || restored.value() != low || low > high)
            return fail(__LINE__);
        restored.select(1);
    }
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "RESONATE") || restored.requires_sample())
        return fail(__LINE__);
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "RHYTHM") || restored.requires_sample())
        return fail(__LINE__);
    while (restored.selection() != 1)
        restored.select(1);
    if (std::strcmp(restored.choice_at(1), "QUARTER"))
        return fail(__LINE__);
    restored.adjust(1);
    if (std::strcmp(restored.choice_at(1), "EIGHTH"))
        return fail(__LINE__);
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "SAMPLE") || !restored.requires_sample())
        return fail(__LINE__);
    for (unsigned page = 0; page < grainmm::ParameterEditor::page_count; ++page) {
        for (unsigned field = 0; field < grainmm::ParameterEditor::field_count; ++field) {
            for (int i = 0; i < 2000; ++i)
                restored.adjust(1, 10);
            const auto high = restored.value();
            restored.adjust(1);
            if (!std::isfinite(high) || restored.value() != high)
                return fail(__LINE__);
            for (int i = 0; i < 2000; ++i)
                restored.adjust(-1, 10);
            const auto low = restored.value();
            restored.adjust(-1);
            if (!std::isfinite(low) || restored.value() != low || low > high)
                return fail(__LINE__);
            restored.select(1);
        }
        restored.switch_page();
    }
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "HARMONY") || !restored.requires_sample())
        return fail(__LINE__);
    for (const auto *page : {"CRUSH", "FILTER", "CHORUS", "SLICE", "BALLS", "BOWL", "EVOLVE",
                             "DRONE", "GENERATE", "OUTPUT", "OSC B", "TIMBRE", "GROOVE"}) {
        restored.switch_page();
        if (std::strcmp(restored.page_label(), page) ||
            (restored.requires_sample() !=
             ((std::strcmp(page, "SLICE") == 0 || std::strcmp(page, "EVOLVE") == 0 ||
               std::strcmp(page, "GROOVE") == 0))))
            return fail(__LINE__);
    }
    restored.switch_page();
    if (std::strcmp(restored.page_label(), "GRAIN"))
        return fail(__LINE__);
    for (const auto *page : {"GROOVE", "TIMBRE", "OSC B", "OUTPUT", "GENERATE", "DRONE", "EVOLVE",
                             "BOWL", "BALLS", "SLICE", "CHORUS", "FILTER", "CRUSH"}) {
        restored.switch_page(-1);
        if (std::strcmp(restored.page_label(), page))
            return fail(__LINE__);
    }
    restored.switch_page(-1);
    if (std::strcmp(restored.page_label(), "HARMONY") || !restored.requires_sample())
        return fail(__LINE__);
    restored.switch_page(-1);
    if (std::strcmp(restored.page_label(), "SAMPLE"))
        return fail(__LINE__);
    restored.switch_page(-1);
    if (std::strcmp(restored.page_label(), "RHYTHM"))
        return fail(__LINE__);
    restored.switch_page(-1);
    if (std::strcmp(restored.page_label(), "RESONATE"))
        return fail(__LINE__);
    restored.switch_page(-1);
    if (std::strcmp(restored.page_label(), "TAPE"))
        return fail(__LINE__);
    restored.switch_page(-1);
    if (std::strcmp(restored.page_label(), "COMP"))
        return fail(__LINE__);
    restored.switch_page(-1);
    if (std::strcmp(restored.page_label(), "OSC"))
        return fail(__LINE__);
    for (unsigned page = 0; page < grainmm::ParameterEditor::page_count; ++page) {
        grainmm::ParameterEditor fresh;
        for (unsigned i = 0; i < page; ++i)
            fresh.switch_page();
        for (unsigned field = 0; field < grainmm::ParameterEditor::field_count; ++field) {
            const auto before = fresh.value();
            const auto range = fresh.range_at(field);
            if (!std::isfinite(range.first) || !std::isfinite(range.second) ||
                range.first > before || range.second < before)
                return fail(__LINE__);
            fresh.adjust(1);
            fresh.reset_selected();
            if (fresh.value() != before)
                return fail(__LINE__);
            fresh.select(1);
        }
    }
    // Group-local bounds must match the full instrument reference, including
    // dependent sample-region bounds and pages with independent UI/storage order.
    for (unsigned pass = 0; pass < 2; ++pass)
        for (unsigned page = 0; page < grainmm::ParameterEditor::page_count; ++page) {
            grainmm::ParameterEditor probe;
            auto values = probe.snapshot();
            values.grain.start = pass ? .4F : 0;
            values.grain.end = pass ? .65F : 1;
            probe.restore(values);
            probe.set_page(page);
            const auto &schema = grainmm::parameter_pages()[page];
            for (unsigned i = 0; i < 6; ++i) {
                const auto &d = schema.fields[schema.display_order[i]];
                auto lo = values, hi = values;
                d.write(lo) = -std::numeric_limits<float>::max();
                d.write(hi) = std::numeric_limits<float>::max();
                lo = grainmm::bounded(lo);
                hi = grainmm::bounded(hi);
                const auto actual = probe.range_at(i);
                if (actual.first != d.read(lo) || actual.second != d.read(hi))
                    return fail(__LINE__);
            }
        }
    // Common wet/dry and gain controls stay in the same physical positions.
    grainmm::ParameterEditor layout;
    for (unsigned page = 0; page < layout.page_count; ++page) {
        const auto *name = layout.page_label();
        const bool wet_dry = std::strcmp(name, "DELAY") == 0 || std::strcmp(name, "REVERB") == 0 ||
                             std::strcmp(name, "COMP") == 0 || std::strcmp(name, "TAPE") == 0 ||
                             std::strcmp(name, "RESONATE") == 0 ||
                             std::strcmp(name, "CRUSH") == 0 || std::strcmp(name, "FILTER") == 0 ||
                             std::strcmp(name, "CHORUS") == 0;
        if (wet_dry) {
            if (std::strcmp(layout.label_at(5), "MIX %") != 0)
                return fail(__LINE__);
            layout.select(-1);
            layout.adjust(1);
            if (std::abs(layout.value() - .05F) > 1e-6F || !layout.effect_active())
                return fail(__LINE__);
            layout.reset_selected();
            if (layout.value() != 0 || layout.effect_active())
                return fail(__LINE__);
            layout.select(1);
        }
        if ((std::strcmp(name, "TAPE") == 0 || std::strcmp(name, "CRUSH") == 0 ||
             std::strcmp(name, "FILTER") == 0) &&
            std::strcmp(layout.label_at(4), "OUTPUT DB") != 0)
            return fail(__LINE__);
        if (std::strcmp(name, "COMP") == 0 && std::strcmp(layout.label_at(4), "MAKEUP DB") != 0)
            return fail(__LINE__);
        layout.switch_page();
    }
    grainmm::ParameterEditor percent;
    auto full = percent.snapshot();
    full.drone_synth.level = .2F;
    percent.restore(full);
    percent.set_page(19);
    if (percent.display_value_at(0) != 20 || percent.value_at(0) != .2F ||
        percent.decimals_at(0) != 0)
        return fail(__LINE__);
    percent.adjust(1);
    if (percent.display_value_at(0) != 25)
        return fail(__LINE__);
    for (int n = 0; n < 5; ++n)
        percent.adjust(-1);
    if (percent.value_at(0) != 0)
        return fail(__LINE__);
    percent.set_page(7);
    percent.select(-1);
    for (int n = 0; n < 6; ++n)
        percent.adjust(1);
    for (int n = 0; n < 6; ++n)
        percent.adjust(-1);
    if (percent.value() != 0 || percent.effect_active())
        return fail(__LINE__);
    return 0;
}
