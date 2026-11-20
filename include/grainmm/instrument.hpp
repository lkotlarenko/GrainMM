// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/audio_visuals.hpp"
#include "grainmm/instrument_parameters.hpp"
#include "grainmm/tone.hpp"
#include <optional>
namespace grainmm {
// Portable audio graph, independent of SDL/UI. Sample outlives the instrument.
// Construction allocates delay storage; all subsequent operations are bounded,
// allocation-free, and called by one audio owner.
class Instrument {
  public:
    Instrument(const Sample &sample, int rate);
    Instrument(const Sample *sample, int rate);
    Instrument(const Instrument &) = delete;
    Instrument &operator=(const Instrument &) = delete;
    // Control/offline convenience; prepares coefficients outside audio processing.
    void set_parameters(InstrumentParameters parameters);
    void apply(const PreparedInstrumentParameters &parameters) noexcept;
    void clear_effects() noexcept; // Call with callback ownership held, outside rendering.
    void render(std::int16_t *out, std::size_t frames, int channels, bool playing) noexcept;
    AudioVisuals visuals() const noexcept {
        return {peaks_,
                granular_ ? granular_->visual_state() : 0,
                collision_.hits(),
                collision_.positions(),
                drone_synth_.visual_state(),
                drone_synth_.voice_levels(),
                drone_synth_.event_levels(),
                drone_synth_.event_count(),
                output_.reduction()};
    }
    unsigned clipped_samples() const noexcept { return clipped_; }
    unsigned spawned() const noexcept { return granular_ ? granular_->spawned() : 0; }
    std::uint32_t sequence_view() const noexcept {
        return slicer_ && slice_enabled_ ? slicer_->view() : 0;
    }
    unsigned slices_triggered() const noexcept { return slicer_ ? slicer_->triggered() : 0; }
    unsigned dropped() const noexcept { return granular_ ? granular_->dropped() : 0; }
    std::size_t reverb_storage_bytes() const noexcept { return reverb_.storage_bytes(); }
    std::size_t delay_storage_bytes() const noexcept { return delay_.storage_bytes(); }

  private:
    std::optional<Granular> granular_;
    std::optional<SlicePlayer> slicer_;
    FrameClock clock_;
    bool slice_enabled_ = false, was_playing_ = false;
    float source_blend_ = 0;
    std::array<float, 512> slice_scratch_{};
    CollisionSynth collision_;
    DroneSynth drone_synth_;
    unsigned peaks_ = 0;
    Oscillator oscillator_, oscillator_b_;
    Compressor compressor_;
    Tape tape_;
    Resonator resonator_;
    Rhythm rhythm_;
    Crusher crusher_;
    Filter filter_;
    Chorus chorus_;
    Tone fallback_;
    Delay delay_;
    Equalizer eq_;
    Reverb reverb_;
    Output output_;
    int rate_;
    std::array<float, 512> scratch_{};
    std::array<std::int16_t, 512> tone_scratch_{};
    float output_gain_ = 0, output_step_;
    bool muted_ = true;
    unsigned clipped_ = 0;
};
} // namespace grainmm
