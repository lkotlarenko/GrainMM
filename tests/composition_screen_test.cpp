// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/composition_screen.hpp"
#include "grainmm/display.hpp"
#include "grainmm/parameter_editor.hpp"
#include "grainmm/sdl_audio.hpp"
#include "grainmm/sdl_runtime.hpp"
#include "grainmm/waveform.hpp"
#include <SDL.h>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <unistd.h>
#include <vector>
namespace fs = std::filesystem;
namespace {
void check(bool ok) {
    if (!ok)
        throw std::runtime_error("composition screen regression");
}
} // namespace
int main() {
    char pattern[] = "/tmp/grainmm-composition-XXXXXX";
    const auto path = ::mkdtemp(pattern);
    if (!path)
        return 1;
    const fs::path root(path);
    try {
        grainmm::Sample sample(48000, 1, std::vector<float>(4800, .1F));
        grainmm::Waveform waveform(sample);
        grainmm::SdlRuntime runtime;
        grainmm::Display display;
        grainmm::ParameterEditor editor;
        grainmm::SdlAudio audio(1024, &sample, true);
        audio.start();
        grainmm::CompositionScreen screen(display, editor, audio);
        screen.open_pattern(&waveform);
        check(screen.active());
        check(screen.input(SDLK_LALT));
        check(editor.snapshot().slice_pattern.manual && editor.snapshot().slice.mode == 1);
        screen.input(SDLK_t);
        check(screen.input(SDLK_LSHIFT) && !editor.snapshot().slice_pattern.steps[1].enabled);
        audio.set_parameters(editor.snapshot());
        screen.input(SDLK_SPACE);
        check(audio.playing());
        screen.draw();
        screen.input(SDLK_LCTRL);
        check(!screen.active() && audio.playing());
        screen.open_export(root.string(), &sample);
        check(!audio.playing());
        screen.input(SDLK_LEFT); // 15 -> 5 seconds.
        screen.input(SDLK_SPACE);
        check(screen.rendering());
        screen.draw();
        screen.input(SDLK_LCTRL);
        check(!screen.rendering() && screen.active());
        check(fs::is_empty(root / "exports"));
        screen.input(SDLK_SPACE);
        unsigned ticks = 0;
        while (screen.rendering() && ticks++ < 1000)
            screen.draw();
        check(!screen.rendering() && ticks < 1000);
        check(fs::file_size(root / "exports/GrainMM-001.wav") == 44 + 5 * 48000 * 4);
        screen.input(SDLK_LCTRL);
        check(!screen.active() && !audio.playing());
        screen.open_export(root.string(), &sample);
        screen.input(SDLK_SPACE);
        screen.close(); // MENU/exit cancellation leaves completed export intact.
        check(!fs::exists(root / "exports/GrainMM-002.wav"));
        check(fs::exists(root / "exports/GrainMM-001.wav"));
        check(screen.exports_completed() == 1 && screen.exports_cancelled() == 2 &&
              screen.pattern_views() == 1);
        std::puts(
            "composition screens: live pattern edits, export/cancel, exit and paused return pass");
        fs::remove_all(root);
    } catch (const std::exception &e) {
        std::fprintf(stderr, "%s\n", e.what());
        fs::remove_all(root);
        return 1;
    }
}
