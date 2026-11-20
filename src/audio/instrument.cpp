// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument.hpp"
#include <algorithm>
namespace grainmm {
Instrument::Instrument(const Sample &sample, int rate) : Instrument(&sample, rate) {}
Instrument::Instrument(const Sample *sample, int rate)
    : collision_(rate), drone_synth_(rate), oscillator_(rate), oscillator_b_(rate),
      compressor_(rate), tape_(rate), resonator_(rate), rhythm_(rate), crusher_(rate),
      filter_(rate), chorus_(rate), fallback_(rate), delay_(rate), eq_(rate), reverb_(rate),
      output_(rate), rate_(rate), output_step_(1.0F / (0.02F * static_cast<float>(rate))) {
    if (sample) {
        granular_.emplace(*sample, rate);
        slicer_.emplace(*sample, rate);
    }
    clock_.apply(prepare_clock(120, rate));
}
void Instrument::set_parameters(InstrumentParameters parameters) {
    apply(prepare_parameters(parameters, rate_));
}
void Instrument::apply(const PreparedInstrumentParameters &parameters) noexcept {
    if (granular_) {
        granular_->set_parameters(parameters.values.grain);
        granular_->set_drone(parameters.values.drone);
        granular_->set_harmony(parameters.harmony);
    }
    if (slicer_) {
        slicer_->apply(parameters.slice, parameters.values.grain.pitch,
                       parameters.values.grain.gain, parameters.values.grain.start,
                       parameters.values.grain.end);
    }
    slice_enabled_ = parameters.values.slice.mode != 0;
    clock_.apply(parameters.clock);
    collision_.apply(parameters.collision);
    drone_synth_.apply(parameters.drone_synth);
    oscillator_.apply(parameters.oscillator);
    oscillator_b_.apply(parameters.oscillator_b);
    compressor_.apply(parameters.compressor);
    tape_.apply(parameters.tape);
    resonator_.apply(parameters.resonator);
    rhythm_.apply(parameters.rhythm);
    delay_.set_parameters(parameters.values.delay);
    eq_.set_coefficients(parameters.eq);
    reverb_.apply(parameters.reverb);
    crusher_.apply(parameters.crusher);
    filter_.apply(parameters.filter);
    chorus_.apply(parameters.chorus);
    output_.apply(parameters.output);
}
void Instrument::clear_effects() noexcept {
    delay_.reset();
    eq_.reset();
    reverb_.reset();
    compressor_.reset();
    tape_.reset();
    resonator_.reset();
    rhythm_.reset();
    crusher_.reset();
    filter_.reset();
    chorus_.reset();
    output_.reset();
}
void Instrument::render(std::int16_t *out, std::size_t frames, int channels,
                        bool playing) noexcept {
    if (!out || (channels != 1 && channels != 2))
        return;
    float peak_left = 0, peak_right = 0;
    if (playing && !was_playing_)
        clock_.reset();
    was_playing_ = playing;
    if (playing)
        muted_ = false;
    // Small fixed chunks bound scratch storage for any backend block length.
    for (std::size_t at = 0; at < frames;) {
        const auto count = std::min<std::size_t>(256, frames - at);
        if (granular_ && (!slice_enabled_ || source_blend_ < 1))
            granular_->render_float(scratch_.data(), count, channels, playing);
        else
            std::fill_n(scratch_.data(), count * static_cast<unsigned>(channels), 0.0F);
        if (slicer_ && (slice_enabled_ || source_blend_ > 0)) {
            slicer_->render(slice_scratch_.data(), count, channels, playing && slice_enabled_,
                            clock_);
            for (std::size_t f = 0; f < count; ++f) {
                const auto before = source_blend_;
                source_blend_ += std::clamp((slice_enabled_ ? 1.0F : 0.0F) - source_blend_,
                                            -output_step_, output_step_);
                for (unsigned ch = 0; ch < static_cast<unsigned>(channels); ++ch) {
                    const auto i = f * static_cast<unsigned>(channels) + ch;
                    scratch_[i] += source_blend_ * (slice_scratch_[i] - scratch_[i]);
                }
                if (source_blend_ == 1 && before < 1)
                    granular_->reset();
            }
        }
        if (playing)
            clock_.advance(static_cast<unsigned>(count));
        oscillator_.add(scratch_.data(), count, channels, playing);
        oscillator_b_.add(scratch_.data(), count, channels, playing);
        collision_.add(scratch_.data(), count, channels, playing);
        drone_synth_.add(scratch_.data(), count, channels, playing);
        if (playing || output_gain_ > 0) {
            filter_.process(scratch_.data(), count, channels);
            resonator_.process(scratch_.data(), count, channels);
            eq_.process(scratch_.data(), count, channels);
            tape_.process(scratch_.data(), count, channels);
            crusher_.process(scratch_.data(), count, channels);
            chorus_.process(scratch_.data(), count, channels);
            compressor_.process(scratch_.data(), count, channels);
            rhythm_.process(scratch_.data(), count, channels);
            delay_.process(scratch_.data(), count, channels);
            reverb_.process(scratch_.data(), count, channels);
        }
        if (!granular_)
            fallback_.render(tone_scratch_.data(), count, channels,
                             playing && !oscillator_.enabled() && !oscillator_b_.enabled() &&
                                 !collision_.enabled() && !drone_synth_.enabled());
        if (!granular_)
            for (std::size_t i = 0; i < count * static_cast<unsigned>(channels); ++i)
                scratch_[i] += static_cast<float>(tone_scratch_[i]) / 32767.F;
        output_.process(scratch_.data(), count, channels);
        for (std::size_t frame = 0; frame < count; ++frame) {
            output_gain_ +=
                std::clamp((playing ? 1.0F : 0.0F) - output_gain_, -output_step_, output_step_);
            if (!playing && output_gain_ == 0 && !muted_) {
                delay_.reset(); // Clear even if playback resumes immediately next call.
                eq_.reset();
                reverb_.reset();
                oscillator_.reset();
                oscillator_b_.reset();
                compressor_.reset();
                tape_.reset();
                resonator_.reset();
                rhythm_.reset();
                crusher_.reset();
                filter_.reset();
                chorus_.reset();
                muted_ = true;
            }
            for (unsigned channel = 0; channel < static_cast<unsigned>(channels); ++channel) {
                const auto index = frame * static_cast<unsigned>(channels) + channel;
                const float mixed = scratch_[index] * output_gain_;
                if (mixed > 1 || mixed < -1)
                    ++clipped_;
                const float value = std::clamp(mixed, -1.0F, 1.0F);
                (channel == 0 ? peak_left : peak_right) =
                    std::max(channel == 0 ? peak_left : peak_right, std::abs(value));
                out[(at + frame) * static_cast<unsigned>(channels) + channel] =
                    static_cast<std::int16_t>(value * 32767);
            }
        }
        at += count;
    }
    peaks_ = static_cast<unsigned>(peak_left * 65535) |
             (static_cast<unsigned>(peak_right * 65535) << 16);
}
} // namespace grainmm
