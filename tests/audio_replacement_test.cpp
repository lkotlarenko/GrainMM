// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/sdl_audio.hpp"
#include "grainmm/sdl_runtime.hpp"
#include <SDL.h>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <vector>
namespace {
std::atomic<bool> fail_next{false};
void require(bool ok, const char *message) {
    if (!ok)
        throw std::runtime_error(message);
}
std::unique_ptr<grainmm::Sample> source(float value) {
    return std::make_unique<grainmm::Sample>(48000, 1, std::vector<float>(1000, value));
}
} // namespace
void *operator new(std::size_t size) {
    if (fail_next.exchange(false, std::memory_order_relaxed))
        throw std::bad_alloc();
    if (void *p = std::malloc(size ? size : 1))
        return p;
    throw std::bad_alloc();
}
void *operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }
int main() {
    try {
        grainmm::SdlRuntime runtime;
        auto current = source(0.75F);
        grainmm::SdlAudio audio(256, current.get(), true);
        grainmm::InstrumentParameters parameters;
        require(audio.set_parameters(parameters), "initial controls rejected");
        audio.start();
        audio.set_playing(true);
        SDL_Delay(50);
        for (unsigned i = 0; i < 8; ++i) {
            auto next = source(i % 2 ? -0.5F : 0.5F);
            audio.replace_sample(next.get(), parameters);
            current = std::move(next); // Free the prior sample immediately after the handoff.
            require(!audio.playing(), "replacement unexpectedly started audio");
            audio.start();
            audio.set_playing(true);
            SDL_Delay(40);
            require(audio.metrics().grains > 0, "replacement renderer did not run");
        }
        auto next = source(0.2F);
        fail_next.store(true, std::memory_order_relaxed);
        bool rejected = false;
        try {
            audio.replace_sample(next.get(), parameters);
        } catch (const std::bad_alloc &) {
            rejected = true;
        }
        fail_next.store(false, std::memory_order_relaxed);
        require(rejected && !audio.playing(), "allocation failure did not preserve paused state");
        next.reset(); // Failed replacement must not be retained by the callback.
        audio.start();
        audio.set_playing(true);
        SDL_Delay(50);
        require(audio.metrics().grains > 0, "old renderer was lost on allocation failure");
        audio.replace_sample(nullptr, parameters);
        current.reset();
        audio.start();
        audio.set_playing(true);
        SDL_Delay(40);
        require(audio.metrics().grains == 0, "tone fallback retained sample voices");
        audio.fade_and_pause();
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
