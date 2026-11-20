// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace grainmm {
struct DroneSynthParameters {
    // A standalone self-playing source. Level0 disables; no WAV or live input.
    float level = 0, root = 60, mood = 1, evolve_seconds = 12, texture = .35F, movement = .6F;
    float event_mix = .25F, event_rate = .4F, glide_seconds = .6F, stereo = 1, seed = 42,
          pattern = 0;
};
DroneSynthParameters bounded(DroneSynthParameters p) noexcept;
struct PreparedDroneSynth {
    DroneSynthParameters values;
    std::array<std::array<double, 4>, 4> steps{};
    double cycle_step = 0;
    float pitch_smoothing = 0;
};
unsigned drone_chord_note(DroneSynthParameters p, unsigned chord, unsigned voice) noexcept;
PreparedDroneSynth prepare_drone_synth(DroneSynthParameters p, int rate);
// Four sustained voices, smooth harmonic changes, slow breathing/pan and quiet
// filtered air. Audio-owned fixed state; preparation/table construction outside render.
class DroneSynth {
  public:
    explicit DroneSynth(int rate, std::uint32_t seed = 0xa178ce35U);
    void apply(const PreparedDroneSynth &p) noexcept;
    void add(float *out, std::size_t frames, int channels, bool playing) noexcept;
    void reset() noexcept;
    bool enabled() const noexcept { return target_.values.level > 0 || gain_ > 0; }
    unsigned visual_state() const noexcept;
    unsigned changes() const noexcept { return changes_; }
    unsigned voice_levels() const noexcept;
    unsigned event_levels() const noexcept;
    unsigned event_count() const noexcept { return event_count_; }

  private:
    float lookup(const std::array<float, 1025> &table, double phase) const noexcept;
    static float noise(std::uint32_t &state) noexcept;
    struct EventVoice {
        double phase = 0, step = 0;
        float pan = 0, age_scale = 0;
        unsigned age = 0, duration = 0;
    };
    void reseed() noexcept;
    void trigger_event() noexcept;
    static unsigned pack_levels(const std::array<float, 4> &) noexcept;
    PreparedDroneSynth target_;
    std::array<EventVoice, 4> event_voices_{};
    std::array<float, 4> voice_peaks_{}, event_peaks_{};
    std::array<float, 1025> sine_{}, rich_{};
    std::array<double, 4> phase_{}, step_{};
    std::array<float, 4> drift_{1, 1, 1, 1}, drift_current_{1, 1, 1, 1};
    int rate_;
    std::uint32_t seed_, events_, air_random_, note_random_ = 1;
    double cycle_ = 0, event_countdown_ = 0;
    float gain_ = 0, texture_ = .35F, movement_ = .6F, air_ = 0, air_slow_ = 0, pitch_smoothing_,
          slew_, air_fast_alpha_, air_slow_alpha_, event_mix_ = .25F, stereo_ = 1;
    unsigned degree_ = 0, changes_ = 0, event_count_ = 0;
    int direction_ = 1;
};
} // namespace grainmm
