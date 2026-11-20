// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/display.hpp"
#include "grainmm/parameter_editor.hpp"
#include "grainmm/waveform.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>
namespace grainmm {
void Display::scene(const ParameterEditor &editor, const Waveform *wave, bool has_sample,
                    unsigned drone_state) {
    const auto p = editor.snapshot();
    const auto page = editor.page_index();
    rect(18, 108, 604, 138, color(Panel));
    if (page != 16 && page != 17 && page != 19 && page != 20 && page != 21) {
        for (int x = 34; x < 500; x += 58)
            line(x, 130, x, 230, color(Track));
        line(30, 180, 500, 180, color(Track));
    }
    text(530, 116, "L   R", color(Muted), 1);
    if (!has_sample && editor.requires_sample()) {
        text(40, 148, "LOAD A WAV FOR THIS SOURCE", color(Ink), 2);
        text(40, 194, "Y SAMPLES / X MODULES", color(Muted), 2);
        return;
    }
    if (page == 16 || page == 17) {
        for (unsigned i = 0; i < 6; ++i) {
            const double a = (30 + i * 60) * 3.141592654 / 180,
                         b = (30 + (i + 1) * 60) * 3.141592654 / 180;
            line(180 + static_cast<int>(62 * std::cos(a)), 178 + static_cast<int>(62 * std::sin(a)),
                 180 + static_cast<int>(62 * std::cos(b)), 178 + static_cast<int>(62 * std::sin(b)),
                 color(Accent));
        }
        text(286, 133, "COLLISION SYNTH", color(Accent), 2);
        char tuning[32]{};
        std::snprintf(tuning, sizeof(tuning), "NOTE %.0F  OCT %+.0F",
                      static_cast<double>(p.collision.note),
                      static_cast<double>(p.collision.octave));
        text(286, 164, tuning, color(Ink), 1);
        text(286, 188, p.collision.enabled ? "A PLAY / PAUSE" : "MODE ON TO EXCITE", color(Muted),
             1);
    } else if (page == 21) {
        text(34, 130, "MASTER OUTPUT / AFTER THE FX RACK", color(Accent), 2);
        text(34, 169, "ENABLE LIMITER, THEN RAISE GAIN DB", color(Ink), 1);
        text(34, 195, "LIMITER ACTIVITY", color(Muted), 1);
        rect(34, 218, 450, 8, color(Track));
    } else if (page == 19 || page == 20) {
        text(30, 116, "PAD VOICES / TARGET NOTES", color(Muted), 1);
        for (unsigned i = 0; i < 4; ++i) {
            const int x = 30 + static_cast<int>(i) * 116;
            rect(x, 134, 106, 94, color(Dark));
            rect(x, 134, 106, 2, color(i % 2 ? Signal : Accent));
            const unsigned midi = drone_chord_note(p.drone_synth, drone_state & 3U, i);
            constexpr const char *names[] = {"C",  "C#", "D",  "D#", "E",  "F",
                                             "F#", "G",  "G#", "A",  "A#", "B"};
            char note[12]{};
            std::snprintf(note, sizeof(note), "%s%d", names[midi % 12],
                          static_cast<int>(midi / 12) - 1);
            text(x + 12, 143, note, color(Ink), 2);
            circle(x + 53, 190, 20, color(Track));
            line(x + 12, 218, x + 94, 218, color(Track));
        }
    } else if (page == 23) {
        text(30, 116, "OSC A COLOR / GENERATED TABLES", color(Muted), 1);
        constexpr const char *spectra[] = {"PURE", "HOLLOW", "BRIGHT", "GLASS"};
        for (unsigned i = 0; i < 4; ++i) {
            const int x = 30 + static_cast<int>(i) * 116;
            rect(x, 143, 106, 42, color(Dark));
            text(x + 12, 157, spectra[i],
                 color(i <= static_cast<unsigned>(p.synth_color.position) ? Signal : Muted), 2);
        }
        rect(30, 199, 454, 5, color(Track));
        rect(30 + static_cast<int>(p.synth_color.position / 3 * 446), 195, 8, 13, color(Accent));
        char levels[80]{};
        std::snprintf(levels, sizeof(levels), "TABLE %.0F%% / SUB %.0F%% / OSC A %s",
                      static_cast<double>(p.synth_color.table_mix * 100),
                      static_cast<double>(p.synth_color.sub_level * 100),
                      p.oscillator.level > 0 ? "ENABLED" : "LEVEL ZERO");
        text(30, 222, levels, color(Ink), 1);
    } else if (page == 24) {
        text(30, 116, "SLICE PHRASE / STORED STEPS STAY INTACT", color(Muted), 1);
        for (unsigned i = 0; i < 16; ++i) {
            const int x = 30 + static_cast<int>(i) * 29;
            const bool active = i < static_cast<unsigned>(p.groove.length);
            rect(x, 146, 25, 34, color(active ? Accent : Track));
            char number[8]{};
            std::snprintf(number, sizeof(number), "%02u", i + 1);
            text(x + 6, 159, number, color(active ? Dark : Muted), 1);
        }
        char timing[80]{};
        std::snprintf(timing, sizeof(timing), "SWING %.0F%% / ACCENT EVERY %.0F STEPS",
                      static_cast<double>(p.groove.swing * 100),
                      static_cast<double>(p.groove.accent_every));
        text(30, 205, timing, color(Ink), 1);
        text(30, 225,
             p.slice.mode != 0 ? "SLICE ENABLED / A PLAY OR PAUSE"
                               : "SET SLICE MODE ON TO HEAR THE PHRASE",
             color(Muted), 1);
    } else if ((editor.requires_sample() || page == 4) && has_sample) {
        if (wave)
            for (unsigned i = 0; i < Waveform::columns; ++i) {
                const auto peak = wave->peaks()[i];
                const int x = 30 + static_cast<int>(i) * 470 / 256;
                const auto inside = static_cast<float>(i) / 256 >= p.grain.start &&
                                    static_cast<float>(i) / 256 <= p.grain.end;
                line(x, 180 - static_cast<int>(peak.high * 43), x,
                     180 - static_cast<int>(peak.low * 43), color(inside ? Signal : Muted));
            }
        const int start = 30 + static_cast<int>(p.grain.start * 470),
                  end = 30 + static_cast<int>(p.grain.end * 470);
        line(start, 132, start, 225, color(Accent));
        line(end, 132, end, 225, color(Accent));
        if (page == 15) {
            const auto cuts = slice_cuts(p.slice_pattern.manual ? p.slice_pattern : SlicePattern{},
                                         static_cast<unsigned>(p.slice.regions));
            for (unsigned i = 0; i <= static_cast<unsigned>(p.slice.regions); ++i) {
                const int x = start + static_cast<int>(static_cast<float>(end - start) * cuts[i]);
                line(x, 136, x, 219, color(Track));
            }
        }
        text(30, 116,
             page == 15   ? (p.slice_pattern.manual ? "MANUAL PATTERN / B EDIT"
                                                    : "GENERATED PATTERN / B EDIT")
             : page == 18 ? "EVOLVE / BASE MARK AND ACTIVE GRAINS"
                          : "SAMPLE / GRAIN POSITION",
             color(Muted), 1);
        if (page == 18) {
            const auto base = p.grain.start + p.grain.position * (p.grain.end - p.grain.start);
            const int x = 30 + static_cast<int>(base * 470);
            rect(x - 2, 224, 5, 7, color(Ink));
        }
    } else if (page == 2) {
        for (unsigned i = 0; i < 6; ++i) {
            const int x = 46 + static_cast<int>(i) * 74, h = static_cast<int>(p.eq_db[i] * 2);
            rect(x, h > 0 ? 180 - h : 180, 36, std::max(1, std::abs(h)),
                 color(h >= 0 ? Accent : Signal));
        }
        text(30, 116, "SIX BAND EQ / 0 DB", color(Muted), 1);
    } else if (page == 5 || page == 22 || (!has_sample && page == 0)) {
        const auto &oscillator = page == 22 ? p.oscillator_b : p.oscillator;
        int previous = 180;
        for (int x = 0; x < 470; ++x) {
            const double phase = static_cast<double>(x % 156) / 156;
            double value = std::sin(phase * 6.283185307);
            if (oscillator.wave == 1)
                value = 1 - 4 * std::abs(phase - .5);
            if (oscillator.wave == 2)
                value = phase * 2 - 1;
            if (oscillator.wave == 3)
                value = phase < oscillator.pulse_width ? 1 : -1;
            const int y = 180 - static_cast<int>(value * 32);
            if (x)
                line(29 + x, previous, 30 + x, y, color(Signal));
            previous = y;
        }
        text(30, 116, page == 22 ? "OSC B / INDEPENDENT PITCH AND WAVE" : "OSC A / BASE WAVE SHAPE",
             color(Muted), 1);
    } else if (page == 1) {
        for (unsigned i = 0; i < 7; ++i) {
            const int x = 40 + static_cast<int>(i) * 65,
                      h = static_cast<int>(65 * std::pow(p.delay.feedback, static_cast<float>(i)));
            rect(x, 226 - h, 16, h, color(i ? Signal : Accent));
        }
        text(30, 116, "DELAY / FEEDBACK TAPS", color(Muted), 1);
    } else if (page == 3) {
        int prior = 136;
        for (int x = 0; x < 455; ++x) {
            const int y = 226 - static_cast<int>(90 * std::exp(-static_cast<float>(x) /
                                                               (20 + p.reverb.decay_s * 16)));
            if (x)
                line(30 + x - 1, prior, 30 + x, y, color(Signal));
            prior = y;
        }
        text(30, 116, "REVERB / DECAY ENVELOPE", color(Muted), 1);
    } else if (page == 13) {
        int prior = 0;
        for (int x = 0; x < 455; ++x) {
            const float f = 20 * std::pow(800.F, static_cast<float>(x) / 454),
                        r = f / p.filter.cutoff_hz;
            float v = 1 / std::sqrt(1 + r * r * r * r);
            if (p.filter.mode == 1)
                v = 1 - v;
            if (p.filter.mode == 2)
                v = std::min(1.F, 2 * r / (1 + r * r));
            const int y = 224 - static_cast<int>(v * 80);
            if (x)
                line(29 + x, prior, 30 + x, y, color(Signal));
            prior = y;
        }
        text(30, 116, "FILTER / PARAMETER RESPONSE", color(Muted), 1);
    } else {
        text(30, 116, "OUTPUT / STEREO LEVEL", color(Muted), 1);
        text(40, 153, editor.page_label(), color(Accent), 3);
        const auto active = editor.effect_active();
        text(40, 199, active ? "PROCESSING ENABLED" : "DRY / BYPASS",
             color(active ? Signal : Muted), 2);
    }
}
void Display::dynamics(const ParameterEditor &editor, bool playing, unsigned sequence,
                       const AudioVisuals &v) {
    for (unsigned ch = 0; ch < 2; ++ch) {
        const auto raw = (v.peaks >> (ch * 16)) & 65535U;
        const float level =
            raw ? std::clamp((20 * std::log10(static_cast<float>(raw) / 65535) + 48) / 48, 0.F, 1.F)
                : 0;
        meters_[ch] = std::max(level, meters_[ch] - .08F);
        for (unsigned i = 0; i < 18; ++i)
            rect(532 + static_cast<int>(ch) * 40, 225 - static_cast<int>(i) * 5, 22, 3,
                 color(static_cast<float>(i) / 18 < meters_[ch] ? (i > 15 ? Accent : Signal)
                                                                : Track));
    }
    if (editor.page_index() == 21) {
        rect(34, 218, static_cast<int>(std::min(v.output_reduction, 65535U) * 450U / 65535U), 8,
             color(Accent));
    }
    if (editor.page_index() == 19 || editor.page_index() == 20) {
        const auto p = editor.snapshot().drone_synth;
        const float phase = static_cast<float>((v.drone >> 8) & 65535U) / 65535;
        for (unsigned i = 0; i < 4; ++i) {
            const float raw = static_cast<float>((v.drone_voices >> (8 * i)) & 255U) / 255;
            const float event = static_cast<float>((v.drone_events >> (8 * i)) & 255U) / 255;
            drone_levels_[i] = std::max(raw, drone_levels_[i] - .06F);
            event_levels_[i] = std::max(event, event_levels_[i] - .10F);
            const int x = 83 + static_cast<int>(i) * 116;
            const int radius = 5 + static_cast<int>(drone_levels_[i] * 15);
            circle(x, 190, radius, color(drone_levels_[i] > 0 ? (i % 2 ? Signal : Accent) : Track));
            const double angle = (phase + static_cast<float>(i) * .25F) * 6.283185307;
            const int dx = static_cast<int>(std::cos(angle) * 15 * p.movement * p.stereo);
            const int dy = static_cast<int>(std::sin(angle) * 11 * p.movement);
            rect(x + dx - 2, 188 + dy, 5, 5, color(drone_levels_[i] > 0 ? Ink : Muted));
            rect(x - 41, 216, static_cast<int>(82 * drone_levels_[i]), 3,
                 color(i % 2 ? Signal : Accent));
            circle(x, 235, 3 + static_cast<int>(event_levels_[i] * 3),
                   color(event_levels_[i] > 0 ? Signal : Track));
            if (event_levels_[i] > 0)
                rect(x - 1, 234, 3, 3, color(Ink));
        }
        rect(30, 244, 454, 2, color(Track));
        rect(30, 244, static_cast<int>(phase * 454), 2, color(Accent));
    }
    if ((editor.page_index() == 16 || editor.page_index() == 17)) {
        for (auto position : v.balls)
            if (position & (1U << 16)) {
                const int x = 180 + (static_cast<int>(position & 255U) - 127) * 68 / 127,
                          y = 178 + (static_cast<int>((position >> 8) & 255U) - 127) * 68 / 127;
                circle(x, y, 4, color(Signal));
                rect(x - 1, y - 1, 3, 3, color(Ink));
            }
        char hits[32]{};
        std::snprintf(hits, sizeof(hits), "IMPACTS %u", v.hits);
        text(286, 215, hits, color(Signal), 1);
    } else if (editor.page_index() == 15) {
        const auto p = editor.snapshot();
        const int start = 30 + static_cast<int>(p.grain.start * 470);
        const float length = (p.grain.end - p.grain.start) * 470;
        const auto count = static_cast<unsigned>(p.slice.regions);
        const auto region = std::min((sequence >> 4) & 15U, count - 1);
        const auto cuts =
            slice_cuts(p.slice_pattern.manual ? p.slice_pattern : SlicePattern{}, count);
        if (playing && p.slice.mode != 0) {
            rect(start + static_cast<int>(length * cuts[region]), 133,
                 std::max(1, static_cast<int>(length * (cuts[region + 1] - cuts[region]))), 2,
                 color(Accent));
        }
        for (unsigned i = 0; i < 16; ++i)
            rect(30 + static_cast<int>(i) * 29, 233, 25, 5,
                 color(playing && p.slice.mode != 0 && (sequence & 15U) == i ? Accent : Track));
    } else if (editor.page_index() == 24) {
        const auto p = editor.snapshot();
        for (unsigned i = 0; i < 16; ++i)
            rect(30 + static_cast<int>(i) * 29, 183, 25, 3,
                 color(playing && p.slice.mode != 0 && (sequence & 15U) == i ? Signal : Panel));
    } else if (editor.requires_sample() && (v.grain >> 16) != 0) {
        const int x = 30 + static_cast<int>((v.grain & 65535U) * 470 / 65535);
        line(x, 136, x, 224, color(Accent));
    }
}
void Display::draw(const std::string &sample, bool playing, const ParameterEditor &editor,
                   bool has_sample, const std::string &error, unsigned sequence,
                   const Waveform *wave, AudioVisuals visual) {
    ++requests_;
    const auto p = editor.snapshot();
    const bool drone_page = editor.page_index() == 19 || editor.page_index() == 20;
    std::array<float, parameter_count> values{};
    for (std::size_t i = 0; i < values.size(); ++i)
        values[i] = parameter_definition(i).read(p);
    const bool scene_layout = invalid_ || view_ != View::Parameters ||
                              cached_page_ != editor.page_index() || cached_values_ != values ||
                              cached_pattern_ != p.slice_pattern || cached_sample_ != has_sample ||
                              cached_playing_ != playing || cached_wave_ != wave ||
                              base_[0] != sample || base_[3] != error ||
                              (drone_page && (visual.drone & 3U) != (last_visuals_.drone & 3U));
    const bool layout = scene_layout || cached_selection_ != editor.selection();
    const auto now = SDL_GetTicks();
    const bool visual_changed =
        last_peaks_ != visual.peaks || last_sequence_ != sequence ||
        last_visuals_.grain != visual.grain || last_visuals_.hits != visual.hits ||
        last_visuals_.balls != visual.balls || last_visuals_.drone != visual.drone ||
        last_visuals_.drone_voices != visual.drone_voices ||
        last_visuals_.drone_events != visual.drone_events ||
        last_visuals_.output_reduction != visual.output_reduction ||
        (drone_page &&
         (std::any_of(drone_levels_.begin(), drone_levels_.end(), [](float x) { return x > 0; }) ||
          std::any_of(event_levels_.begin(), event_levels_.end(),
                      [](float x) { return x > 0; }))) ||
        meters_[0] > 0 || meters_[1] > 0;
    if (!layout && (!visual_changed || now - last_animation_ < 50))
        return;
    if (layout) {
        if (scene_layout) {
            base_ = {sample, playing ? "PLAY" : "PAUSE", editor.page_label(), error};
            for (unsigned i = 0; i < 6; ++i) {
                char value[24]{};
                std::snprintf(value, sizeof(value), "%.*F", static_cast<int>(editor.decimals_at(i)),
                              static_cast<double>(editor.display_value_at(i)));
                base_[4 + i] = editor.choice_at(i) ? editor.choice_at(i) : value;
            }
            chrome(editor.page_label(), PageNavigator::group_for(editor.page_index()), playing);
            text(208, 64,
                 label(has_sample                ? sample
                       : p.drone_synth.level > 0 ? "GENERATIVE DRONE"
                       : p.collision.enabled     ? "COLLISION SYNTH"
                       : p.oscillator.level > 0 || p.oscillator_b.level > 0 ||
                               p.synth_color.sub_level > 0
                           ? "BUILT-IN SYNTH"
                           : "BUILT-IN SOURCE",
                       24),
                 color(Muted), 2);
            char tempo[20]{};
            std::snprintf(tempo, sizeof(tempo), "%.0F BPM", static_cast<double>(p.rhythm.bpm));
            text(526, 65, tempo, color(Accent), 2);
            if (!error.empty())
                text(18, 91, label(error, 94), color(Accent), 1);
            else
                text(18, 91,
                     has_sample && p.slice.mode != 0 && editor.page_index() == 0
                         ? "SLICE ACTIVE / ONLY GRAIN PITCH AND GAIN APPLY"
                     : editor.page_index() == 18 && p.slice.mode != 0
                         ? "EVOLVE NEEDS GRAINS / SET SLICE MODE OFF"
                         : "X MODULE NAVIGATOR / L AND R CHANGE PAGES",
                     color(Muted), 1);
            scene(editor, wave, has_sample, visual.drone);
            ++scenes_;
        } else
            copy_pixels(backdrop_, surface_);
        for (unsigned i = 0; i < 6; ++i) {
            if (!scene_layout && i != cached_selection_ && i != editor.selection())
                continue;
            const int x = 18 + static_cast<int>(i % 2) * 308,
                      y = 258 + static_cast<int>(i / 2) * 56;
            const bool selected = i == editor.selection();
            rect(x, y, 296, 54, color(selected ? Dark : Panel));
            rect(x, y, 3, 54, color(selected ? Accent : Track));
            text(x + 12, y + 5, editor.label_at(i), color(selected ? Accent : Muted), 2);
            const auto value = label(base_[4 + i], 15);
            text(x + 12, y + 24, value, color(Ink), 3);
            if (selected)
                rect(x, y, 296, 1, color(Accent));
            const auto range = editor.range_at(i);
            const float fraction =
                range.second > range.first
                    ? std::clamp((editor.value_at(i) - range.first) / (range.second - range.first),
                                 0.F, 1.F)
                    : 0;
            rect(x + 12, y + 50, 272, 2, color(Track));
            rect(x + 12, y + 50, static_cast<int>(272 * fraction), 2,
                 color(selected ? Accent : Muted));
        }
        if (scene_layout)
            footer(
                "D-PAD EDIT   A PLAY   X MODULES",
                drone_page ? "L/R PAGES   B SCENES   Y SAMPLES   SELECT HELP   MENU EXIT"
                : editor.page_index() == 15
                    ? "L/R PAGES   B PATTERN   Y SAMPLES   SELECT HELP   MENU EXIT"
                : editor.page_index() == 21
                    ? "L/R PAGES   B EXPORT WAV   SELECT HELP   MENU EXIT"
                    : "L/R PAGES   Y SAMPLES   B TRIM   START PATCHES   SELECT HELP   MENU EXIT");
        copy_pixels(surface_, backdrop_);
        cached_values_ = values;
        cached_pattern_ = p.slice_pattern;
        cached_page_ = editor.page_index();
        cached_sample_ = has_sample;
        cached_playing_ = playing;
        cached_wave_ = wave;
        ++layouts_;
    } else
        copy_pixels(backdrop_, surface_);
    dynamics(editor, playing, sequence, visual);
    present();
    last_animation_ = now;
    last_peaks_ = visual.peaks;
    last_sequence_ = sequence;
    last_visuals_ = visual;
    cached_selection_ = static_cast<unsigned>(editor.selection());
    view_ = View::Parameters;
    invalid_ = false;
}
} // namespace grainmm
