// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/display.hpp"
#include "grainmm/parameter_editor.hpp"
#include "grainmm/sdl_runtime.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
int main(int argc, char **argv) {
    unsigned requests = 2000;
    if (argc != 1) {
        if (argc != 3 || std::strcmp(argv[1], "--requests"))
            return 1;
        char *end = nullptr;
        const auto n = std::strtoul(argv[2], &end, 10);
        if (!*argv[2] || *end || n < 1 || n > 2000)
            return 1;
        requests = static_cast<unsigned>(n);
    }
    const char *driver = std::getenv("SDL_VIDEODRIVER");
    if (!driver || std::strcmp(driver, "dummy")) {
        std::fprintf(
            stderr, "UI benchmark requires SDL_VIDEODRIVER=dummy; no hardware display is opened\n");
        return 1;
    }
    try {
        grainmm::SdlRuntime runtime;
        for (unsigned mode = 0; mode < 3; ++mode) {
            grainmm::Display display;
            grainmm::ParameterEditor editor;
            editor.set_page(13);

            const auto begin = std::chrono::steady_clock::now();
            for (unsigned i = 0; i < requests; ++i) {
                if (mode == 1)
                    editor.select(1);
                if (mode == 2)
                    editor.adjust(i % 2 ? 1 : -1);
                display.draw("", false, editor, false);
            }
            const double us =
                std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - begin)
                    .count();
            std::printf("UI mode=%s requests=%u presentations=%u layouts=%u scenes=%u "
                        "us_per_request=%.3f\n",
                        mode == 0   ? "idle"
                        : mode == 1 ? "selection"
                                    : "editing",
                        requests, display.presentations(), display.layouts(), display.scenes(),
                        us / requests);
        }
    } catch (const std::exception &e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
}
