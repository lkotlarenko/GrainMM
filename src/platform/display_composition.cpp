// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/display.hpp"
#include "grainmm/waveform.hpp"
#include <algorithm>
#include <cstdio>
namespace grainmm {
void Display::draw_pattern(const InstrumentParameters &p, const PatternEditor &editor,
                           const Waveform *wave, unsigned sequence, bool playing) {
    ++requests_;
    const std::array<std::string, 10> frame{std::to_string(editor.step()),
                                            std::to_string(editor.field()),
                                            std::to_string(p.slice.regions),
                                            std::to_string(p.slice.mode),
                                            playing ? "1" : "0",
                                            std::to_string(p.grain.start),
                                            std::to_string(p.grain.end)};
    const auto now = SDL_GetTicks();
    const bool layout = invalid_ || view_ != View::Pattern || previous_ != frame ||
                        cached_pattern_ != p.slice_pattern;
    if (!layout && (last_sequence_ == sequence || now - last_animation_ < 50))
        return;
    chrome("SLICE PATTERN", 0, playing);
    char context[96]{};
    std::snprintf(context, sizeof(context), "%s / STEP %02u OF 16 / %s",
                  p.slice_pattern.manual ? "MANUAL" : "GENERATED", editor.step() + 1,
                  p.slice.mode == 0 ? "MODE OFF - EDIT OR Y TO ENABLE" : "L/R SELECT STEP");
    text(18, 91, context, color(Accent), 1);
    const auto count = static_cast<unsigned>(p.slice.regions);
    const auto active = prepare_slice(p.slice, p.rhythm.bpm, 48000, p.slice_pattern);
    const auto cuts =
        p.slice_pattern.manual ? slice_cuts(p.slice_pattern, count) : slice_cuts({}, count);
    rect(18, 108, 604, 112, color(Panel));
    if (wave) {
        for (unsigned i = 0; i < Waveform::columns; ++i) {
            const auto &peak = wave->peaks()[i];
            const int x = 24 + static_cast<int>(i) * 588 / 256;
            line(x, 164 - static_cast<int>(peak.high * 44), x,
                 164 - static_cast<int>(peak.low * 44), color(Muted));
        }
    }
    for (unsigned i = 0; i <= count; ++i) {
        const float position = p.grain.start + (p.grain.end - p.grain.start) * cuts[i];
        const int x = 24 + static_cast<int>(position * 588);
        line(x, 112, x, 214, color(Accent));
    }
    for (unsigned i = 0; i < 16; ++i) {
        const int x = 18 + static_cast<int>(i % 8) * 76;
        const int y = 232 + static_cast<int>(i / 8) * 44;
        const auto &s = active.pattern.steps[i];
        const bool selected = i == editor.step();
        rect(x, y, 72, 40, color(selected ? Accent : Panel));
        char text_value[12]{};
        if (s.enabled)
            std::snprintf(text_value, sizeof(text_value), "%02u%s",
                          std::min(s.region, count - 1) + 1, s.reverse ? "R" : "");
        else
            std::snprintf(text_value, sizeof(text_value), "REST");
        text(x + 6, y + 10, text_value, color(selected ? Dark : Ink), 2);
        if (playing && i == (sequence & 15U))
            rect(x + 4, y + 35, 64, 3, color(Signal));
    }
    static constexpr const char *fields[] = {"REGION", "PLAY STEP", "REVERSE", "CHANCE %",
                                             "CUT END %"};
    const auto &step = p.slice_pattern.steps[editor.step()];
    const auto region = std::min(step.region, count - 1);
    const auto manual_cuts = slice_cuts(p.slice_pattern, count);
    for (unsigned i = 0; i < 5; ++i) {
        const int y = 336 + static_cast<int>(i) * 18;
        text(30, y, (i == editor.field() ? "> " : "  ") + std::string(fields[i]),
             color(i == editor.field() ? Accent : Muted), 2);
        char value[32]{};
        if (i == 0)
            std::snprintf(value, sizeof(value), "%u / %u", region + 1, count);
        if (i == 1)
            std::snprintf(value, sizeof(value), "%s", step.enabled ? "ON" : "REST");
        if (i == 2)
            std::snprintf(value, sizeof(value), "%s", step.reverse ? "ON" : "OFF");
        if (i == 3)
            std::snprintf(value, sizeof(value), "%.0F",
                          static_cast<double>(step.probability * 100));
        if (i == 4)
            std::snprintf(value, sizeof(value), "%.1F%s",
                          static_cast<double>(manual_cuts[region + 1] * 100),
                          region + 1 == count ? " FIXED" : "");
        text(256, y, value, color(Ink), 2);
    }
    text(442, 344, "L/R STEP", color(Accent), 2);
    text(442, 370, "X REST", color(Muted), 2);
    text(442, 396, "Y MODE", color(Muted), 2);
    footer("D-PAD EDIT   A PLAY   B BACK", "MENU SAVES PATTERN / SOURCE WAV UNCHANGED");
    previous_ = frame;
    cached_pattern_ = p.slice_pattern;
    last_sequence_ = sequence;
    last_animation_ = now;
    invalid_ = false;
    view_ = View::Pattern;
    present();
}
void Display::draw_export(unsigned seconds, unsigned progress, const std::string &message,
                          bool rendering) {
    ++requests_;
    const std::array<std::string, 10> frame{std::to_string(seconds), std::to_string(progress),
                                            message, rendering ? "1" : "0"};
    if (!invalid_ && view_ == View::Export && previous_ == frame)
        return;
    chrome("EXPORT WAV", 3, false);
    text(30, 124, "FULL INSTRUMENT + EFFECTS", color(Ink), 3);
    text(30, 166, "RESTARTS YOUR SOUND / NOT A LIVE RECORDING", color(Muted), 2);
    char value[48]{};
    std::snprintf(value, sizeof(value), "%u SEC / 48 KHZ / STEREO", seconds);
    text(30, 220, value, color(Accent), 3);
    rect(30, 272, 580, 12, color(Track));
    rect(30, 272, static_cast<int>(580 * std::min(progress, 100U) / 100), 12, color(Accent));
    text(30, 316, label(message, 47), color(Ink), 2);
    text(30, 360, "SAVED IN STATE/EXPORTS / PLAYBACK PAUSED", color(Muted), 2);
    footer(rendering ? "B CANCEL   MENU EXIT" : "LEFT/RIGHT DURATION   A EXPORT   B BACK",
           "NEW NUMBERED FILE / EXISTING WAVS ARE NEVER OVERWRITTEN");
    previous_ = frame;
    invalid_ = false;
    view_ = View::Export;
    present();
}
} // namespace grainmm
