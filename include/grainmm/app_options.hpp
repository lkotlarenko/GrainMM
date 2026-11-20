// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <string>
namespace grainmm {
enum class AppAction { Run, Help, Version };
struct AppOptions {
    AppAction action = AppAction::Run;
    bool self_test = false, browser_test = false, synth_test = false, rack_test = false,
         library_test = false, patch_failure_test = false, harmony_test = false,
         texture_test = false, fx_reset_test = false, shortcut_test = false, repeat_test = false,
         sound_reset_test = false, slice_test = false, scene_test = false, snapshot_test = false,
         evolve_test = false, drone_test = false, ambient_test = false, pattern_test = false,
         tools_test = false, performance_test = false, resume_sample = false;
    unsigned seconds = 0;
    int buffer = 1024;
    std::string sample_directory = ".", sample_name, state_directory;
};
AppOptions parse_options(int argc, char **argv);
void print_help();
int run_application(const AppOptions &options);
} // namespace grainmm
