// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/lifecycle_probe.hpp"
#include "grainmm/page_navigator.hpp"
#include <SDL.h>
#include <iterator>
#include <stdexcept>
namespace grainmm {
namespace {
constexpr int performance_keys[] = {
    SDLK_LSHIFT, SDLK_UP,    SDLK_UP,    SDLK_SPACE, SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT,
    SDLK_t,      SDLK_UP,    SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT,
    SDLK_DOWN,   SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_LSHIFT,
    SDLK_t,      SDLK_t,     SDLK_DOWN,  SDLK_SPACE, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,
    SDLK_LEFT,   SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT,
    SDLK_DOWN,   SDLK_LEFT,  SDLK_SPACE, SDLK_RCTRL, SDLK_RCTRL, SDLK_SPACE, SDLK_ESCAPE};
constexpr int tools_keys[] = {SDLK_LSHIFT, SDLK_DOWN,   SDLK_DOWN,  SDLK_DOWN,   SDLK_DOWN,
                              SDLK_DOWN,   SDLK_DOWN,   SDLK_SPACE, SDLK_LSHIFT, SDLK_SPACE,
                              SDLK_LCTRL,  SDLK_LSHIFT, SDLK_e,     SDLK_SPACE,  SDLK_LEFT,
                              SDLK_SPACE,  SDLK_LCTRL,  SDLK_LALT,  SDLK_SPACE,  SDLK_SPACE,
                              SDLK_SPACE,  SDLK_SPACE,  SDLK_ESCAPE};
constexpr int pattern_keys[] = {SDLK_LCTRL, SDLK_LALT,  SDLK_t,     SDLK_LSHIFT, SDLK_DOWN,
                                SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_LEFT,   SDLK_DOWN,
                                SDLK_LEFT,  SDLK_e,     SDLK_SPACE, SDLK_LCTRL,  SDLK_RCTRL,
                                SDLK_RCTRL, SDLK_SPACE, SDLK_ESCAPE};
constexpr int ambient_keys[] = {
    SDLK_LSHIFT, SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_SPACE,
    SDLK_RIGHT,  SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT,
    SDLK_RIGHT,  SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT, SDLK_SPACE, SDLK_LCTRL, SDLK_DOWN,  SDLK_RIGHT,
    SDLK_LCTRL,  SDLK_LCTRL, SDLK_DOWN,  SDLK_RIGHT, SDLK_SPACE, SDLK_LCTRL, SDLK_DOWN,  SDLK_LCTRL,
    SDLK_LSHIFT, SDLK_e,     SDLK_DOWN,  SDLK_DOWN,  SDLK_SPACE, SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT,
    SDLK_RIGHT,  SDLK_RIGHT, SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_LEFT,  SDLK_DOWN,
    SDLK_RIGHT,  SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_RCTRL, SDLK_RCTRL, SDLK_SPACE,
    SDLK_ESCAPE};
constexpr int evolve_keys[] = {
    SDLK_LSHIFT, SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,   SDLK_SPACE,
    SDLK_RIGHT,  SDLK_RIGHT, SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN, SDLK_RIGHT,
    SDLK_DOWN,   SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_SPACE, SDLK_DOWN, SDLK_RIGHT,
    SDLK_LEFT,   SDLK_RCTRL, SDLK_RCTRL, SDLK_SPACE, SDLK_ESCAPE};
constexpr int pad_keys[] = {
    SDLK_LSHIFT, SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_SPACE,
    SDLK_RIGHT,  SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT, SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,
    SDLK_RIGHT,  SDLK_DOWN,  SDLK_LEFT,  SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_t,
    SDLK_UP,     SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT,
    SDLK_DOWN,   SDLK_LEFT,  SDLK_DOWN,  SDLK_LEFT,  SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT,
    SDLK_e,      SDLK_SPACE, SDLK_RCTRL, SDLK_RCTRL, SDLK_SPACE, SDLK_ESCAPE};
constexpr int snapshot_keys[] = {
    SDLK_SPACE,   -SDLK_SPACE, SDLK_RCTRL,  SDLK_LCTRL,  -SDLK_LCTRL, -SDLK_RCTRL, SDLK_RCTRL,
    SDLK_SPACE,   -SDLK_SPACE, -SDLK_RCTRL, SDLK_RIGHT,  -SDLK_RIGHT, SDLK_RCTRL,  SDLK_LCTRL,
    -SDLK_LCTRL,  -SDLK_RCTRL, SDLK_SPACE,  -SDLK_SPACE, SDLK_LALT,   -SDLK_LALT,  SDLK_LSHIFT,
    -SDLK_LSHIFT, SDLK_RCTRL,  SDLK_LCTRL,  -SDLK_LCTRL, -SDLK_RCTRL, SDLK_RCTRL,  SDLK_SPACE,
    -SDLK_SPACE,  -SDLK_RCTRL, SDLK_RIGHT,  -SDLK_RIGHT, SDLK_RCTRL,  SDLK_LCTRL,  -SDLK_LCTRL,
    -SDLK_RCTRL,  SDLK_ESCAPE};
constexpr int scene_keys[] = {
    SDLK_LSHIFT, SDLK_DOWN,   SDLK_DOWN,  SDLK_DOWN,  SDLK_DOWN,   SDLK_DOWN,  SDLK_DOWN,
    SDLK_DOWN,   SDLK_SPACE,  SDLK_RIGHT, SDLK_DOWN,  SDLK_DOWN,   SDLK_DOWN,  SDLK_DOWN,
    SDLK_DOWN,   SDLK_RIGHT,  SDLK_SPACE, SDLK_t,     SDLK_DOWN,   SDLK_RIGHT, SDLK_RCTRL,
    SDLK_RCTRL,  SDLK_LSHIFT, SDLK_LCTRL, SDLK_SPACE, SDLK_LSHIFT, SDLK_t,     SDLK_t,
    SDLK_t,      SDLK_DOWN,   SDLK_DOWN,  SDLK_DOWN,  SDLK_SPACE,  SDLK_DOWN,  SDLK_LCTRL,
    SDLK_LSHIFT, SDLK_e,      SDLK_DOWN,  SDLK_DOWN,  SDLK_DOWN,   SDLK_SPACE, SDLK_DOWN,
    SDLK_SPACE,  SDLK_ESCAPE};
constexpr int texture_keys[] = {SDLK_e,    SDLK_e,    SDLK_e,     SDLK_e,     SDLK_DOWN, SDLK_DOWN,
                                SDLK_DOWN, SDLK_DOWN, SDLK_DOWN,  SDLK_RIGHT, SDLK_e,    SDLK_RIGHT,
                                SDLK_UP,   SDLK_UP,   SDLK_UP,    SDLK_UP,    SDLK_UP,   SDLK_LEFT,
                                SDLK_e,    SDLK_LEFT, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN, SDLK_DOWN,
                                SDLK_DOWN, SDLK_DOWN, SDLK_RIGHT, SDLK_SPACE};
constexpr unsigned texture_count = sizeof(texture_keys) / sizeof(texture_keys[0]);
constexpr int slice_keys[] = {SDLK_e,     SDLK_e,     SDLK_e,     SDLK_RIGHT, SDLK_DOWN,
                              SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_LEFT,
                              SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_SPACE,
                              SDLK_RCTRL, SDLK_RCTRL, SDLK_ESCAPE};
constexpr int harmony_keys[] = {
    SDLK_e,      SDLK_e,     SDLK_e,     SDLK_e,     SDLK_e,     SDLK_e,     SDLK_e,    SDLK_DOWN,
    SDLK_RIGHT,  SDLK_RIGHT, SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_RIGHT, SDLK_DOWN, SDLK_RIGHT,
    SDLK_DOWN,   SDLK_LEFT,  SDLK_DOWN,  SDLK_RIGHT, SDLK_SPACE, SDLK_RCTRL, SDLK_UP,   SDLK_LEFT,
    SDLK_LSHIFT, SDLK_t,     SDLK_e,     SDLK_RCTRL, SDLK_SPACE, SDLK_ESCAPE};
constexpr int library_keys[] = {SDLK_SPACE,  SDLK_LCTRL,  SDLK_RIGHT, SDLK_LCTRL,  SDLK_RETURN,
                                SDLK_LALT,   SDLK_UP,     SDLK_RIGHT, SDLK_DOWN,   SDLK_SPACE,
                                SDLK_LSHIFT, SDLK_RETURN, SDLK_RIGHT, SDLK_RETURN, SDLK_SPACE,
                                SDLK_SPACE,  SDLK_LCTRL,  SDLK_RCTRL, SDLK_RCTRL,  SDLK_LCTRL,
                                SDLK_RETURN, SDLK_LSHIFT, SDLK_LCTRL, SDLK_RETURN, SDLK_ESCAPE};
} // namespace

unsigned LifecycleProbe::steps() const noexcept {
    switch (mode_) {
    case ProbeMode::Performance:
        return static_cast<unsigned>(std::size(performance_keys) * 2 - 1);
    case ProbeMode::Tools:
        return static_cast<unsigned>(std::size(tools_keys) * 2 - 1);
    case ProbeMode::Pattern:
        return static_cast<unsigned>(std::size(pattern_keys) * 2 - 1);
    case ProbeMode::Ambient:
        return static_cast<unsigned>(std::size(ambient_keys) * 2 - 1);
    case ProbeMode::Drone:
        return static_cast<unsigned>(std::size(pad_keys) * 2 - 1);
    case ProbeMode::Evolve:
        return static_cast<unsigned>(std::size(evolve_keys) * 2 - 1);
    case ProbeMode::Snapshot:
        return static_cast<unsigned>(std::size(snapshot_keys));
    case ProbeMode::Scene:
        return static_cast<unsigned>(std::size(scene_keys) * 2 - 1);
    case ProbeMode::Slice:
        return static_cast<unsigned>(sizeof(slice_keys) / sizeof(slice_keys[0]) * 2 - 1);
    case ProbeMode::SoundReset:
        return 13;
    case ProbeMode::Repeat:
        return 6;
    case ProbeMode::Shortcut:
        return 11;
    case ProbeMode::Texture:
        return 2 * (texture_count + 3) - 1;
    case ProbeMode::FxReset:
        return 2 * texture_count + 16;
    case ProbeMode::None:
        return 0;
    case ProbeMode::Basic:
        return 15;
    case ProbeMode::Browser:
        return 21;
    case ProbeMode::Synth:
        return 27;
    case ProbeMode::Rack:
        return 33;
    case ProbeMode::Sample:
        return 45;
    case ProbeMode::Harmony:
        return static_cast<unsigned>(sizeof(harmony_keys) / sizeof(harmony_keys[0]) * 2 - 1);
    case ProbeMode::PatchFailure:
        return 11;
    case ProbeMode::Library:
        return static_cast<unsigned>(sizeof(library_keys) / sizeof(library_keys[0]) * 2 - 1);
    }
    return 0;
}
void LifecycleProbe::inject(unsigned elapsed_ms) {
    if (stage_ >= steps() ||
        elapsed_ms <
            100 + stage_ * (mode_ == ProbeMode::Basic ? 50U : 60U) +
                ((mode_ == ProbeMode::Shortcut || mode_ == ProbeMode::SoundReset) && stage_ >= 5
                     ? 900U
                     : 0U) +
                (mode_ == ProbeMode::Repeat && stage_ >= 4 ? 2500U : 0U) +
                // Scene's first A/play is key 16: wait after its down/up pair.
                (mode_ == ProbeMode::Scene && stage_ >= 34 ? 1400U : 0U) +
                (mode_ == ProbeMode::Evolve && stage_ >= 42 ? 800U : 0U) +
                (mode_ == ProbeMode::Drone && stage_ >= 84 ? 1000U : 0U) +
                (mode_ == ProbeMode::Performance && stage_ >= 76 ? 800U : 0U) +
                (mode_ == ProbeMode::Tools && stage_ >= 32 ? 4000U : 0U) +
                (mode_ == ProbeMode::Tools && stage_ >= 42 ? 500U : 0U))
        return;
    constexpr int basic[] = {SDLK_SPACE, SDLK_SPACE, SDLK_SPACE, SDLK_DOWN,  SDLK_DOWN,
                             SDLK_RIGHT, SDLK_RIGHT, SDLK_t,     SDLK_t,     SDLK_UP,
                             SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_RIGHT, SDLK_ESCAPE};
    constexpr int browser[] = {
        SDLK_SPACE, SDLK_SPACE, SDLK_SPACE, SDLK_DOWN,  SDLK_DOWN,  SDLK_RIGHT, SDLK_RIGHT,
        SDLK_t,     SDLK_t,     SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_RIGHT,
        SDLK_LALT,  SDLK_LALT,  SDLK_SPACE, SDLK_SPACE, SDLK_SPACE, SDLK_SPACE, SDLK_ESCAPE};
    constexpr int synth[] = {SDLK_RIGHT, SDLK_RIGHT, SDLK_SPACE, SDLK_SPACE, SDLK_DOWN,  SDLK_DOWN,
                             SDLK_RIGHT, SDLK_RIGHT, SDLK_DOWN,  SDLK_DOWN,  SDLK_RIGHT, SDLK_RIGHT,
                             SDLK_t,     SDLK_t,     SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_UP,
                             SDLK_UP,    SDLK_UP,    SDLK_RIGHT, SDLK_RIGHT, SDLK_t,     SDLK_t,
                             SDLK_RIGHT, SDLK_RIGHT, SDLK_ESCAPE};
    bool key_up =
        stage_ == 2 || stage_ == 4 || stage_ == 6 || stage_ == 8 || stage_ == 10 || stage_ == 12;
    const int *keys = basic;
    if (mode_ == ProbeMode::Browser) {
        keys = browser;
        key_up = key_up || stage_ == 15 || stage_ == 17 || stage_ == 19;
    }
    if (mode_ == ProbeMode::Synth) {
        keys = synth;
        key_up = stage_ % 2 == 1;
    }
    constexpr int rack[] = {SDLK_RIGHT, SDLK_SPACE, SDLK_t,     SDLK_t,     SDLK_t,     SDLK_UP,
                            SDLK_RIGHT, SDLK_t,     SDLK_UP,    SDLK_UP,    SDLK_UP,    SDLK_RIGHT,
                            SDLK_DOWN,  SDLK_RIGHT, SDLK_RCTRL, SDLK_RCTRL, SDLK_ESCAPE};
    constexpr int sample[] = {SDLK_e,     SDLK_e,     SDLK_e,     SDLK_e,     SDLK_e,     SDLK_e,
                              SDLK_e,     SDLK_e,     SDLK_RIGHT, SDLK_DOWN,  SDLK_LEFT,  SDLK_DOWN,
                              SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,  SDLK_RIGHT, SDLK_DOWN,
                              SDLK_LEFT,  SDLK_SPACE, SDLK_RCTRL, SDLK_RCTRL, SDLK_ESCAPE};
    int key;
    constexpr int failure_keys[] = {SDLK_SPACE,  SDLK_RETURN, SDLK_SPACE,
                                    SDLK_RETURN, SDLK_SPACE,  SDLK_ESCAPE};
    if (mode_ == ProbeMode::Performance) {
        key = performance_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Tools) {
        key = tools_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Pattern) {
        key = pattern_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Ambient) {
        key = ambient_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Drone) {
        key = pad_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Evolve) {
        key = evolve_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Snapshot) {
        key = snapshot_keys[stage_];
        key_up = key < 0;
        if (key_up)
            key = -key;
    } else if (mode_ == ProbeMode::Scene) {
        key = scene_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Slice) {
        key = slice_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Repeat) {
        constexpr int held[] = {SDLK_SPACE, SDLK_SPACE, SDLK_RIGHT,
                                SDLK_RIGHT, SDLK_RIGHT, SDLK_ESCAPE};
        key = held[stage_];
        key_up = stage_ == 1 || stage_ == 4;
    } else if (mode_ == ProbeMode::SoundReset) {
        constexpr int sound_keys[] = {
            SDLK_SPACE, SDLK_SPACE, SDLK_RCTRL,  SDLK_RETURN, SDLK_RCTRL,  SDLK_RCTRL, SDLK_RETURN,
            SDLK_DOWN,  SDLK_DOWN,  SDLK_LSHIFT, SDLK_LSHIFT, SDLK_LSHIFT, SDLK_ESCAPE};
        key = sound_keys[stage_];
        key_up = stage_ == 1 || stage_ == 5 || stage_ == 6 || stage_ == 8 || stage_ == 11;
    } else if (mode_ == ProbeMode::Shortcut) {
        constexpr int keys14[] = {SDLK_SPACE,  SDLK_SPACE,  SDLK_RCTRL,  SDLK_RETURN,
                                  SDLK_RCTRL,  SDLK_RCTRL,  SDLK_RETURN, SDLK_LSHIFT,
                                  SDLK_LSHIFT, SDLK_LSHIFT, SDLK_ESCAPE};
        key = keys14[stage_];
        key_up = stage_ == 1 || stage_ == 5 || stage_ == 6 || stage_ == 9;
    } else if (mode_ == ProbeMode::Texture) {
        constexpr int end[] = {SDLK_RCTRL, SDLK_RCTRL, SDLK_ESCAPE};
        key =
            stage_ / 2 < texture_count ? texture_keys[stage_ / 2] : end[stage_ / 2 - texture_count];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::FxReset) {
        if (stage_ < 2 * texture_count) {
            key = texture_keys[stage_ / 2];
            key_up = stage_ % 2 == 1;
        } else {
            constexpr int end[] = {SDLK_RCTRL,  SDLK_RCTRL, SDLK_RETURN, SDLK_RETURN,
                                   SDLK_LCTRL,  SDLK_LCTRL, SDLK_RETURN, SDLK_RETURN,
                                   SDLK_UP,     SDLK_UP,    SDLK_LSHIFT, SDLK_LSHIFT,
                                   SDLK_LSHIFT, SDLK_RCTRL, SDLK_RCTRL,  SDLK_ESCAPE};
            const auto i = stage_ - 2 * texture_count;
            key = end[i];
            key_up = i == 1 || i == 3 || i == 5 || i == 7 || i == 9 || i == 12 || i == 14;
        }
    } else if (mode_ == ProbeMode::Harmony) {
        key = harmony_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::PatchFailure) {
        key = failure_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Library) {
        key = library_keys[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Rack) {
        key = rack[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else if (mode_ == ProbeMode::Sample) {
        key = sample[stage_ / 2];
        key_up = stage_ % 2 == 1;
    } else {
        key = keys[stage_];
    }
    // Older regression scripts express destinations in stored-page order. Reach
    // those destinations through real grouped shoulder events, never a UI bypass.
    if (mode_ != ProbeMode::Scene && mode_ != ProbeMode::Drone && mode_ != ProbeMode::Ambient &&
        mode_ != ProbeMode::Pattern && mode_ != ProbeMode::Tools &&
        mode_ != ProbeMode::Performance && (key == SDLK_e || key == SDLK_t)) {
        if (!key_up) {
            const auto destination = (legacy_page_ + (key == SDLK_e ? 17U : 1U)) % 18;
            unsigned cursor = legacy_page_;
            for (unsigned n = 0; n < PageNavigator::destinations && cursor != destination; ++n) {
                SDL_Event step{};
                step.type = SDL_KEYDOWN;
                step.key.keysym.sym = SDLK_t;
                if (SDL_PushEvent(&step) < 0)
                    throw std::runtime_error("probe queue full");
                step.type = SDL_KEYUP;
                if (SDL_PushEvent(&step) < 0)
                    throw std::runtime_error("probe queue full");
                cursor = PageNavigator::adjacent(cursor, 1);
            }
            legacy_page_ = destination;
        }
        ++stage_;
        return;
    }
    SDL_Event event{};
    event.type = key_up ? SDL_KEYUP : SDL_KEYDOWN;
    event.key.keysym.sym = static_cast<decltype(event.key.keysym.sym)>(key);
    if (SDL_PushEvent(&event) < 0)
        throw std::runtime_error("self-test event injection failed");
    ++stage_;
}
} // namespace grainmm
