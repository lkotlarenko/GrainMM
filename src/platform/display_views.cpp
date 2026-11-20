// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/display.hpp"
#include "grainmm/drone_scenes.hpp"
#include "grainmm/parameter_editor.hpp"
#include "grainmm/patch_menu.hpp"
#include "grainmm/waveform.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
namespace grainmm {
void Display::draw_navigation(const ParameterEditor &editor, const PageNavigator &nav) {
    ++requests_;
    const auto p = editor.snapshot();
    const auto status = [&](unsigned page) {
        if (page == PageNavigator::export_page)
            return "FULL MIX TO WAV";
        if (page == PageNavigator::patches_page)
            return "SAVE / LOAD SOUND";
        if (page == PageNavigator::pattern_page)
            return "EDIT 16 STEPS / CUTS";
        if (page == 21)
            return "GAIN / LIMIT / STEREO";
        if (page == PageNavigator::style_page)
            return "UI PREFERENCE";
        ParameterEditor item = editor;
        item.set_page(page);
        if (nav.section() == 1)
            return item.effect_active() ? "ACTIVE" : "BYPASS";
        if (page == 5 || page == 23)
            return p.oscillator.level > 0 || p.synth_color.sub_level > 0 ? "ENABLED" : "LEVEL ZERO";
        if (page == 22)
            return p.oscillator_b.level > 0 ? "ENABLED" : "LEVEL ZERO";
        if (page == 16 || page == 17)
            return p.collision.enabled > 0 ? "ENABLED" : "OFF";
        if (page == 19 || page == 20)
            return p.drone_synth.level > 0 ? "ENABLED" : "LEVEL ZERO";
        return item.requires_sample() ? "WAV SOURCE" : "OPEN CONTROLS";
    };
    std::string states;
    for (unsigned i = 0; i < nav.count(); ++i)
        states += status(nav.page_at(i));
    const std::array<std::string, 10> frame{std::to_string(nav.section()),
                                            std::to_string(nav.selection()), states};
    if (!invalid_ && view_ == View::Navigation && previous_ == frame)
        return;
    chrome("MODULES", nav.section());
    text(220, 64,
         nav.section() == 3 ? "FILES / MASTER / APPEARANCE" : "SELECT A SOUND OR PROCESSOR",
         color(Muted), 1);
    if (nav.count() > PageNavigator::bank_size) {
        char bank[40]{};
        std::snprintf(bank, sizeof(bank), "BANK %u/%u", nav.bank_start() / 12 + 1,
                      (nav.count() + 11) / 12);
        text(18, 94, bank, color(Signal), 1);
    }
    const unsigned columns = nav.section() == 3 ? 2U : nav.count() > 9 ? 4U : 3U;
    const int stride = columns == 4 ? 154 : columns == 2 ? 308 : 206;
    for (unsigned slot = 0; slot < nav.visible_count(); ++slot) {
        const auto i = nav.bank_start() + slot;
        ParameterEditor item = editor;
        const auto page = nav.page_at(i);
        if (page < ParameterEditor::page_count)
            item.set_page(page);
        const int x = 18 + static_cast<int>(slot % columns) * stride,
                  y = 114 + static_cast<int>(slot / columns) * 96;
        const bool selected = i == nav.selection();
        rect(x, y, stride - 14, 82, color(selected ? Accent : Panel));
        char number[12]{};
        std::snprintf(number, sizeof(number), "%02u", i + 1);
        text(x + 12, y + 10, number, color(selected ? Dark : Muted), 1);
        text(x + 12, y + 32,
             page == PageNavigator::style_page     ? "COLORS"
             : page == PageNavigator::export_page  ? "EXPORT WAV"
             : page == PageNavigator::patches_page ? "PATCHES"
             : page == PageNavigator::pattern_page ? "PATTERN"
             : page == 21                          ? "MASTER OUTPUT"
                                                   : item.page_label(),
             color(selected ? Dark : Ink), 2);
        text(x + 12, y + 61, status(nav.page_at(i)), color(selected ? Dark : Muted), 1);
    }
    if (nav.section() == 3) {
        text(30, 334, "EXPORT RESTARTS THE SOUND / PLAYBACK PAUSES", color(Muted), 2);
        text(30, 370, "PATCHES KEEP SETTINGS / WAVS STAY UNCHANGED", color(Muted), 2);
    }
    footer("UP/DOWN CHOOSE   A OPEN   B BACK",
           "L/R SOURCE / FX / TEMPO / TOOLS   X CLOSE   MENU SAVE AND EXIT");
    present();
    previous_ = frame;
    view_ = View::Navigation;
    invalid_ = false;
}
void Display::draw_drone_scenes(unsigned selected, unsigned seed, float level) {
    if (selected >= drone_scenes().size() || seed < 1 || seed > 9999 || !std::isfinite(level) ||
        level < 0 || level > 1)
        throw std::invalid_argument("Invalid drone scene display state");
    ++requests_;
    const std::array<std::string, 10> frame{std::to_string(selected), std::to_string(seed),
                                            std::to_string(level)};
    if (!invalid_ && view_ == View::DroneScenes && previous_ == frame)
        return;
    chrome("DRONE SCENES", 0);
    for (unsigned i = 0; i < drone_scenes().size(); ++i) {
        const int x = 18 + static_cast<int>(i % 2) * 308, y = 116 + static_cast<int>(i / 2) * 56;
        rect(x, y, 296, 46, color(Panel));
        if (i == selected)
            rect(x, y, 3, 46, color(Accent));
        text(x + 14, y + 15, drone_scenes()[i].name, color(i == selected ? Accent : Ink), 2);
    }
    text(22, 354, drone_scenes()[selected].description, color(Signal), 1);
    char detail[96]{};
    std::snprintf(detail, sizeof(detail), "SEED %u / LEVEL %.0F%% / NO SAMPLE REQUIRED", seed,
                  static_cast<double>(level * 100));
    text(22, 378, detail, color(Ink), 1);
    text(22, 400,
         level > 0 ? "KEEPS LEVEL, FX, OTHER SOURCES AND PLAYBACK"
                   : "LEVEL IS ZERO / APPLY THEN RAISE DRONE LEVEL",
         color(Muted), 1);
    footer("UP/DN SCENE  LEFT/RIGHT SEED  A USE  B BACK",
           "BROWSING DOES NOT CHANGE YOUR SOUND   X CANCEL   MENU EXIT");
    present();
    previous_ = frame;
    view_ = View::DroneScenes;
    invalid_ = false;
}
void Display::draw_style(unsigned selected, const std::string &message) {
    ++requests_;
    const std::array<std::string, 10> frame{std::to_string(selected), message};
    if (!invalid_ && view_ == View::Style && previous_ == frame)
        return;
    chrome("COLOR PROFILE", 3);
    for (unsigned i = 0; i < themes.size(); ++i) {
        const int x = 18 + static_cast<int>(i % 2) * 308, y = 120 + static_cast<int>(i / 2) * 92;
        rect(x, y, 296, 78, color(Panel));
        const auto &t = themes[i];
        const auto c = SDL_MapRGB(surface_->format, t.accent[0], t.accent[1], t.accent[2]);
        rect(x + 12, y + 14, 28, 48, c);
        text(x + 54, y + 30, t.name, color(i == selected ? Accent : Ink), 2);
        if (i == selected)
            rect(x, y, 3, 78, color(Accent));
    }
    text(18, 407, label(message, 94), color(Muted), 1);
    footer("UP/DOWN PREVIEW   A SAVE   B CANCEL",
           "L/R LEAVE   X CANCEL   COLORS DO NOT ALTER SOUND");
    present();
    previous_ = frame;
    view_ = View::Style;
    invalid_ = false;
}
void Display::draw_browser(const std::string &path, const std::string &previous,
                           const std::string &selected, const std::string &next,
                           const std::string &message) {
    ++requests_;
    const std::array<std::string, 10> frame{path, previous, selected, next, message};
    if (!invalid_ && view_ == View::Browser && previous_ == frame)
        return;
    chrome("SAMPLES", 0);
    text(208, 65, label(path.empty() ? "USER LIBRARY" : path, 32), color(Muted), 2);
    const std::string entries[] = {previous, selected, next};
    for (unsigned i = 0; i < 3; ++i) {
        const int y = 122 + static_cast<int>(i) * 78;
        rect(18, y, 604, 64, color(i == 1 ? Accent : Panel));
        text(34, y + 23, label(entries[i], 46), color(i == 1 ? Dark : Muted), 2);
    }
    text(18, 382, label(message, 94), color(Signal), 1);
    footer("UP/DOWN CHOOSE   A LOAD   B BACK", "Y CLOSE   X BUILT-IN SOURCE   MENU EXIT");
    credits(0, 0);
    present();
    previous_ = frame;
    view_ = View::Browser;
    invalid_ = false;
}
void Display::draw_patches(const PatchMenu &menu) {
    ++requests_;
    const auto selected = menu.selection();
    const auto entry = [&](unsigned i) {
        const auto &p = menu.entries()[i];
        return std::to_string(i + 1) + "  " + (p.occupied ? p.name : "EMPTY");
    };
    const std::array<std::string, 10> frame{std::to_string(selected),
                                            menu.name(),
                                            menu.message(),
                                            menu.naming() ? menu.edit_text() : "",
                                            std::to_string(menu.cursor()),
                                            selected ? entry(selected - 1) : "",
                                            entry(selected),
                                            selected + 1 < PatchStore::slots ? entry(selected + 1)
                                                                             : "",
                                            menu.naming() ? "NAME" : "SLOTS"};
    if (!invalid_ && view_ == View::Patches && previous_ == frame)
        return;
    chrome(menu.naming() ? "NAME PATCH" : "PATCHES", 0);
    if (menu.naming()) {
        text(18, 119, "NAME FOR NEXT SAVE", color(Muted), 2);
        rect(18, 165, 604, 76, color(Panel));
        text(34, 192, menu.edit_text(), color(Ink), 2);
        rect(34 + static_cast<int>(menu.cursor()) * 12, 217, 10, 3, color(Accent));
        text(18, 284, "D-PAD: LETTER AND CURSOR", color(Muted), 2);
        text(18, 326, "ACCEPT THE NAME, THEN X TO SAVE", color(Signal), 1);
    } else {
        text(208, 65, label(menu.name(), 32), color(Muted), 2);
        for (unsigned i = 0; i < 3; ++i) {
            const int y = 122 + static_cast<int>(i) * 78;
            rect(18, y, 604, 64, color(i == 1 ? Accent : Panel));
            text(34, y + 23, label(frame[5 + i], 46), color(i == 1 ? Dark : Muted), 2);
        }
    }
    text(18, 386, label(menu.message(), 94), color(Signal), 1);
    footer(menu.naming() ? "A ACCEPT NAME   B CANCEL" : "A LOAD   X SAVE   Y NAME   B CLOSE",
           "START CLOSE   MENU SAVE SETTINGS AND EXIT");
    credits(0, 0);
    present();
    previous_ = frame;
    view_ = View::Patches;
    invalid_ = false;
}
void Display::draw_help(const ParameterEditor &editor, bool editing, bool reset, ResetScope scope) {
    ++requests_;
    const std::array<std::string, 10> frame{editor.page_label(),
                                            editor.label(),
                                            std::to_string(editor.value()),
                                            editing ? "EDIT" : "DIALOG",
                                            reset ? "RESET" : "HELP",
                                            std::to_string(static_cast<unsigned>(scope))};
    if (!invalid_ && view_ == View::Help && previous_ == frame)
        return;
    chrome(reset ? "RESET SOUND" : "CONTROL INFO", PageNavigator::group_for(editor.page_index()));
    if (reset) {
        constexpr const char *names[] = {"EFFECTS ONLY", "EFFECTS + MODULATION",
                                         "ALL SOUND CONTROLS"};
        constexpr const char *notes[] = {"DRY RACK / KEEP SOURCE AND TEMPO",
                                         "CLEAR MOVEMENT / KEEP SOURCE AND TEMPO",
                                         "FACTORY DEFAULTS / RETURNS PAUSED"};
        for (unsigned i = 0; i < 3; ++i) {
            const int y = 122 + static_cast<int>(i) * 90;
            const bool selected = i == static_cast<unsigned>(scope);
            rect(18, y, 604, 76, color(selected ? Accent : Panel));
            text(34, y + 15, names[i], color(selected ? Dark : Ink), 2);
            text(34, y + 48, notes[i], color(selected ? Dark : Muted), 1);
        }
        footer("UP/DOWN CHOOSE   X APPLY   B CANCEL", "A PLAY / PAUSE   SELECT CANCEL   MENU EXIT");
    } else {
        rect(18, 114, 604, 112, color(Panel));
        text(34, 130, editor.page_label(), color(Muted), 1);
        text(34, 156, editor.label(), color(Accent), 2);
        char value[32]{};
        std::snprintf(value, sizeof(value), "%.*F",
                      static_cast<int>(editor.decimals_at(editor.selection())),
                      static_cast<double>(editor.display_value_at(editor.selection())));
        text(34, 191,
             editor.choice_at(editor.selection()) ? editor.choice_at(editor.selection()) : value,
             color(Ink), 2);
        constexpr const char *tips[] = {
            "LOAD A WAV / SIZE AND DENSITY SHAPE THE GRAINS",
            "MIX ADDS ECHOES / FEEDBACK EXTENDS THE TAIL",
            "BOOSTS CAN CLIP / LOWER GAIN BEFORE LARGE CHANGES",
            "MIX ADDS SPACE / DECAY SETS THE TAIL",
            "RANDOMNESS AND SCAN MOVE INDIVIDUAL GRAINS",
            "LEVEL ZERO DISABLES OSC / OTHER SOURCES MAY PLAY",
            "DYNAMICS PROCESSOR / NOT A BRICK-WALL LIMITER",
            "DRIVE COLORS THE SOUND / OUTPUT TRIMS THE WET SIGNAL",
            "TUNED RESONANCE / MIDI NOTE SETS THE PITCH",
            "SHARED BPM / TREMOLO AND STEREO PULSE",
            "NON-DESTRUCTIVE TRIM / ENVELOPE SHAPES NEW GRAINS",
            "GRAIN NOTE PATTERNS / REQUIRES A WAV",
            "BITS AND HOLD ADD DIGITAL GRIT",
            "MIX ENABLES THE RESONANT FILTER",
            "SHORT MODULATED DELAYS ADD STEREO MOVEMENT",
            "B OPENS THE STEP/CUT EDITOR / BPM ON TEMPO PAGE",
            "HEXAGON WALL IMPACTS EXCITE THE BOWL SOURCE",
            "TUNING AND TIMBRE OF THE COLLISION INSTRUMENT",
            "SLOW GRAIN MOVEMENT / HOLD FREEZES SCAN, NOT SOUND",
            "SELF-PLAYING PAD / LEVEL % / R FOR MORE GENERATION OPTIONS",
            "SPARKLES / RATE / GLIDE / STEREO / SEED / HARMONY PATTERN",
            "ENABLE LIMITER BEFORE GAIN / B EXPORTS FULL MIX WAV",
            "SECOND OSCILLATOR / TUNE A FIFTH OR OCTAVE ABOVE OSC",
            "OSC A TABLE COLOR / SUB FOLLOWS OSC A NOTE",
            "WAV SLICES ONLY / SWING, PHRASE LENGTH AND ACCENTS"};
        static_assert(std::size(tips) == ParameterEditor::page_count, "page help missing");
        text(18, 252, tips[editor.page_index()], color(Signal), 1);
        text(18, 292,
             editing ? "D-PAD EDIT   L/R PAGE   X RESET CONTROL" : "CLOSE THE DIALOG TO EDIT SOUND",
             color(Ink), 2);
        text(18, 338, "HOLD START + SELECT FOR RESET OPTIONS", color(Muted), 2);
        text(18, 372, "MAIN VIEW: HOLD SELECT THEN A CAPTURE / B RECALL", color(Muted), 1);
        footer("A PLAY   B OR SELECT CLOSE", "MENU SAVE AND EXIT");
    }
    credits(0, 0);
    present();
    previous_ = frame;
    view_ = View::Help;
    invalid_ = false;
}
void Display::draw_waveform(const Waveform &wave, const ParameterEditor &editor,
                            const std::string &name, bool playing) {
    ++requests_;
    std::array<std::string, 10> frame{name, playing ? "PLAY" : "PAUSE",
                                      std::to_string(editor.selection()),
                                      std::to_string(wave.seconds())};
    for (unsigned i = 0; i < 6; ++i)
        frame[4 + i] = std::to_string(editor.value_at(i));
    if (!invalid_ && view_ == View::Waveform && previous_ == frame)
        return;
    chrome("SAMPLE TRIM", 0, playing);
    text(250, 65, label(name, 30), color(Muted), 2);
    rect(18, 114, 604, 194, color(Panel));
    const auto p = editor.snapshot().grain;
    for (unsigned i = 0; i < Waveform::columns; ++i) {
        const auto peak = wave.peaks()[i];
        const float at = static_cast<float>(i) / 256;
        line(32 + static_cast<int>(i) * 2, 210 - static_cast<int>(peak.high * 78),
             32 + static_cast<int>(i) * 2, 210 - static_cast<int>(peak.low * 78),
             color(at >= p.start && at < p.end ? Signal : Muted));
    }
    for (auto boundary : {p.start, p.end})
        line(32 + static_cast<int>(boundary * 512), 128, 32 + static_cast<int>(boundary * 512), 294,
             color(Accent));
    text(18, 338, editor.label(), color(Accent), 2);
    char value[32]{};
    std::snprintf(value, sizeof(value), "%.*F",
                  static_cast<int>(editor.decimals_at(editor.selection())),
                  static_cast<double>(editor.display_value_at(editor.selection())));
    text(460, 335, value, color(Ink), 3);
    text(18, 387, "YOUR WAV STAYS INTACT", color(Muted), 1);
    footer("D-PAD TRIM   A PLAY   B BACK", "SELECT HELP   MENU SAVE AND EXIT");
    credits(0, 0);
    present();
    previous_ = frame;
    view_ = View::Waveform;
    invalid_ = false;
}
} // namespace grainmm
