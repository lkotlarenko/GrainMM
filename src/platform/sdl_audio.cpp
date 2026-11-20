// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/sdl_audio.hpp"
#include "grainmm/control_queue.hpp"
#include "grainmm/instrument.hpp"
#include <SDL.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace grainmm {
struct SdlAudio::State {
    std::unique_ptr<Instrument> instrument;
    ControlQueue<PreparedInstrumentParameters> commands;
    std::atomic<unsigned> audible{0}, callbacks{0}, late{0}, worst_us{0}, grains{0}, dropped{0},
        updates{0}, clipped{0}, sequence{0}, slices{0};
    std::atomic<unsigned> processed_frames{0}, min_frames{0}, max_frames{0}, span_ms{0};
    std::atomic<unsigned> peaks{0}, grain_view{0}, hits{0}, drone{0}, drone_voices{0},
        drone_events{0}, drone_count{0}, output_reduction{0};
    std::array<std::atomic<unsigned>, 6> balls{};
    unsigned previous_budget_us = 0;
    std::chrono::steady_clock::time_point first{};
    SDL_AudioSpec format{};
    unsigned budget_us = 0;
    bool open = false, running = false;
    std::chrono::steady_clock::time_point previous{};
    ~State() {
        if (open) {
            SDL_PauseAudio(1);
            SDL_CloseAudio(); // Joins callback before owned engines/control state are destroyed.
        }
    }
    static void callback(void *user, Uint8 *stream, int length) noexcept {
        auto &state = *static_cast<State *>(user);
        const auto begin = std::chrono::steady_clock::now();
        if (state.previous.time_since_epoch().count() != 0) {
            const auto gap =
                std::chrono::duration_cast<std::chrono::microseconds>(begin - state.previous)
                    .count();
            if (gap > static_cast<long long>(state.previous_budget_us) * 2)
                state.late.fetch_add(1, std::memory_order_relaxed);
        }
        state.previous = begin;
        if (length <= 0)
            return;
        std::memset(stream, 0, static_cast<std::size_t>(length));
        const auto frames =
            static_cast<std::size_t>(length) / (sizeof(std::int16_t) * state.format.channels);
        const auto count = static_cast<unsigned>(frames);
        state.previous_budget_us =
            static_cast<unsigned>(1000000ULL * frames / static_cast<unsigned>(state.format.freq));
        state.processed_frames.fetch_add(count, std::memory_order_relaxed);
        const auto minimum = state.min_frames.load(std::memory_order_relaxed);
        if (minimum == 0 || count < minimum)
            state.min_frames.store(count, std::memory_order_relaxed);
        if (count > state.max_frames.load(std::memory_order_relaxed))
            state.max_frames.store(count, std::memory_order_relaxed);
        if (state.first.time_since_epoch().count() == 0)
            state.first = begin;
        state.span_ms.store(
            static_cast<unsigned>(
                std::chrono::duration_cast<std::chrono::milliseconds>(begin - state.first).count()),
            std::memory_order_relaxed);
        const bool playing = state.audible.load(std::memory_order_relaxed) != 0;
        auto *out = reinterpret_cast<std::int16_t *>(stream);
        {
            PreparedInstrumentParameters parameters;
            // Producer cannot extend callback work indefinitely.
            for (unsigned i = 0; i < 7 && state.commands.pop(parameters); ++i) {
                state.instrument->apply(parameters);
                state.updates.fetch_add(1, std::memory_order_relaxed);
            }
            state.instrument->render(out, frames, state.format.channels, playing);
            const auto visual = state.instrument->visuals();
            state.peaks.store(visual.peaks, std::memory_order_relaxed);
            state.grain_view.store(visual.grain, std::memory_order_relaxed);
            state.hits.store(visual.hits, std::memory_order_relaxed);
            state.drone.store(visual.drone, std::memory_order_relaxed);
            state.drone_voices.store(visual.drone_voices, std::memory_order_relaxed);
            state.drone_events.store(visual.drone_events, std::memory_order_relaxed);
            state.drone_count.store(visual.drone_count, std::memory_order_relaxed);
            state.output_reduction.store(visual.output_reduction, std::memory_order_relaxed);
            for (unsigned i = 0; i < 6; ++i)
                state.balls[i].store(visual.balls[i], std::memory_order_relaxed);
            state.sequence.store(state.instrument->sequence_view(), std::memory_order_relaxed);
            state.slices.store(state.instrument->slices_triggered(), std::memory_order_relaxed);
            state.grains.store(state.instrument->spawned(), std::memory_order_relaxed);
            state.dropped.store(state.instrument->dropped(), std::memory_order_relaxed);
            state.clipped.store(state.instrument->clipped_samples(), std::memory_order_relaxed);
        }
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                                 std::chrono::steady_clock::now() - begin)
                                 .count();
        const auto duration = static_cast<unsigned>(std::min<long long>(elapsed, 0xffffffffLL));
        if (duration > state.worst_us.load(std::memory_order_relaxed))
            state.worst_us.store(duration, std::memory_order_relaxed);
        state.callbacks.fetch_add(1, std::memory_order_relaxed);
    }
};
SdlAudio::SdlAudio(int frames, const Sample *sample, bool dummy_required)
    : state_(std::make_unique<State>()) {
    if (frames != 256 && frames != 512 && frames != 1024)
        throw std::invalid_argument("audio buffer must be 256, 512 or 1024 frames");
    SDL_AudioSpec wanted{};
    wanted.freq = 48000;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 2;
    wanted.samples = static_cast<Uint16>(frames);
    wanted.callback = State::callback;
    wanted.userdata = state_.get();
    // Opens paused. The State destructor also closes on constructor failure.
    if (SDL_OpenAudio(&wanted, &state_->format) != 0)
        throw std::runtime_error(std::string("audio open: ") + SDL_GetError());
    state_->open = true;
    const auto &format = state_->format;
    if (format.format != AUDIO_S16SYS || (format.channels != 1 && format.channels != 2) ||
        format.samples == 0 || format.samples > 4096)
        throw std::runtime_error("audio backend returned unsupported PCM format");
    state_->instrument = std::make_unique<Instrument>(sample, format.freq);
    state_->budget_us =
        static_cast<unsigned>(1000000ULL * format.samples / static_cast<unsigned>(format.freq));
#ifdef GRAINMM_SDL1
    char name[128]{};
    const char *driver = SDL_AudioDriverName(name, sizeof(name));
#else
    const char *driver = SDL_GetCurrentAudioDriver();
#endif
    if (dummy_required && (!driver || std::strcmp(driver, "dummy") != 0))
        throw std::runtime_error("self-test is restricted to dummy audio");
    std::printf("GrainMM experimental; GrainBrick by ETRNLIAM\n"
                "audio=%s rate=%d channels=%u frames=%u deadline_us=%u XRUN=unavailable mode=%s\n",
                driver ? driver : "unknown", format.freq, format.channels, format.samples,
                state_->budget_us, sample ? "granular" : "tone");
}
SdlAudio::~SdlAudio() = default;
void SdlAudio::replace_sample(const Sample *sample, InstrumentParameters parameters) {
    fade_and_pause();
    // Construct first: allocation failure leaves the previous renderer/sample
    // untouched and paused. No callback can observe partially changed ownership.
    auto instrument = std::make_unique<Instrument>(sample, state_->format.freq);
    instrument->set_parameters(parameters);
    SDL_LockAudio(); // SDL1.2 PauseAudio alone does not join an in-flight callback.
    state_->instrument.swap(instrument);
    PreparedInstrumentParameters discarded;
    // Synchronous callback pause transfers queue consumer ownership temporarily.
    for (unsigned i = 0; i < 7 && state_->commands.pop(discarded); ++i) {
    }
    state_->previous = {};
    state_->grains.store(0, std::memory_order_relaxed);
    state_->dropped.store(0, std::memory_order_relaxed);
    state_->clipped.store(0, std::memory_order_relaxed);
    SDL_UnlockAudio();
    std::puts(sample ? "source switched: sample; paused" : "source switched: tone; paused");
}
void SdlAudio::reset_effects(InstrumentParameters parameters) {
    const auto prepared =
        prepare_parameters(grainmm::reset_effects(parameters), state_->format.freq);
    const bool resume = playing();
    fade_and_pause();
    SDL_LockAudio();
    PreparedInstrumentParameters discarded;
    for (unsigned i = 0; i < 7 && state_->commands.pop(discarded); ++i) {
    }
    state_->instrument->apply(prepared);
    state_->instrument->clear_effects();
    state_->previous = {};
    SDL_UnlockAudio();
    start();
    set_playing(resume);
}
void SdlAudio::start() {
    state_->running = true;
    SDL_PauseAudio(0);
}
void SdlAudio::fade_and_pause() {
    set_playing(false);
    if (!state_->running)
        return;
    SDL_Delay(20U + (2U * state_->budget_us + 999U) / 1000U);
    SDL_PauseAudio(1);
    SDL_LockAudio();
    state_->previous = {}; // Browser/decoding pause is not callback scheduling lateness.
    SDL_UnlockAudio();     // Coherent callback counters and safe paused-state ownership.
    state_->running = false;
}
void SdlAudio::set_playing(bool playing) noexcept {
    state_->audible.store(playing ? 1U : 0U, std::memory_order_relaxed);
}
bool SdlAudio::playing() const noexcept {
    return state_->audible.load(std::memory_order_relaxed) != 0;
}
bool SdlAudio::set_parameters(InstrumentParameters parameters) {
    return state_->commands.push(prepare_parameters(parameters, state_->format.freq));
}
AudioMetrics SdlAudio::metrics() const noexcept {
    return {state_->callbacks.load(std::memory_order_relaxed),
            state_->late.load(std::memory_order_relaxed),
            state_->worst_us.load(std::memory_order_relaxed),
            state_->grains.load(std::memory_order_relaxed),
            state_->dropped.load(std::memory_order_relaxed),
            state_->updates.load(std::memory_order_relaxed),
            state_->processed_frames.load(std::memory_order_relaxed),
            state_->min_frames.load(std::memory_order_relaxed),
            state_->max_frames.load(std::memory_order_relaxed),
            state_->span_ms.load(std::memory_order_relaxed),
            state_->clipped.load(std::memory_order_relaxed)};
}
int SdlAudio::rate() const noexcept { return state_->format.freq; }
int SdlAudio::frames() const noexcept { return state_->format.samples; }
} // namespace grainmm

unsigned grainmm::SdlAudio::sequence_view() const noexcept {
    return state_->sequence.load(std::memory_order_relaxed);
}
unsigned grainmm::SdlAudio::slices_triggered() const noexcept {
    return state_->slices.load(std::memory_order_relaxed);
}

grainmm::AudioVisuals grainmm::SdlAudio::visuals() const noexcept {
    AudioVisuals out;
    out.peaks = state_->peaks.load(std::memory_order_relaxed);
    out.grain = state_->grain_view.load(std::memory_order_relaxed);
    out.hits = state_->hits.load(std::memory_order_relaxed);
    out.drone = state_->drone.load(std::memory_order_relaxed);
    out.drone_voices = state_->drone_voices.load(std::memory_order_relaxed);
    out.drone_events = state_->drone_events.load(std::memory_order_relaxed);
    out.drone_count = state_->drone_count.load(std::memory_order_relaxed);
    out.output_reduction = state_->output_reduction.load(std::memory_order_relaxed);
    for (unsigned i = 0; i < 6; ++i)
        out.balls[i] = state_->balls[i].load(std::memory_order_relaxed);
    return out;
}
