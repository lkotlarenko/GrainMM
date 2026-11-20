// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/app_options.hpp"
#include "grainmm/composition_screen.hpp"
#include "grainmm/directional_repeat.hpp"
#include "grainmm/display.hpp"
#include "grainmm/drone_scenes.hpp"
#include "grainmm/lifecycle_probe.hpp"
#include "grainmm/page_navigator.hpp"
#include "grainmm/parameter_editor.hpp"
#include "grainmm/patch_menu.hpp"
#include "grainmm/preset.hpp"
#include "grainmm/sample_library.hpp"
#include "grainmm/sdl_audio.hpp"
#include "grainmm/sdl_runtime.hpp"
#include "grainmm/sound_snapshot.hpp"
#include "grainmm/ui_shortcuts.hpp"
#include "grainmm/wave.hpp"
#include "grainmm/waveform.hpp"
#include <SDL.h>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
volatile std::sig_atomic_t interrupted = 0;
void signal_handler(int) { interrupted = 1; }
bool quit_key(int key) { return key == SDLK_ESCAPE; }
bool tone_key(int key) { return key == SDLK_SPACE; }
} // namespace
int grainmm::run_application(const AppOptions &options) {
    auto sample_name = options.sample_name;
    const auto &sample_directory = options.sample_directory;
    const auto &state_directory = options.state_directory;
    const auto self_test = options.self_test, browser_test = options.browser_test,
               synth_test = options.synth_test, rack_test = options.rack_test,
               resume_sample = options.resume_sample;
    const auto library_test = options.library_test, failure_test = options.patch_failure_test;
    const auto seconds = options.seconds;
    const auto buffer = options.buffer;
    grainmm::Preset initial;
    bool restored = false;
    if (!state_directory.empty()) {
        if (const auto saved = grainmm::load_preset(state_directory)) {
            initial = *saved;
            restored = true;
        }
    }
    if (resume_sample) {
        if (state_directory.empty())
            throw std::runtime_error("--resume-sample requires a state directory");
        if (initial.source == grainmm::SampleSource::File)
            sample_name = initial.sample;
        else if (initial.source == grainmm::SampleSource::Tone)
            sample_name.clear();
    }
    if (synth_test && !sample_name.empty())
        throw std::runtime_error("synth self-test requires a built-in/no-WAV source");
    grainmm::SampleLibrary library(sample_directory, false);
    std::unique_ptr<grainmm::Sample> sample;
    std::string browser_message, startup_error;
    if (!sample_name.empty()) {
        try {
            sample = std::make_unique<grainmm::Sample>(library.load(sample_name));
        } catch (const std::exception &error) {
            if (!resume_sample)
                throw;
            startup_error = "SAMPLE ERROR - Y: BROWSE";
            std::fprintf(stderr, "Sample restore failed: %s\n", error.what());
        }
    }
    if ((library_test || failure_test || options.harmony_test || options.texture_test ||
         options.fx_reset_test || options.slice_test || options.pattern_test ||
         options.tools_test || options.performance_test || options.scene_test ||
         options.snapshot_test || options.evolve_test) &&
        !sample)
        throw std::runtime_error("library self-test requires a WAV");
    bool pending_sample_restore = !sample && !sample_name.empty();
    auto selected_source = pending_sample_restore ? initial.source
                           : sample               ? grainmm::SampleSource::File
                                                  : grainmm::SampleSource::Tone;
    std::string selected_sample = pending_sample_restore ? initial.sample
                                  : sample               ? sample_name
                                                         : std::string{};
    std::optional<Waveform> waveform;
    if (sample)
        waveform.emplace(*sample);
    std::unique_ptr<PatchStore> patch_store;
    std::optional<PatchMenu> patch_menu;
    grainmm::SdlRuntime sdl;
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    grainmm::Display display;
    unsigned theme = 0, theme_selected = 0, theme_before = 0;
    bool styling = false;
    std::string theme_message;
    if (!state_directory.empty()) {
        try {
            theme = load_theme(state_directory.c_str());
        } catch (const std::exception &e) {
            std::fprintf(stderr, "Theme restore: %s\n", e.what());
        }
    }
    display.set_theme(theme);
    grainmm::SdlAudio audio(buffer, sample.get(), self_test);
    grainmm::ParameterEditor editor;
    CompositionScreen composition(display, editor, audio);
    bool pending_parameters = restored, parameters_changed = false, source_changed = false;
    editor.restore(initial.parameters);
    if (!sample)
        editor.show_oscillator();
    if (options.pattern_test)
        editor.set_page(15);
    if (options.tools_test || options.performance_test)
        editor.set_page(0);
    if (restored)
        std::puts("preset restored");
    audio.start();
    const auto start = SDL_GetTicks();
    bool running = true, tone_held = false, page_held = false, browser_held = false,
         browsing = false, helping = false, help_held = false, previous_page_held = false,
         next_page_held = false, patch_held = false, back_held = false, patching = false,
         waving = false;
    PageNavigator navigator;
    bool navigating = false, choosing_drone = false;
    unsigned drone_scene = 0, drone_seed = 42, drone_scene_applies = 0, drone_scene_cancels = 0;
    SoundSnapshot sound_snapshot;
    std::uint64_t source_generation = 0;
    unsigned snapshot_captures = 0, snapshot_recalls = 0, snapshot_rejections = 0;
    std::string snapshot_message;
    unsigned snapshot_message_at = 0;
    UiShortcuts shortcuts;
    DirectionalRepeat directions;
    ResetScope reset_scope = ResetScope::Modulation;
    bool reset_confirmation = false, requested_effects_reset = false;
    unsigned effects_resets = 0;
    unsigned source_changes = 0, patch_saves = 0, patch_loads = 0, waveform_views = 0;
    const auto activate_source = [&](std::unique_ptr<Sample> next, Preset requested) {
        const auto begin = std::chrono::steady_clock::now();
        std::optional<Waveform> preview;
        if (next)
            preview.emplace(*next);
        audio.replace_sample(next.get(), requested.parameters);
        editor.restore(requested.parameters);
        sample = std::move(next); // Mixing owner dropped its old reference before this destroys it.
        waveform = std::move(preview);
        if (!sample)
            editor.show_oscillator();
        selected_source = requested.source;
        selected_sample = std::move(requested.sample);
        pending_sample_restore = false;
        source_changed = true;
        ++source_generation;
        pending_parameters = false;
        browsing = false;
        patching = false;
        waving = false;
        browser_message.clear();
        startup_error.clear();
        ++source_changes;
        audio.start();
        display.invalidate();
        std::printf("source_prepare_us=%lld\n",
                    static_cast<long long>(std::chrono::duration_cast<std::chrono::microseconds>(
                                               std::chrono::steady_clock::now() - begin)
                                               .count()));
    };
    const auto adopt_sample = [&](std::unique_ptr<Sample> next, std::string name) {
        auto values = editor.snapshot();
        if (!next)
            values.oscillator.level = 0;
        Preset state{values, next ? SampleSource::File : SampleSource::Tone, std::move(name)};
        try {
            activate_source(std::move(next), std::move(state));
        } catch (const std::exception &error) {
            browser_message = error.what();
            std::fprintf(stderr, "Sample switch failed: %s\n", error.what());
        }
    };
    const auto patch_action = [&](PatchAction action) {
        if (action == PatchAction::Close) {
            patching = false;
            audio.start();
            display.invalidate();
            return;
        }
        if (action == PatchAction::None)
            return;
        try {
            if (action == PatchAction::Save) {
                patch_store->save(
                    patch_menu->selection(),
                    {patch_menu->name(), {editor.snapshot(), selected_source, selected_sample}},
                    patch_menu->replacing());
                patch_menu->refresh(patch_store->list());
                patch_menu->message("PATCH SAVED");
                ++patch_saves;
                std::puts("patch saved");
            } else {
                auto patch = patch_store->load(patch_menu->selection());
                if (!patch)
                    throw std::runtime_error("patch slot is empty");
                std::unique_ptr<Sample> next;
                if (patch->preset.source != SampleSource::Tone) {
                    if (patch->preset.source == SampleSource::Legacy) {
                        patch->preset.source = SampleSource::File;
                        patch->preset.sample = "sample.wav";
                    }
                    next = std::make_unique<Sample>(library.load(patch->preset.sample));
                }
                activate_source(std::move(next), std::move(patch->preset));
                ++patch_loads;
                std::puts("patch loaded");
            }
        } catch (const std::exception &error) {
            patch_menu->message(error.what());
            std::fprintf(stderr, "Patch operation failed: %s\n", error.what());
        }
    };
    const auto open_patch_menu = [&] {
        if (state_directory.empty()) {
            startup_error = "SET A STATE DIRECTORY FOR PATCHES";
            return;
        }
        audio.fade_and_pause();
        browsing = waving = false;
        try {
            if (!patch_store)
                patch_store = std::make_unique<PatchStore>(state_directory);
            patch_menu.emplace(patch_store->list());
            patching = true;
        } catch (const std::exception &error) {
            startup_error = error.what();
            audio.start();
        }
    };
    grainmm::LifecycleProbe probe(
        options.performance_test   ? grainmm::ProbeMode::Performance
        : options.tools_test       ? grainmm::ProbeMode::Tools
        : options.pattern_test     ? grainmm::ProbeMode::Pattern
        : options.drone_test       ? grainmm::ProbeMode::Drone
        : options.evolve_test      ? grainmm::ProbeMode::Evolve
        : options.snapshot_test    ? grainmm::ProbeMode::Snapshot
        : options.ambient_test     ? grainmm::ProbeMode::Ambient
        : options.scene_test       ? grainmm::ProbeMode::Scene
        : options.slice_test       ? grainmm::ProbeMode::Slice
        : options.sound_reset_test ? grainmm::ProbeMode::SoundReset
        : options.repeat_test      ? grainmm::ProbeMode::Repeat
        : options.shortcut_test    ? grainmm::ProbeMode::Shortcut
        : options.fx_reset_test    ? grainmm::ProbeMode::FxReset
        : options.texture_test     ? grainmm::ProbeMode::Texture
        : options.harmony_test     ? grainmm::ProbeMode::Harmony
        : failure_test             ? grainmm::ProbeMode::PatchFailure
        : library_test             ? grainmm::ProbeMode::Library
        : rack_test    ? (sample ? grainmm::ProbeMode::Sample : grainmm::ProbeMode::Rack)
        : !self_test   ? grainmm::ProbeMode::None
        : synth_test   ? grainmm::ProbeMode::Synth
        : browser_test ? grainmm::ProbeMode::Browser
                       : grainmm::ProbeMode::Basic,
        editor.page_index());
    unsigned toggles = 0, observed_keys = 0, help_views = 0;
    const auto change_page = [&](int direction) {
        const auto next = PageNavigator::adjacent(editor.page_index(), direction);
        if (next == PageNavigator::style_page) {
            styling = true;
            theme_before = theme;
            theme_selected = theme;
            theme_message = "PREVIEW / A SAVE / L AND R LEAVE";
        } else
            editor.set_page(next);
    };
    const auto process = [&](const SDL_Event &input, unsigned multiplier = 1,
                             bool repeated = false) {
        SDL_Event event = input;
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            const auto key = event.key.keysym.sym;
            const auto direction = key == SDLK_UP      ? DirectionalRepeat::Direction::Up
                                   : key == SDLK_DOWN  ? DirectionalRepeat::Direction::Down
                                   : key == SDLK_LEFT  ? DirectionalRepeat::Direction::Left
                                   : key == SDLK_RIGHT ? DirectionalRepeat::Direction::Right
                                                       : DirectionalRepeat::Direction::None;
            if (!repeated && direction != DirectionalRepeat::Direction::None) {
                if (event.type == SDL_KEYDOWN && !directions.press(direction, SDL_GetTicks()))
                    return;
                if (event.type == SDL_KEYUP)
                    directions.release(direction);
            }
            if (!repeated && event.type == SDL_KEYDOWN &&
                direction == DirectionalRepeat::Direction::None && key != SDLK_SPACE)
                directions.cancel();
            if (!repeated && observed_keys++ < 64)
                std::printf("input key=%d down=%u t=%u\n", static_cast<int>(event.key.keysym.sym),
                            event.type == SDL_KEYDOWN ? 1U : 0U, SDL_GetTicks());
            const bool snapshot_key = key == SDLK_SPACE || key == SDLK_LCTRL;
            if (!repeated && event.type == SDL_KEYDOWN && snapshot_key && !tone_held &&
                !back_held && !browsing && !patching && !helping && !waving && !navigating &&
                !styling && !choosing_drone && !composition.active() &&
                shortcuts.consume_select_modifier()) {
                directions.cancel();
                if (key == SDLK_SPACE) {
                    tone_held = true;
                    sound_snapshot.capture(editor.snapshot(), source_generation);
                    ++snapshot_captures;
                    snapshot_message = "SNAPSHOT CAPTURED / SELECT + B TO RECALL";
                } else {
                    back_held = true;
                    auto values = editor.snapshot();
                    const auto result = sound_snapshot.recall(source_generation, values);
                    if (result == SoundSnapshot::Result::Restored) {
                        editor.restore(values);
                        pending_parameters = parameters_changed = true;
                        ++snapshot_recalls;
                        snapshot_message = "SNAPSHOT RECALLED / PLAYBACK UNCHANGED";
                    } else {
                        ++snapshot_rejections;
                        snapshot_message = result == SoundSnapshot::Result::Empty
                                               ? "NO SNAPSHOT / HOLD SELECT THEN TAP A"
                                               : "SOURCE CHANGED / CAPTURE A NEW SNAPSHOT";
                    }
                }
                snapshot_message_at = SDL_GetTicks();
                display.invalidate();
                std::printf("snapshot captures=%u recalls=%u playing=%u\n", snapshot_captures,
                            snapshot_recalls, audio.playing() ? 1U : 0U);
                return;
            }
            if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_RCTRL) {
                const bool start_key = event.key.keysym.sym == SDLK_RETURN;
                const auto tap = shortcuts.key(start_key ? UiShortcuts::Button::Start
                                                         : UiShortcuts::Button::Select,
                                               event.type == SDL_KEYDOWN, SDL_GetTicks());
                if (event.type == SDL_KEYUP) {
                    if (start_key)
                        patch_held = false;
                    else
                        help_held = false;
                }
                if (tap == UiShortcuts::Tap::None)
                    return;
                event.type = SDL_KEYDOWN; // Existing tap actions dispatch on release.
            }
        }
        if (event.type == SDL_QUIT)
            running = false;
        if (event.type == SDL_KEYDOWN) {
            if (quit_key(event.key.keysym.sym)) {
                running = false;
                return;
            }
            const int key = event.key.keysym.sym;
            if (composition.active()) {
                bool *held = key == SDLK_SPACE    ? &tone_held
                             : key == SDLK_LCTRL  ? &back_held
                             : key == SDLK_LSHIFT ? &page_held
                             : key == SDLK_LALT   ? &browser_held
                             : key == SDLK_e      ? &previous_page_held
                             : key == SDLK_t      ? &next_page_held
                                                  : nullptr;
                if (held && *held)
                    return;
                if (held)
                    *held = true;
                const bool before = audio.playing();
                if (composition.input(key, multiplier))
                    pending_parameters = parameters_changed = true;
                if (audio.playing() != before)
                    ++toggles;
                return;
            }
            if (choosing_drone) {
                if (key == SDLK_UP || key == SDLK_DOWN) {
                    drone_scene = (drone_scene + (key == SDLK_UP ? 7U : 1U)) % 8;
                    drone_seed = static_cast<unsigned>(drone_scenes()[drone_scene].sound.seed);
                }
                if (key == SDLK_LEFT || key == SDLK_RIGHT)
                    drone_seed = (drone_seed - 1 + (key == SDLK_LEFT ? 9998U : 1U)) % 9999 + 1;
                if (tone_key(key) && !tone_held) {
                    tone_held = true;
                    editor.restore(apply_drone_scene(editor.snapshot(), drone_scene, drone_seed));
                    editor.set_page(19);
                    pending_parameters = parameters_changed = true;
                    choosing_drone = false;
                    shortcuts.cancel();
                    directions.cancel();
                    ++drone_scene_applies;
                    startup_error = "DRONE SCENE APPLIED / LEVEL AND FX KEPT";
                }
                if ((key == SDLK_LCTRL && !back_held) || (key == SDLK_LSHIFT && !page_held)) {
                    back_held = key == SDLK_LCTRL;
                    page_held = key == SDLK_LSHIFT;
                    choosing_drone = false;
                    shortcuts.cancel();
                    directions.cancel();
                    ++drone_scene_cancels;
                }
                display.invalidate();
                return;
            }
            if (styling) {
                if ((key == SDLK_e && !previous_page_held) || (key == SDLK_t && !next_page_held)) {
                    previous_page_held = key == SDLK_e;
                    next_page_held = key == SDLK_t;
                    editor.set_page(key == SDLK_e ? 21U : 0U);
                    styling = false;
                    display.set_theme(theme);
                    return;
                }

                if (key == SDLK_UP || key == SDLK_DOWN) {
                    theme_selected = (theme_selected + (key == SDLK_UP ? 5U : 1U)) % 6;
                    display.set_theme(theme_selected);
                }
                if (tone_key(key) && !tone_held) {
                    tone_held = true;
                    try {
                        if (state_directory.empty())
                            throw std::runtime_error("No state directory");
                        save_theme(state_directory.c_str(), theme_selected);
                        theme = theme_selected;
                        styling = false;
                        display.invalidate();
                    } catch (const std::exception &e) {
                        theme_message = e.what();
                        display.invalidate();
                    }
                }
                if ((key == SDLK_LCTRL && !back_held) || (key == SDLK_LSHIFT && !page_held)) {
                    back_held = key == SDLK_LCTRL;
                    page_held = key == SDLK_LSHIFT;
                    styling = false;
                    display.set_theme(theme_before);
                }
                return;
            }
            if (navigating) {
                if (key == SDLK_UP || key == SDLK_DOWN)
                    navigator.move(key == SDLK_UP ? -1 : 1);
                if (key == SDLK_e && !previous_page_held) {
                    previous_page_held = true;
                    navigator.group(-1);
                }
                if (key == SDLK_t && !next_page_held) {
                    next_page_held = true;
                    navigator.group(1);
                }
                if (tone_key(key) && !tone_held) {
                    tone_held = true;
                    if (navigator.page() == PageNavigator::export_page) {
                        composition.open_export(state_directory, sample.get());
                        shortcuts.cancel();
                        directions.cancel();
                    } else if (navigator.page() == PageNavigator::patches_page) {
                        open_patch_menu();
                    } else if (navigator.page() == PageNavigator::pattern_page) {
                        if (sample) {
                            editor.set_page(15);
                            composition.open_pattern(waveform ? &*waveform : nullptr);
                            shortcuts.cancel();
                            directions.cancel();
                        } else
                            startup_error = "LOAD A WAV WITH Y BEFORE EDITING A PATTERN";
                    } else if (navigator.page() == PageNavigator::style_page) {
                        styling = true;
                        theme_before = theme;
                        theme_selected = theme;
                        theme_message = "PREVIEW / A TO KEEP THIS PROFILE";
                    } else
                        editor.set_page(navigator.page());
                    navigating = false;
                }
                if ((key == SDLK_LCTRL && !back_held) || (key == SDLK_LSHIFT && !page_held)) {
                    back_held = key == SDLK_LCTRL;
                    page_held = key == SDLK_LSHIFT;
                    navigating = false;
                }
                display.invalidate();
                return;
            }
            if (helping && reset_confirmation) {
                if (tone_key(key) && !tone_held) {
                    tone_held = true;
                    audio.set_playing(!audio.playing());
                    ++toggles;
                }
                if (key == SDLK_UP || key == SDLK_DOWN) {
                    reset_scope = static_cast<ResetScope>(
                        (static_cast<unsigned>(reset_scope) + (key == SDLK_UP ? 2U : 1U)) % 3U);
                }
                if (key == SDLK_RCTRL && !help_held) {
                    help_held = true;
                    reset_confirmation = false;
                }
                if (key == SDLK_LCTRL && !back_held) {
                    back_held = true;
                    reset_confirmation = false;
                }
                if (key == SDLK_LSHIFT && !page_held) {
                    page_held = true;
                    reset_confirmation = false;
                    requested_effects_reset = true;
                }
                display.invalidate();
                return;
            }
            if (key == SDLK_RCTRL && !help_held) {
                help_held = true;
                helping = !helping;
                if (helping)
                    ++help_views;
                display.invalidate();
                return;
            }
            if (helping) {
                if (key == SDLK_RETURN && !patch_held && !browsing && !patching) {
                    patch_held = true;
                    reset_scope = ResetScope::Modulation;
                    reset_confirmation = true;
                    display.invalidate();
                    return;
                }
                if (key == SDLK_LCTRL) {
                    back_held = true;
                    helping = false;
                    display.invalidate();
                }
                if (!browsing && !patching) {
                    if (key == SDLK_e && !previous_page_held) {
                        previous_page_held = true;
                        waving = false;
                        change_page(-1);
                    }
                    if (key == SDLK_t && !next_page_held) {
                        next_page_held = true;
                        waving = false;
                        change_page(1);
                    }
                    if (key == SDLK_UP || key == SDLK_DOWN)
                        editor.select(key == SDLK_UP ? -1 : 1);
                    if ((sample || !editor.requires_sample()) &&
                        ((key == SDLK_LEFT || key == SDLK_RIGHT) ||
                         (key == SDLK_LSHIFT && !page_held))) {
                        if (key == SDLK_LSHIFT) {
                            page_held = true;
                            editor.reset_selected();
                        } else
                            editor.adjust(key == SDLK_LEFT ? -1 : 1, multiplier);
                        pending_parameters = true;
                        parameters_changed = true;
                    }
                }
                if (tone_key(key) && !tone_held && !browsing && !patching) {
                    tone_held = true;
                    audio.set_playing(!audio.playing());
                    ++toggles;
                }
                return;
            }
            if (key == SDLK_RETURN && !patch_held) {
                patch_held = true;
                if (patching) {
                    patching = false;
                    audio.start();
                } else
                    open_patch_menu();
                display.invalidate();
                return;
            }
            if (patching) {
                auto action = PatchAction::None;
                if (key == SDLK_UP || key == SDLK_DOWN)
                    action =
                        patch_menu->input(key == SDLK_UP ? PatchKey::Previous : PatchKey::Next);
                if (key == SDLK_LEFT || key == SDLK_RIGHT)
                    action = patch_menu->input(key == SDLK_LEFT ? PatchKey::Left : PatchKey::Right);
                if (tone_key(key) && !tone_held) {
                    tone_held = true;
                    action = patch_menu->input(PatchKey::Accept);
                }
                if (key == SDLK_LSHIFT && !page_held) {
                    page_held = true;
                    action = patch_menu->input(PatchKey::Save);
                }
                if (key == SDLK_LALT && !browser_held) {
                    browser_held = true;
                    action = patch_menu->input(PatchKey::Name);
                }
                if (key == SDLK_LCTRL && !back_held) {
                    back_held = true;
                    action = patch_menu->input(PatchKey::Back);
                }
                patch_action(action);
                return;
            }
            if (key == SDLK_LALT && !browser_held) {
                waving = false;
                browser_held = true;
                if (!browsing) {
                    audio.fade_and_pause();
                    browsing = true;
                    browser_message.clear();
                    try {
                        library.refresh();
                    } catch (const std::exception &error) {
                        browser_message = error.what();
                    }
                } else {
                    browsing = false;
                    audio.start();
                }
                display.invalidate();
                return;
            }
            if (browsing) {
                if (key == SDLK_UP || key == SDLK_DOWN)
                    library.move(key == SDLK_UP ? -1 : 1);
                if (key == SDLK_LCTRL) {
                    back_held = true;
                    try {
                        if (!library.parent()) {
                            browsing = false;
                            audio.start();
                        }
                    } catch (const std::exception &error) {
                        browser_message = error.what();
                    }
                }
                if (key == SDLK_LSHIFT && !page_held) {
                    page_held = true;
                    adopt_sample(nullptr, {});
                }
                if (tone_key(key) && !tone_held) {
                    tone_held = true;
                    if (!library.entries().empty()) {
                        if (library.entries()[library.selected()].directory) {
                            try {
                                library.enter();
                                browser_message.clear();
                            } catch (const std::exception &error) {
                                browser_message = error.what();
                            }
                        } else {
                            std::unique_ptr<grainmm::Sample> next;
                            const auto name = library.selected_path();
                            display.draw_browser(library.path(), "", "LOADING...", "", name);
                            try {
                                next = std::make_unique<grainmm::Sample>(library.load(name));
                            } catch (const std::exception &error) {
                                browser_message = error.what();
                                std::fprintf(stderr, "Sample load failed: %s\n", error.what());
                            }
                            if (next)
                                adopt_sample(std::move(next), name);
                        }
                    }
                }
                return;
            }
            if (key == SDLK_LCTRL && !back_held && !waving &&
                (editor.page_index() == 21 || (editor.page_index() == 15 && sample))) {
                back_held = true;
                if (editor.page_index() == 15)
                    composition.open_pattern(waveform ? &*waveform : nullptr);
                else
                    composition.open_export(state_directory, sample.get());
                shortcuts.cancel();
                directions.cancel();
                return;
            }
            if (key == SDLK_LCTRL && !back_held && !waving &&
                (editor.page_index() == 19 || editor.page_index() == 20)) {
                back_held = true;
                choosing_drone = true;
                drone_seed = static_cast<unsigned>(drone_scenes()[drone_scene].sound.seed);
                directions.cancel();
                shortcuts.cancel();
                display.invalidate();
                return;
            }
            if (key == SDLK_LCTRL && !back_held && sample) {
                back_held = true;
                waving = !waving;
                if (waving)
                    ++waveform_views;
                if (waving)
                    editor.show_sample();
                display.invalidate();
                return;
            }
            if (tone_key(event.key.keysym.sym) && !tone_held) {
                tone_held = true;
                audio.set_playing(!audio.playing());
                ++toggles;
            }
            {
                if (key == SDLK_LSHIFT && !page_held) {
                    waving = false;
                    page_held = true;
                    navigator.open(editor.page_index());
                    navigating = true;
                    directions.cancel();
                    display.invalidate();
                }
                if (key == SDLK_e && !previous_page_held) {
                    waving = false;
                    previous_page_held = true;
                    change_page(-1);
                }
                if (key == SDLK_t && !next_page_held) {
                    waving = false;
                    next_page_held = true;
                    change_page(1);
                }
                if (key == SDLK_UP || key == SDLK_DOWN)
                    editor.select(key == SDLK_UP ? -1 : 1);
                if ((key == SDLK_LEFT || key == SDLK_RIGHT) &&
                    (sample || !editor.requires_sample())) {
                    editor.adjust(key == SDLK_LEFT ? -1 : 1, multiplier);
                    pending_parameters = true;
                    parameters_changed = true;
                }
            }
        }
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_RETURN)
            patch_held = false;
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LCTRL)
            back_held = false;
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_RCTRL)
            help_held = false;
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LALT)
            browser_held = false;
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LSHIFT)
            page_held = false;
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_e)
            previous_page_held = false;
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_t)
            next_page_held = false;
        if (event.type == SDL_KEYUP && tone_key(event.key.keysym.sym))
            tone_held = false;
