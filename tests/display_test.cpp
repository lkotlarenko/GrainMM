// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/display.hpp"
#include "grainmm/parameter_editor.hpp"
#include "grainmm/patch_menu.hpp"
#include "grainmm/sdl_runtime.hpp"
#include "grainmm/waveform.hpp"
#include <SDL.h>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace {
void capture(const grainmm::Display &display, const std::filesystem::path &directory,
             const char *name) {
    if (!directory.empty())
        display.save_snapshot((directory / name).string());
}
void require_at(bool condition, int line) {
    if (!condition)
        throw std::runtime_error("Display cache/state regression at line " + std::to_string(line));
}
} // namespace
#define require(value) require_at((value), __LINE__)
int main(int argc, char **argv) {
    try {
        std::filesystem::path directory;
        if (argc == 2) {
            directory = argv[1];
            if (!std::filesystem::create_directory(directory))
                throw std::runtime_error("Capture directory must not already exist");
        } else if (argc != 1)
            throw std::runtime_error("Usage: display_test [NEW_CAPTURE_DIRECTORY]");
        grainmm::SdlRuntime sdl;
        grainmm::Display display;
        grainmm::ParameterEditor editor;
        display.draw("piano.wav", false, editor, true);
        display.draw("piano.wav", false, editor, true);
        require(display.presentations() == 1);
        capture(display, directory, "grain.bmp");
        if (!directory.empty()) {
            bool refused = false;
            try {
                capture(display, directory, "grain.bmp");
            } catch (const std::runtime_error &) {
                refused = true;
            }
            require(refused);
        }
        editor.select(1);
        display.draw("piano.wav", false, editor, true);
        require(display.presentations() == 2);
        editor.adjust(1);
        display.draw("piano.wav", false, editor, true);
        require(display.presentations() == 3);
        editor.switch_page();
        display.draw("piano.wav", false, editor, true);
        capture(display, directory, "delay.bmp");
        display.draw_browser("Keys", "organ.wav", "piano.wav", "strings.wav",
                             "A: LOAD WAV  X: TEST TONE");
        display.draw_browser("Keys", "organ.wav", "piano.wav", "strings.wav",
                             "A: LOAD WAV  X: TEST TONE");
        require(display.presentations() == 5);
        capture(display, directory, "browser.bmp");
        display.draw("piano.wav", false, editor, true);
        require(display.presentations() == 6);
        display.invalidate();
        display.draw("piano.wav", false, editor, true);
        require(display.presentations() == 7);
        display.draw("", false, editor, false);
        capture(display, directory, "tone.bmp");
        display.draw("", false, editor, false, "SAMPLE ERROR - Y: BROWSE");
        require(display.presentations() == 9);
        grainmm::ParameterEditor synth;
        auto controls = synth.snapshot();
        controls.oscillator.level = 0.08F;
        controls.oscillator.wave = 2;
        synth.restore(controls);
        synth.show_oscillator();
        display.draw("", false, synth, false);
        capture(display, directory, "synth.bmp");
        require(display.presentations() == 10);
        display.draw_help(synth);
        display.draw_help(synth);
        require(display.presentations() == 11);
        display.draw("", false, synth, false);
        require(display.presentations() == 12);
        display.draw_help(synth);
        require(display.presentations() == 13);
        display.draw_browser("", "", "EMPTY", "", "");
        require(display.presentations() == 14);
        grainmm::Sample sample(8000, 1, {-.5F, .5F});
        grainmm::Waveform wave(sample);
        editor.show_sample();
        display.draw_waveform(wave, editor, "input.wav", false);
        display.draw_waveform(wave, editor, "input.wav", false);
        require(display.presentations() == 15);
        editor.adjust(1);
        display.draw_waveform(wave, editor, "input.wav", false);
        require(display.presentations() == 16);
        grainmm::PatchMenu patches({});
        display.draw_patches(patches);
        display.draw_patches(patches);
        require(display.presentations() == 17);
        patches.input(grainmm::PatchKey::Name);
        display.draw_patches(patches);
        require(display.presentations() == 18);
        display.draw_help(editor, false);
        const auto dialog_help = display.presentations();
        display.draw_help(editor, true);
        require(display.presentations() == dialog_help + 1);
        editor.select(1);
        display.draw_help(editor, true);
        require(display.presentations() == dialog_help + 2);
        editor.adjust(-1); // END starts at its upper bound; make a real value change.
        display.draw_help(editor, true);
        require(display.presentations() == dialog_help + 3);
        display.draw_help(editor, true, true);
        const auto confirmation = display.presentations();
        display.draw_help(editor, true, true);
        require(display.presentations() == confirmation);
        display.draw_help(editor, true, false);
        require(display.presentations() == confirmation + 1);
        grainmm::ParameterEditor all_pages;
        for (unsigned page = 0; page < grainmm::ParameterEditor::page_count; ++page) {
            display.draw_help(all_pages);
            all_pages.switch_page();
        }
        grainmm::ParameterEditor sequence;
        sequence.set_page(15);
        require(std::string(sequence.page_label()) == "SLICE");
        display.draw("input.wav", true, sequence, true, "", 1U << 16);
        const auto steps = display.presentations();
        display.draw("input.wav", true, sequence, true, "", 1U << 16);
        require(display.presentations() == steps);
        SDL_Delay(70); // Animation is capped independently of immediate control changes.
        display.draw("input.wav", true, sequence, true, "", (1U << 16) | 1U | (2U << 4));
        require(display.presentations() == steps + 1);
        grainmm::PageNavigator nav;
        nav.open(16);
        display.draw_navigation(sequence, nav);
        const auto navigation = display.presentations();
        display.draw_navigation(sequence, nav);
        require(display.presentations() == navigation);
        nav.group(1);
        display.draw_navigation(sequence, nav);
        require(display.presentations() == navigation + 1);
        const auto cache_layouts = display.layouts();
        sequence.set_page(16);
        auto scene_parameters = sequence.snapshot();
        scene_parameters.collision.enabled = 1;
        scene_parameters.collision.count = 6;
        sequence.restore(scene_parameters);
        grainmm::CollisionSynth collision(48000);
        collision.apply(grainmm::prepare_collision(scene_parameters.collision, 48000));
        std::vector<float> scene_audio(96000);
        collision.add(scene_audio.data(), 48000, 2, true);
        grainmm::AudioVisuals animated;
        animated.balls = collision.positions();
        animated.hits = collision.hits();
        display.draw("", true, sequence, false, "", 0, nullptr, animated);
        for (unsigned tick = 0; tick < 3; ++tick) {
            SDL_Delay(70);
            animated.balls[0] += 1;
            display.draw("", true, sequence, false, "", 0, nullptr, animated);
        }
        require(display.layouts() == cache_layouts + 1);
        capture(display, directory, "balls.bmp");
        display.draw_navigation(sequence, nav);
        capture(display, directory, "modules.bmp");
        std::vector<float> preview_values(4800);
        for (unsigned i = 0; i < preview_values.size(); ++i)
            preview_values[i] = static_cast<float>(std::sin(i * .12) *
                                                   std::exp(-static_cast<double>(i % 800) / 350));
        grainmm::Sample preview(48000, 1, std::move(preview_values));
        grainmm::Waveform preview_wave(preview);
        sequence.set_page(0);
        animated.peaks = 16000U | (24000U << 16);
        animated.grain = 32000U | (6U << 16);
        display.draw("TEXTURE.WAV", true, sequence, true, "", 0, &preview_wave, animated);
        capture(display, directory, "studio.bmp");
        display.set_theme(0);
        display.draw_style(0, "PREVIEW / A SAVE / B CANCEL");
        capture(display, directory, "style.bmp");
        display.draw_drone_scenes(0, 42, 0);
        const auto scenes = display.presentations();
        display.draw_drone_scenes(0, 42, 0);
        require(display.presentations() == scenes);
        for (unsigned theme = 0; theme < 6; ++theme) {
            display.set_theme(theme);
            for (unsigned i = 0; i < 8; ++i)
                display.draw_drone_scenes(i, 9999, 1);
        }
        sequence.set_page(21);
        animated.output_reduction = 32000;
        display.draw("", true, sequence, false, "", 0, nullptr, animated);
        grainmm::ParameterEditor cache_probe;
        cache_probe.set_page(11); // Harmony draws the sample region too.
        display.draw("input.wav", false, cache_probe, true, "", 0, &wave);
        const auto initial_scenes = display.scenes();
        cache_probe.select(1);
        display.draw("input.wav", false, cache_probe, true, "", 0, &wave);
        require(display.scenes() == initial_scenes);
        auto hidden = cache_probe.snapshot();
        hidden.grain.start = .25F;
        cache_probe.restore(hidden);
        display.draw("input.wav", false, cache_probe, true, "", 0, &wave);
        require(display.scenes() == initial_scenes + 1);
        display.invalidate();
        display.draw("input.wav", false, cache_probe, true, "", 0, &wave);
        require(display.scenes() == initial_scenes + 2);
        grainmm::PageNavigator fx_nav;
        fx_nav.open(1);
        display.draw_navigation(cache_probe, fx_nav);
        const auto nav_before = display.presentations();
        auto enabled = cache_probe.snapshot();
        enabled.delay.mix = .5F;
        cache_probe.restore(enabled);
        display.draw_navigation(cache_probe, fx_nav);
        require(display.presentations() == nav_before + 1);
        // Exercise every main view in every palette, at both parameter extremes.
        // Numeric drawing bounds catch clipping without taking more screenshots.
        for (unsigned theme = 0; theme < 6; ++theme) {
            display.set_theme(theme);
            for (float endpoint : {-1e20F, 1e20F}) {
                grainmm::InstrumentParameters extreme;
                for (std::size_t n = 0; n < grainmm::parameter_count; ++n)
                    grainmm::parameter_definition(n).write(extreme) = endpoint;
                extreme = grainmm::bounded(extreme);
                for (unsigned page = 0; page < grainmm::ParameterEditor::page_count; ++page) {
                    cache_probe.restore(extreme);
                    cache_probe.set_page(page);
                    display.draw("A VERY LONG TEST SAMPLE NAME.WAV", false, cache_probe, true, "",
                                 0, &wave);
                }
            }
        }
        grainmm::PatternEditor pattern_editor;
        grainmm::InstrumentParameters musical;
        musical.slice_pattern.manual = true;
        musical.slice_pattern.steps[1].enabled = false;
        musical.slice_pattern.widths[2] = .2F;
        for (unsigned theme = 0; theme < 6; ++theme) {
            display.set_theme(theme);
            grainmm::PageNavigator all_groups;
            for (unsigned group = 0; group < 4; ++group) {
                display.draw_navigation(cache_probe, all_groups);
                if (theme == 0 && (group == 0 || group == 3))
                    capture(display, directory,
                            group == 0 ? "source-modules.bmp" : "tools-modules.bmp");
                for (unsigned i = 0; i < all_groups.count(); ++i) {
                    display.draw_navigation(cache_probe, all_groups);
                    all_groups.move(1);
                }
                all_groups.group(1);
            }
            for (unsigned field = 0; field < 5; ++field) {
                display.draw_pattern(musical, pattern_editor, &wave, 2, true);
                pattern_editor.input(grainmm::PatternKey::Down, musical.slice_pattern, 8);
            }
            display.draw_export(60, 100, "GrainMM-999.wav / CLIPS: LOWER OUTPUT", false);
        }
        display.set_theme(0);
        display.draw_pattern(musical, pattern_editor, &wave, 2, true);
        capture(display, directory, "pattern.bmp");
        display.draw_export(15, 45, "RENDERING / B CANCELS", true);
        capture(display, directory, "export.bmp");
        if (!directory.empty()) {
            grainmm::ParameterEditor showcase;
            auto sound = showcase.snapshot();
            sound.oscillator.level = .1F;
            sound.oscillator_b.level = .08F;
            sound.oscillator_b.note = 64;
            sound.synth_color = {1, 1.5F, .1F, .3F, .03F, 1};
            sound.groove = {.3F, 7, 2, 2, .35F, 3};
            sound.slice.mode = 1;
            sound.drone_synth.level = .6F;
            sound.drone_synth.root = 60;
            sound.drone_synth.mood = 4;
            showcase.restore(sound);
            showcase.set_page(23);
            display.draw("TEXTURE.WAV", false, showcase, true, "", 0, &preview_wave);
            capture(display, directory, "wavetable-color.bmp");
            showcase.set_page(24);
            display.draw("TEXTURE.WAV", false, showcase, true, "", 0, &preview_wave);
            capture(display, directory, "slice-groove.bmp");
            showcase.set_page(19);
            display.draw("", false, showcase, false);
            capture(display, directory, "generative-drone.bmp");
        }
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
