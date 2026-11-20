// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/drone_synth.hpp"
#include "grainmm/dsp_utils.hpp"
namespace grainmm {
DroneSynthParameters bounded(DroneSynthParameters p) noexcept {
    p.level = dsp::limit(p.level, 0, 1);
    p.root = std::round(dsp::limit(p.root, 36, 84, 60));
    p.mood = std::round(dsp::limit(p.mood, 0, 7, 1));
    p.evolve_seconds = dsp::limit(p.evolve_seconds, 4, 60, 12);
    p.texture = dsp::limit(p.texture, 0, 1, .35F);
    p.movement = dsp::limit(p.movement, 0, 1, .6F);
    p.event_mix = dsp::limit(p.event_mix, 0, 1, .25F);
    p.event_rate = dsp::limit(p.event_rate, .05F, 2, .4F);
    p.glide_seconds = dsp::limit(p.glide_seconds, .05F, 3, .6F);
    p.stereo = dsp::limit(p.stereo, 0, 1, 1);
    p.seed = std::round(dsp::limit(p.seed, 1, 9999, 42));
    p.pattern = std::round(dsp::limit(p.pattern, 0, 4));
    return p;
}
unsigned drone_chord_note(DroneSynthParameters p, unsigned chord, unsigned voice) noexcept {
    p = bounded(p);
    constexpr int chords[8][4] = {{0, 3, 7, 12}, {0, 4, 7, 12}, {0, 7, 12, 19}, {0, 1, 6, 13},
                                  {0, 4, 7, 18}, {0, 7, 9, 14}, {0, 4, 7, 9},   {0, 5, 7, 12}};
    constexpr int roots[8][4] = {{0, 5, 3, 7}, {0, 4, 5, 2}, {0, 7, 5, 2}, {0, 1, 5, 6},
                                 {0, 2, 7, 4}, {0, 2, 7, 9}, {0, 5, 7, 2}, {0, 5, 7, 10}};
    const auto mood = static_cast<unsigned>(p.mood);
    return static_cast<unsigned>(p.root) +
           static_cast<unsigned>(roots[mood][chord % 4] + chords[mood][voice % 4]);
}
PreparedDroneSynth prepare_drone_synth(DroneSynthParameters p, int rate) {
    dsp::checked_rate(rate);
    p = bounded(p);
    PreparedDroneSynth result;
    result.values = p;
    for (unsigned chord = 0; chord < 4; ++chord)
        for (unsigned voice = 0; voice < 4; ++voice) {
            const auto note = static_cast<double>(drone_chord_note(p, chord, voice));
            // Tables have three partials; bound fundamental below Nyquist/3.
            result.steps[chord][voice] =
                std::clamp(440 * std::exp2((note - 69) / 12.0) / rate, .000001, .14);
        }
    result.pitch_smoothing = 1 - std::exp(-1.F / (p.glide_seconds * static_cast<float>(rate)));
    result.cycle_step = 1.0 / (p.evolve_seconds * static_cast<double>(rate));
    return result;
}
DroneSynth::DroneSynth(int rate, std::uint32_t seed)
    : target_(prepare_drone_synth({}, rate)), rate_(rate), seed_(seed ? seed : 1), events_(seed_),
      air_random_(seed_ ^ 0x6d2b79f5U),
      pitch_smoothing_(1 - std::exp(-1.F / (.6F * static_cast<float>(rate)))),
      slew_(1.F / (.03F * static_cast<float>(rate))),
      air_fast_alpha_(static_cast<float>(1 - std::exp(-6.283185307179586 * 2200 / rate))),
      air_slow_alpha_(static_cast<float>(1 - std::exp(-6.283185307179586 * 180 / rate))) {
    for (unsigned i = 0; i <= 1024; ++i) {
        const double angle = 6.283185307179586 * i / 1024;
        sine_[i] = static_cast<float>(std::sin(angle));
        rich_[i] = static_cast<float>(.7 * std::sin(angle) + .2 * std::sin(2 * angle) +
                                      .1 * std::sin(3 * angle));
    }
    sine_.back() = sine_.front();
    rich_.back() = rich_.front();
    reset();
}
void DroneSynth::reset() noexcept {
    phase_ = {};
    step_ = target_.steps[0];
    drift_ = drift_current_ = {1, 1, 1, 1};
    cycle_ = gain_ = air_ = air_slow_ = 0;
    texture_ = target_.values.texture;
    movement_ = target_.values.movement;
    degree_ = changes_ = event_count_ = 0;
    event_voices_ = {};
    event_countdown_ = rate_ * .25;
    event_mix_ = target_.values.event_mix;
    stereo_ = target_.values.stereo;
    direction_ = 1;
    reseed();
}
void DroneSynth::apply(const PreparedDroneSynth &p) noexcept {
    const auto previous_seed = target_.values.seed;
    const auto previous_rate = target_.values.event_rate;
    target_ = p;
    target_.values = bounded(p.values);
    if (target_.values.seed != previous_seed)
        reseed();
    event_countdown_ *= previous_rate / target_.values.event_rate;
    pitch_smoothing_ =
        std::isfinite(p.pitch_smoothing) ? std::clamp(p.pitch_smoothing, .000001F, .01F) : .0001F;
    for (auto &chord : target_.steps)
        for (auto &step : chord) {
            if (!std::isfinite(step) || step <= 0) {
                target_.values.level = 0;
                step = .001;
            } else
                step = std::clamp(step, .000001, .14);
        }
    target_.cycle_step = std::isfinite(p.cycle_step)
                             ? std::clamp(p.cycle_step, 1.0 / (60 * rate_), 1.0 / (4 * rate_))
                             : 1.0 / (12 * rate_);
    if (gain_ == 0) {
        step_ = target_.steps[degree_];
        texture_ = target_.values.texture;
        movement_ = target_.values.movement;
        event_mix_ = target_.values.event_mix;
        stereo_ = target_.values.stereo;
    }
}
void DroneSynth::reseed() noexcept {
    auto effective =
        seed_ ^ ((static_cast<std::uint32_t>(target_.values.seed) - 42U) * 0x9e3779b9U);
    if (!effective)
        effective = 1;
    events_ = effective;
    air_random_ = effective ^ 0x6d2b79f5U;
    note_random_ = effective ^ 0xa24baed5U;
    if (!air_random_)
        air_random_ = 1;
    if (!note_random_)
        note_random_ = 1;
}
void DroneSynth::trigger_event() noexcept {
    for (auto &v : event_voices_)
        if (v.age >= v.duration) {
            noise(note_random_);
            const unsigned note = note_random_ % 4;
            v = {};
            v.step = std::min(.14, target_.steps[degree_][note] * 2);
            v.pan = noise(note_random_) * .8F;
            const float duration = .25F + .325F * (noise(note_random_) + 1);
            v.duration = static_cast<unsigned>(duration * static_cast<float>(rate_));
            v.age_scale = 1.F / static_cast<float>(v.duration);
            ++event_count_;
            break;
        }
    event_countdown_ +=
        static_cast<double>(rate_) / target_.values.event_rate * (1 + noise(note_random_) * .5);
}
float DroneSynth::noise(std::uint32_t &state) noexcept {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return static_cast<float>(state >> 8) / 8388607.5F - 1;
}
float DroneSynth::lookup(const std::array<float, 1025> &table, double phase) const noexcept {
    const auto at = phase * 1024;
    const auto i = static_cast<unsigned>(at);
    return table[i] + (table[i + 1] - table[i]) * static_cast<float>(at - i);
}
void DroneSynth::add(float *out, std::size_t frames, int channels, bool playing) noexcept {
    if (!out || (channels != 1 && channels != 2))
        return;
    const float goal = playing ? target_.values.level : 0;
    if (goal == 0 && gain_ == 0)
        return;
    voice_peaks_ = {};
    event_peaks_ = {};
    constexpr float weights[4] = {.3F, .25F, .25F, .2F};
    for (std::size_t f = 0; f < frames; ++f) {
        gain_ += std::clamp(goal - gain_, -.2F * slew_, .2F * slew_);
        texture_ += std::clamp(target_.values.texture - texture_, -slew_, slew_);
        movement_ += std::clamp(target_.values.movement - movement_, -slew_, slew_);
        event_mix_ += std::clamp(target_.values.event_mix - event_mix_, -slew_, slew_);
        stereo_ += std::clamp(target_.values.stereo - stereo_, -slew_, slew_);
        if (playing) {
            cycle_ += target_.cycle_step;
            if (cycle_ >= 1) {
                cycle_ -= 1;
                const auto before = degree_;
                switch (static_cast<unsigned>(target_.values.pattern)) {
                case 0:
                    noise(events_);
                    degree_ = (degree_ + 1 + events_ % 3) % 4;
                    break;
                case 1:
                    degree_ = (degree_ + 1) % 4;
                    break;
                case 2:
                    degree_ = (degree_ + 3) % 4;
                    break;
                case 3:
                    if (degree_ == 3)
                        direction_ = -1;
                    if (degree_ == 0)
                        direction_ = 1;
                    degree_ = static_cast<unsigned>(static_cast<int>(degree_) + direction_);
                    break;
                default:
                    break;
                }
                if (degree_ != before)
                    ++changes_;
                for (auto &weight : drift_)
                    weight = .8F + .2F * noise(events_);
            }
            if (target_.values.event_mix > 0) {
                event_countdown_ -= 1;
                if (event_countdown_ <= 0)
                    trigger_event();
            }
        }
        float left = 0, right = 0;
        for (unsigned i = 0; i < 4; ++i) {
            double lfo_phase = cycle_ + i * .25;
            if (lfo_phase >= 1)
                lfo_phase -= 1;
            const float lfo = lookup(sine_, lfo_phase);
            step_[i] += pitch_smoothing_ * (target_.steps[degree_][i] - step_[i]);
            phase_[i] += step_[i] * (1 + movement_ * .002F * lfo);
            if (phase_[i] >= 1)
                phase_[i] -= 1;
            const float fundamental = lookup(sine_, phase_[i]);
            const float wave = fundamental + texture_ * (lookup(rich_, phase_[i]) - fundamental);
            const float breath = 1 + movement_ * .3F * lfo;
            drift_current_[i] += pitch_smoothing_ * (drift_[i] - drift_current_[i]);
            const float energy =
                wave * weights[i] * breath * (1 + movement_ * (drift_current_[i] - 1));
            voice_peaks_[i] =
                std::max(voice_peaks_[i], std::abs(energy * .6F * gain_ * (1 - .5F * event_mix_)));
            const float pan =
                (static_cast<float>(i) * .4F - .6F + movement_ * .15F * lfo) * stereo_;
            left += energy * (1 - std::max(pan, 0.F));
            right += energy * (1 + std::min(pan, 0.F));
        }
        const auto white = noise(air_random_);
        air_ += air_fast_alpha_ * (white - air_);
        air_slow_ += air_slow_alpha_ * (white - air_slow_);
        const float air = (air_ - air_slow_) * texture_ * .08F;
        left = (left * .6F + air) * gain_ * (1 - .5F * event_mix_);
        right = (right * .6F + air) * gain_ * (1 - .5F * event_mix_);
        for (unsigned i = 0; i < event_voices_.size(); ++i) {
            auto &v = event_voices_[i];
            if (v.age >= v.duration)
                continue;
            const float release = 1 - static_cast<float>(v.age) * v.age_scale;
            const float attack =
                std::min(1.F, static_cast<float>(v.age) / (.008F * static_cast<float>(rate_)));
            v.phase += v.step;
            if (v.phase >= 1)
                v.phase -= 1;
            const float signal =
                lookup(rich_, v.phase) * attack * release * release * .1F * event_mix_ * gain_;
            const float pan = v.pan * stereo_;
            left += signal * (1 - std::max(pan, 0.F));
            right += signal * (1 + std::min(pan, 0.F));
            event_peaks_[i] = std::max(event_peaks_[i], std::abs(signal));
            ++v.age;
        }
        if (channels == 1)
            out[f] += (left + right) * .5F;
        else {
            out[f * 2] += left;
            out[f * 2 + 1] += right;
        }
        if (goal == 0 && gain_ == 0)
            return;
    }
}
unsigned DroneSynth::visual_state() const noexcept {
    if (!enabled())
        return 0;
    return (1U << 24) | degree_ | (static_cast<unsigned>(cycle_ * 65535) << 8);
}
} // namespace grainmm

unsigned grainmm::DroneSynth::pack_levels(const std::array<float, 4> &levels) noexcept {
    unsigned packed = 0;
    for (unsigned i = 0; i < 4; ++i)
        packed |= static_cast<unsigned>(std::clamp(levels[i] * 2048.F, 0.F, 255.F)) << (8 * i);
    return packed;
}
unsigned grainmm::DroneSynth::voice_levels() const noexcept {
    return gain_ > 0 ? pack_levels(voice_peaks_) : 0;
}
unsigned grainmm::DroneSynth::event_levels() const noexcept {
    return gain_ > 0 ? pack_levels(event_peaks_) : 0;
}
