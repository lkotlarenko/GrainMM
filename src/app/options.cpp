// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/app_options.hpp"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
namespace grainmm {
namespace {
unsigned parse_seconds(const char *value) {
    char *end = nullptr;
    const auto result = std::strtoul(value, &end, 10);
    if (!*value || *end || result < 1 || result > 3600)
        throw std::runtime_error("--seconds must be an integer in1..3600");
    return static_cast<unsigned>(result);
}
} // namespace
AppOptions parse_options(int argc, char **argv) {
    AppOptions options;
    if (const char *directory = std::getenv("GRAINMM_SAMPLE_DIR"))
        options.sample_directory = directory;
    if (const char *resume = std::getenv("GRAINMM_RESUME_SAMPLE"))
        options.resume_sample = std::string(resume) == "1";
    if (const char *state = std::getenv("GRAINMM_STATE_DIR"))
        options.state_directory = state;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--resume-sample")
            options.resume_sample = true;
        else if (arg == "--self-test-performance") {
            options.self_test = options.performance_test = true;
        } else if (arg == "--self-test-tools") {
            options.self_test = options.tools_test = true;
        } else if (arg == "--self-test-pattern") {
            options.self_test = options.pattern_test = true;
        } else if (arg == "--self-test-ambient") {
            options.self_test = options.ambient_test = true;
        } else if (arg == "--self-test-drone") {
            options.self_test = options.drone_test = true;
        } else if (arg == "--self-test-evolve") {
            options.self_test = options.evolve_test = true;
        } else if (arg == "--self-test-snapshot") {
            options.self_test = options.snapshot_test = true;
        } else if (arg == "--self-test-scene") {
            options.self_test = options.scene_test = true;
        } else if (arg == "--self-test-slice") {
            options.self_test = options.slice_test = true;
        } else if (arg == "--self-test-reset-sound") {
            options.self_test = options.sound_reset_test = true;
        } else if (arg == "--self-test-repeat") {
            options.self_test = options.repeat_test = true;
        } else if (arg == "--self-test-shortcut") {
            options.self_test = options.shortcut_test = true;
        } else if (arg == "--self-test-texture") {
            options.self_test = options.texture_test = true;
        } else if (arg == "--self-test-fx-reset") {
            options.self_test = options.fx_reset_test = true;
        } else if (arg == "--self-test-harmony") {
            options.self_test = options.harmony_test = true;
        } else if (arg == "--self-test-patch-failure") {
            options.self_test = options.patch_failure_test = true;
        } else if (arg == "--self-test-library") {
            options.self_test = options.library_test = true;
        } else if (arg == "--self-test-rack") {
            options.self_test = options.rack_test = true;
        } else if (arg == "--self-test-synth") {
            options.self_test = options.synth_test = true;
        } else if (arg == "--self-test-browser") {
            options.self_test = options.browser_test = true;
        } else if (arg == "--self-test")
            options.self_test = true;
        else if (arg == "--state-dir" && i + 1 < argc)
            options.state_directory = argv[++i];
        else if (arg == "--sample-dir" && i + 1 < argc)
            options.sample_directory = argv[++i];
        else if (arg == "--sample" && i + 1 < argc)
            options.sample_name = argv[++i];
        else if (arg == "--seconds" && i + 1 < argc)
            options.seconds = parse_seconds(argv[++i]);
        else if (arg == "--buffer" && i + 1 < argc) {
            const std::string size = argv[++i];
            if (size != "256" && size != "512" && size != "1024")
                throw std::runtime_error("--buffer must be 256, 512 or 1024");
            options.buffer = std::stoi(size);
        } else if (arg == "--version") {
            options.action = AppAction::Version;
            return options;
        } else if (arg == "--help") {
            options.action = AppAction::Help;
            return options;
        } else
            throw std::runtime_error("unknown or incomplete argument: " + arg);
    }
    if (options.self_test && options.seconds == 0)
        throw std::runtime_error("self-test requires --seconds");
    if (static_cast<unsigned>(options.browser_test) + static_cast<unsigned>(options.synth_test) +
            static_cast<unsigned>(options.rack_test) + static_cast<unsigned>(options.library_test) +
            static_cast<unsigned>(options.patch_failure_test) +
            static_cast<unsigned>(options.harmony_test) +
            static_cast<unsigned>(options.texture_test) +
            static_cast<unsigned>(options.fx_reset_test) +
            static_cast<unsigned>(options.shortcut_test) +
            static_cast<unsigned>(options.repeat_test) +
            static_cast<unsigned>(options.sound_reset_test) +
            static_cast<unsigned>(options.slice_test) + static_cast<unsigned>(options.scene_test) +
            static_cast<unsigned>(options.snapshot_test) +
            static_cast<unsigned>(options.evolve_test) + static_cast<unsigned>(options.drone_test) +
            static_cast<unsigned>(options.ambient_test) +
            static_cast<unsigned>(options.pattern_test) +
            static_cast<unsigned>(options.tools_test) +
            static_cast<unsigned>(options.performance_test) >
        1)
        throw std::runtime_error("choose one self-test mode");
    if (options.performance_test &&
        (options.seconds < 9 || options.state_directory.empty() || options.sample_name.empty()))
        throw std::runtime_error("performance self-test requires WAV, state and seconds9+");
    if (options.ambient_test &&
        (options.seconds < 12 || options.state_directory.empty() || !options.sample_name.empty()))
        throw std::runtime_error(
            "ambient self-test requires built-in source, state and seconds12+");
    if (options.drone_test &&
        (options.seconds < 8 || options.state_directory.empty() || !options.sample_name.empty()))
        throw std::runtime_error(
            "drone synth self-test requires built-in source, state and seconds8+");
    if (options.evolve_test && (options.seconds < 6 || options.state_directory.empty()))
        throw std::runtime_error("drone self-test needs seconds6+ and state");
    if (options.snapshot_test && (options.seconds < 4 || options.state_directory.empty()))
        throw std::runtime_error("snapshot self-test needs seconds4+ and state");
    if (options.synth_test && (options.seconds < 2 || !options.sample_name.empty()))
        throw std::runtime_error("synth self-test requires no explicit sample and --seconds 2+");
    if (options.rack_test && options.seconds < 3)
        throw std::runtime_error("rack self-test requires --seconds 3+");
    if (options.browser_test && options.seconds < 2)
        throw std::runtime_error("browser self-test requires --seconds 2 or greater");
    if (options.library_test && (options.seconds < 4 || options.state_directory.empty()))
        throw std::runtime_error("library self-test requires --seconds 4+ and --state-dir");
    if (options.patch_failure_test && (options.seconds < 2 || options.state_directory.empty()))
        throw std::runtime_error("patch failure self-test needs --seconds2+ and --state-dir");
    if (options.harmony_test && (options.seconds < 4 || options.state_directory.empty()))
        throw std::runtime_error("harmony self-test requires --seconds4+ and --state-dir");
    if ((options.texture_test || options.fx_reset_test) &&
        (options.seconds < (options.fx_reset_test ? 6U : 4U) || options.state_directory.empty()))
        throw std::runtime_error("texture/reset self-test requires state and seconds4+/6+");
    if ((options.shortcut_test || options.sound_reset_test) &&
        (options.seconds < 3 || options.state_directory.empty()))
        throw std::runtime_error("shortcut self-test needs seconds3+ and state");
    if (options.repeat_test && (options.seconds < 4 || options.state_directory.empty()))
        throw std::runtime_error("repeat self-test needs seconds4+ and state");
    if (options.tools_test && (options.seconds < 12 || options.state_directory.empty()))
        throw std::runtime_error("tools self-test needs seconds12+ and state");
    if (options.pattern_test && (options.seconds < 4 || options.state_directory.empty()))
        throw std::runtime_error("pattern self-test needs seconds4+ and state");
    if (options.slice_test && (options.seconds < 4 || options.state_directory.empty()))
        throw std::runtime_error("slice self-test needs seconds4+ and state");
    if (options.scene_test && (options.seconds < 8 || options.state_directory.empty()))
        throw std::runtime_error("scene self-test needs seconds8+ and state");
    return options;
}
void print_help() {
    std::puts("GrainMM native instrument: [--seconds 1..3600] [--buffer 256|512|1024]\n"
              "Space/A: toggle playback; Escape/MENU: exit. Starts silent.\n"
              "--sample-dir DIRECTORY --sample RELATIVE_WAV: user PCM16 WAV grains.\n"
              "Without a sample, starts with the gentle tone; raise OSC/LEVEL for the "
              "synth.\n"
              "--state-dir DIRECTORY: restore parameters and save edits on clean exit.\n"
              "Y/LeftAlt: samples; X/LeftShift: grouped module navigator. Pages: "
              "GRAIN/DELAY/EQ/REVERB/MOTION/OSC/COMP/TAPE/RESONATE/RHYTHM/SAMPLE/HARMONY/CRUSH/"
              "FILTER/CHORUS/SLICE/BALLS/BOWL/EVOLVE/DRONE/GENERATE/OUTPUT; "
              "X opens grouped modules; L/R changes pages. "
              "B/LeftCtrl: "
              "back.\n"
              "--resume-sample: restore the saved selection (requires a state directory).\n"
              "--self-test requires dummy audio and an explicit duration.\n"
              "--self-test-browser tests loading; --self-test-synth tests no-WAV synthesis; "
              "seconds 2+. --self-test-rack checks expanded controls/help; seconds 3+.\n"
              "START/Return tap: patches; SELECT/RightCtrl tap: help. Hold both0.8s for FX "
              "confirmation; choose with Up/Down, X confirms. Hold arrows to repeat/accelerate.\n"
              "B/LeftCtrl: SLICE pattern editor, OUTPUT WAV export, DRONE scenes; otherwise trim.\n"
              "Pattern: L/R step, D-pad edit, X rest, Y manual/generated, A play, B back.\n"
              "--self-test-pattern: WAV, state and seconds4+; dummy audio.\n"
              "--self-test-performance: WAV, state, seconds9+; dummy audio.\n"
              "--self-test-tools: WAV, shared sample/state directory, seconds12+; dummy audio.\n"
              "Main view: hold SELECT first, tap A capture / B recall temporary sound.\n"
              "--self-test-ambient: built-in source, state and seconds12+; dummy audio only.\n"
              "--self-test-snapshot: WAV, state and seconds4+; dummy audio only.\n"
              "--self-test-library: WAV, state directory and seconds4+; dummy audio only.\n"
              "--self-test-harmony: WAV, state directory and seconds4+; dummy audio only.\n"
              "--self-test-slice: WAV, state and seconds4+; dummy audio.\n"
              "--self-test-shortcut: seconds3+ and state; dummy audio.\n"
              "GrainBrick by ETRNLIAM: https://github.com/ETRNLIAM/GrainBrick");
}
} // namespace grainmm
