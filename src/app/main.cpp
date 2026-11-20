// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/app_options.hpp"
#include <cstdio>
#include <exception>
int main(int argc, char **argv) {
    try {
        std::setvbuf(stdout, nullptr, _IOLBF, 0);
        const auto options = grainmm::parse_options(argc, argv);
        if (options.action == grainmm::AppAction::Help) {
            grainmm::print_help();
            return 0;
        }
        if (options.action == grainmm::AppAction::Version) {
            std::puts("GrainMM " GRAINMM_VERSION " — created by elly (@lkotlarenko)");
            return 0;
        }
        return grainmm::run_application(options);
    } catch (const std::exception &error) {
        std::fprintf(stderr, "GrainMM: %s\n", error.what());
        return 1;
    }
}
