// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/audio_visuals.hpp"
#include "grainmm/instrument_parameters.hpp"
#include <memory>

namespace grainmm {
struct AudioMetrics {
    unsigned callbacks, late, worst_us, grains, dropped, updates;
    unsigned processed_frames, min_frames, max_frames, span_ms;
    unsigned clipped_samples;
};
// Small SDL adapter; Sample (if supplied) must outlive this object.
class SdlAudio {
  public:
    SdlAudio(int frames, const Sample *sample, bool dummy_required);
    ~SdlAudio();
    SdlAudio(const SdlAudio &) = delete;
    SdlAudio &operator=(const SdlAudio &) = delete;
    // Control thread only: pauses callbacks, prepares the replacement before
    // discarding the old renderer, and stays paused. Both samples must outlive
    // the call; the new sample outlives this adapter or its next replacement.
    void replace_sample(const Sample *sample, InstrumentParameters parameters);
    void reset_effects(InstrumentParameters parameters); // UI-owned mute/flush/apply/resume.
    void start();
    void fade_and_pause();
    void set_playing(bool playing) noexcept;
    bool playing() const noexcept;
    bool set_parameters(InstrumentParameters parameters);
    AudioVisuals visuals() const noexcept;
    unsigned sequence_view() const noexcept;
    unsigned slices_triggered() const noexcept;
    AudioMetrics metrics() const noexcept;
    int rate() const noexcept;
    int frames() const noexcept;

  private:
    struct State;
    std::unique_ptr<State> state_;
};
} // namespace grainmm