#ifndef GRAINMM_SDL1
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_EXPOSED)
            display.invalidate();
        if (event.type == SDL_AUDIODEVICEREMOVED && !event.adevice.iscapture)
            throw std::runtime_error("audio device removed; exiting safely");
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
            audio.set_playing(false);
            reset_confirmation = requested_effects_reset = false;
            shortcuts.cancel();
            directions.focus_lost();
            tone_held = page_held = browser_held = help_held = previous_page_held = next_page_held =
                patch_held = back_held = false;
        }
#else
        if (event.type == SDL_VIDEOEXPOSE ||
            (event.type == SDL_ACTIVEEVENT && event.active.gain != 0))
            display.invalidate();
        if (event.type == SDL_ACTIVEEVENT && event.active.gain == 0) {
            audio.set_playing(false);
            reset_confirmation = requested_effects_reset = false;
            shortcuts.cancel();
            directions.focus_lost();
            tone_held = page_held = browser_held = help_held = previous_page_held = next_page_held =
                patch_held = back_held = false;
        }
#endif
    };
    while (running && !interrupted) {
        const auto elapsed = SDL_GetTicks() - start;
        SDL_Event event{};
        while (running && SDL_PollEvent(&event))
            process(event);
        if (!running || interrupted)
            break;
        probe.inject(elapsed);
        if (seconds != 0 && elapsed >= seconds * 1000U)
            running = false;
        if (browsing || patching)
            reset_confirmation = requested_effects_reset = false;
        const auto repeated = directions.poll(SDL_GetTicks());
        if (repeated.direction != DirectionalRepeat::Direction::None) {
            constexpr int keys[] = {SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT};
            SDL_Event repeat_event{};
            repeat_event.type = SDL_KEYDOWN;
            repeat_event.key.keysym.sym = static_cast<decltype(repeat_event.key.keysym.sym)>(
                keys[static_cast<unsigned>(repeated.direction)]);
            process(repeat_event, repeated.multiplier, true);
        }
        if (shortcuts.poll(SDL_GetTicks(), !browsing && !patching && !navigating && !styling &&
                                               !choosing_drone && !composition.active())) {
            directions.cancel();
            reset_scope = ResetScope::Modulation;
            if (!helping)
                ++help_views;
            helping = true;
            reset_confirmation = true;
            display.invalidate();
        }
        if (requested_effects_reset) {
            requested_effects_reset = false;
            editor.reset_all_effects(reset_scope);
            if (reset_scope == ResetScope::Sound)
                audio.set_playing(false);
            audio.reset_effects(editor.snapshot());
            pending_parameters = false;
            parameters_changed = true;
            ++effects_resets;
            startup_error = reset_scope == ResetScope::Rack         ? "FX RACK RESET"
                            : reset_scope == ResetScope::Modulation ? "FX AND MODULATION RESET"
                                                                    : "SOUND DEFAULTS - PAUSED";
            std::printf("sound_reset scope=%u playing=%u\n", static_cast<unsigned>(reset_scope),
                        audio.playing() ? 1U : 0U);
            display.invalidate();
        }
        if (pending_parameters && audio.set_parameters(editor.snapshot()))
            pending_parameters = false;
        display.set_transport(audio.playing());
        if (composition.active()) {
            composition.draw();
            SDL_Delay(5);
            continue;
        }
        if (choosing_drone) {
            display.draw_drone_scenes(drone_scene, drone_seed, editor.snapshot().drone_synth.level);
            SDL_Delay(20);
            continue;
        }
        if (styling) {
            display.draw_style(theme_selected, theme_message);
            SDL_Delay(20);
            continue;
        }
        if (navigating) {
            display.draw_navigation(editor, navigator);
            SDL_Delay(40);
            continue;
        }
        if (helping) {
            display.draw_help(editor, !browsing && !patching, reset_confirmation, reset_scope);
            SDL_Delay(40);
            continue;
        }
        if (patching) {
            display.draw_patches(*patch_menu);
            SDL_Delay(40);
            continue;
        }
        if (waving && waveform) {
            display.draw_waveform(*waveform, editor, selected_sample, audio.playing());
            SDL_Delay(40);
            continue;
        }
        if (browsing) {
            const auto &entries = library.entries();
            const auto selected = library.selected();
            const auto entry_label = [&](std::size_t index) {
                return (entries[index].directory ? "DIR " : "") + entries[index].name;
            };
            display.draw_browser(library.path(), selected > 0 ? entry_label(selected - 1) : "",
                                 entries.empty() ? "NO WAV FILES" : entry_label(selected),
                                 selected + 1 < entries.size() ? entry_label(selected + 1) : "",
                                 browser_message.empty() ? "A: LOAD WAV  X: BUILT-IN SOURCE"
                                                         : browser_message);
            SDL_Delay(40);
            continue;
        }
        const auto &notice =
            !snapshot_message.empty() && SDL_GetTicks() - snapshot_message_at < 2400
                ? snapshot_message
                : startup_error;
        display.draw(sample ? selected_sample.substr(selected_sample.find_last_of('/') + 1) : "",
                     audio.playing(), editor, static_cast<bool>(sample), notice,
                     audio.sequence_view(), waveform ? &*waveform : nullptr, audio.visuals());
        SDL_Delay(display.frame_delay());
    }
    composition.close();
    audio.fade_and_pause();
    if ((parameters_changed || source_changed) && !state_directory.empty() && !interrupted) {
        grainmm::save_preset(state_directory,
                             {editor.snapshot(), selected_source, selected_sample});
        std::puts("preset saved");
    }
    const auto metrics = audio.metrics();
    std::printf(
        "exit callbacks=%u late=%u worst_us=%u toggles=%u grains=%u dropped=%u updates=%u\n",
        metrics.callbacks, metrics.late, metrics.worst_us, toggles, metrics.grains, metrics.dropped,
        metrics.updates);
    std::printf("audio_frames=%u callback_min=%u callback_max=%u span_ms=%u\n",
                metrics.processed_frames, metrics.min_frames, metrics.max_frames, metrics.span_ms);
    std::printf("ui_requests=%u ui_presentations=%u\n", display.requests(),
                display.presentations());
    std::printf("source_changes=%u help_views=%u\n", source_changes, help_views);
    std::printf("patch_saves=%u patch_loads=%u waveform_views=%u\n", patch_saves, patch_loads,
                waveform_views);
    std::printf("clipped_samples=%u\n", metrics.clipped_samples);
    std::printf("effects_resets=%u\n", effects_resets);
    std::printf("snapshot_captures=%u snapshot_recalls=%u snapshot_rejections=%u\n",
                snapshot_captures, snapshot_recalls, snapshot_rejections);
    std::printf("slices_triggered=%u\n", audio.slices_triggered());
    std::printf("drone_scene_applies=%u drone_scene_cancels=%u output_reduction=%u\n",
                drone_scene_applies, drone_scene_cancels, audio.visuals().output_reduction);
    std::printf("drone_state=%u drone_events=%u\n", audio.visuals().drone,
                audio.visuals().drone_count);
    std::printf("collision_hits=%u ui_layouts=%u\n", audio.visuals().hits, display.layouts());
    std::printf("exports_completed=%u exports_cancelled=%u pattern_views=%u\n",
                composition.exports_completed(), composition.exports_cancelled(),
                composition.pattern_views());
    const auto final_parameters = editor.snapshot();
    std::printf("synth_level=%.3f compressor_mix=%.3f source=%s\n",
                static_cast<double>(final_parameters.oscillator.level),
                static_cast<double>(final_parameters.compressor.mix), sample ? "wav" : "builtin");
    std::printf("source_mode=%u pending_restore=%u\n", static_cast<unsigned>(selected_source),
                pending_sample_restore ? 1U : 0U);
    if (self_test &&
        (!probe.complete() ||
         toggles != (options.tools_test ? 3U
                     : browser_test || library_test || failure_test || options.harmony_test ||
                             options.scene_test || options.snapshot_test || options.evolve_test ||
                             options.drone_test || options.ambient_test || options.pattern_test ||
                             options.performance_test
                         ? 2U
                         : 1U) ||
         (browser_test && source_changes == 0) || (synth_test && metrics.updates == 0) ||
         (rack_test && help_views == 0) ||
         (library_test &&
          (patch_saves != 1 || patch_loads != 1 || waveform_views < 2 || help_views != 1)) ||
         ((options.fx_reset_test || options.shortcut_test || options.sound_reset_test) &&
          effects_resets != 1) ||
         (options.ambient_test && (drone_scene_applies != 1 || drone_scene_cancels != 2 ||
                                   toggles != 2 || metrics.updates == 0)) ||
         (options.performance_test &&
          (metrics.updates == 0 || help_views != 1 || final_parameters.oscillator_b.level == 0 ||
           final_parameters.synth_color.sub_level == 0 || final_parameters.groove.length != 15)) ||
         (options.drone_test && (audio.visuals().drone == 0 || audio.visuals().drone_count == 0 ||
                                 metrics.updates == 0 || help_views != 1)) ||
         (options.evolve_test &&
          (metrics.grains == 0 || metrics.updates == 0 || help_views != 1)) ||
         (options.snapshot_test &&
          (snapshot_captures != 2 || snapshot_recalls != 2 || snapshot_rejections != 2 ||
           source_changes != 1 || help_views != 0)) ||
         (options.scene_test && (audio.visuals().hits == 0 || help_views != 1)) ||
         (options.slice_test && (audio.slices_triggered() == 0 || help_views != 1)) ||
         (options.pattern_test && (!editor.snapshot().slice_pattern.manual ||
                                   editor.snapshot().slice_pattern.steps[1].enabled ||
                                   audio.slices_triggered() == 0 || help_views != 1)) ||
         (options.tools_test &&
          (composition.exports_completed() != 1 || composition.pattern_views() != 1 ||
           source_changes != 1 || selected_sample != "exports/GrainMM-001.wav")) ||
         (options.texture_test && effects_resets != 0) ||
         (failure_test && (patch_loads != 0 || patch_saves != 0 || source_changes != 0)) ||
         metrics.callbacks == 0 || interrupted ||
         (sample && ((metrics.grains == 0 && audio.slices_triggered() == 0) ||
                     (!browser_test && metrics.updates == 0)))))
        throw std::runtime_error("lifecycle self-test did not complete");
    return 0;
}
