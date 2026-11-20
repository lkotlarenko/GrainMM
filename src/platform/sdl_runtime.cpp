// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/sdl_runtime.hpp"
#include <SDL.h>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace grainmm {
SdlRuntime::SdlRuntime() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        const std::string error = SDL_GetError();
        SDL_Quit();
        throw std::runtime_error(error);
    }
#ifdef GRAINMM_SDL1
    char name[128]{};
    const char *driver = SDL_VideoDriverName(name, sizeof(name));
    std::printf("video=%s\n", driver ? driver : "unknown");
#else
    const char *driver = SDL_GetCurrentVideoDriver();
    std::printf("video=%s\n", driver ? driver : "unknown");
#endif
}
SdlRuntime::~SdlRuntime() { SDL_Quit(); }
} // namespace grainmm
