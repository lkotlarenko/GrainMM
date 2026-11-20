// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/chorus.hpp"
#include "grainmm/collision.hpp"
#include "grainmm/compressor.hpp"
#include "grainmm/crusher.hpp"
#include "grainmm/delay.hpp"
#include "grainmm/drone_synth.hpp"
#include "grainmm/equalizer.hpp"
#include "grainmm/filter.hpp"
#include "grainmm/granular.hpp"
#include "grainmm/oscillator.hpp"
#include "grainmm/output.hpp"
#include "grainmm/reset_scope.hpp"
#include "grainmm/resonator.hpp"
#include "grainmm/reverb.hpp"
#include "grainmm/rhythm.hpp"
#include "grainmm/slice.hpp"
#include "grainmm/tape.hpp"
#include <algorithm>
#include <cmath>
namespace grainmm {
struct InstrumentParameters {
    GrainParameters grain;
    DelayParameters delay;
    std::array<float, 6> eq_db{};
    ReverbParameters reverb;
    OscillatorParameters oscillator;
    CompressorParameters compressor;
    TapeParameters tape;
    ResonatorParameters resonator;
    RhythmParameters rhythm;
    HarmonyParameters harmony;
    CrusherParameters crusher;
    FilterParameters filter;
    ChorusParameters chorus;
    SliceParameters slice;
    CollisionParameters collision;
    DroneParameters drone;
    DroneSynthParameters drone_synth;
    OutputParameters output;
    SlicePattern slice_pattern;
    OscillatorParameters oscillator_b;
    SynthColorParameters synth_color;
    GrooveParameters groove;
};
inline InstrumentParameters bounded(InstrumentParameters p) noexcept {
    p.grain = bounded(p.grain);
    p.delay = bounded(p.delay);
    p.reverb = bounded(p.reverb);
    p.oscillator = bounded(p.oscillator);
    p.compressor = bounded(p.compressor);
    p.tape = bounded(p.tape);
    p.resonator = bounded(p.resonator);
    p.rhythm = bounded(p.rhythm);
    p.harmony = bounded(p.harmony);
    p.crusher = bounded(p.crusher);
    p.filter = bounded(p.filter);
    p.chorus = bounded(p.chorus);
    p.slice = bounded(p.slice);
    p.collision = bounded(p.collision);
    p.drone = bounded(p.drone);
    p.drone_synth = bounded(p.drone_synth);
    p.output = bounded(p.output);
    p.slice_pattern = bounded(p.slice_pattern);
    p.oscillator_b = bounded(p.oscillator_b);
    p.synth_color = bounded(p.synth_color);
    p.groove = bounded(p.groove);
    for (auto &gain : p.eq_db)
        gain = std::isfinite(gain) ? std::clamp(gain, -20.0F, 20.0F) : 0;
    return p;
}
// Reset the selected sound scope. Sample ownership/path and stored patches are external.
inline InstrumentParameters reset_effects(InstrumentParameters p,
                                          ResetScope scope = ResetScope::Rack) noexcept {
    if (scope == ResetScope::Sound)
        return {};
    if (scope == ResetScope::Modulation) {
        p.grain.reverse = p.grain.pitch_jitter = p.grain.size_jitter = p.grain.timing_jitter = 0;
        p.grain.pan_spread = p.grain.scan_hz = 0;
        p.grain.attack = .2F;
        p.grain.release = .2F;
        p.grain.curve = 0;
        p.grain.width = 1;
        p.harmony = {};
        p.drone = {};
        p.drone_synth.movement = 0;
        p.slice.reverse = p.slice.variation = 0;
        for (auto &step : p.slice_pattern.steps) {
            step.reverse = false;
            step.probability = 1;
        }
        p.collision.rotation = 0;
        p.oscillator.detune_cents = 0;
        p.oscillator.vibrato_cents = 0;
        p.oscillator_b.detune_cents = p.oscillator_b.vibrato_cents = 0;
        p.synth_color.scan_hz = p.synth_color.scan_depth = 0;
        p.groove = {};
    }
    p.delay = {};
    p.eq_db = {};
    p.reverb = {};
    p.compressor = {};
    p.tape = {};
    p.resonator = {};
    const float tempo = p.rhythm.bpm;
    p.rhythm = {};
    p.rhythm.bpm = tempo; // Shared source tempo survives rack/modulation reset.
    p.crusher = {};
    p.filter = {};
    p.chorus = {};
    return p;
}
struct PreparedInstrumentParameters {
    InstrumentParameters values;
    EqCoefficients eq;
    PreparedReverb reverb;
    PreparedOscillator oscillator;
    PreparedCompressor compressor;
    PreparedTape tape;
    PreparedResonator resonator;
    PreparedRhythm rhythm;
    PreparedHarmony harmony;
    PreparedCrusher crusher;
    PreparedFilter filter;
    PreparedChorus chorus;
    PreparedSlice slice;
    PreparedClock clock;
    PreparedCollision collision;
    PreparedDroneSynth drone_synth;
    PreparedOutput output;
    PreparedOscillator oscillator_b;
};
// Call on the control thread, never from the callback.
inline PreparedInstrumentParameters prepare_parameters(InstrumentParameters p, int rate) {
    p = bounded(p);
    auto bands = default_eq_bands();
    for (std::size_t i = 0; i < bands.size(); ++i)
        bands[i].gain_db = p.eq_db[i];
    return {p,
            design_equalizer(bands, rate),
            prepare_reverb(p.reverb, rate),
            prepare_oscillator(p.oscillator, rate, p.synth_color),
            prepare_compressor(p.compressor, rate),
            prepare_tape(p.tape, rate),
            prepare_resonator(p.resonator, rate),
            prepare_rhythm(p.rhythm, rate),
            prepare_harmony(p.harmony),
            prepare_crusher(p.crusher, rate),
            prepare_filter(p.filter, rate),
            prepare_chorus(p.chorus, rate),
            prepare_slice(p.slice, p.rhythm.bpm, rate, p.slice_pattern, p.groove),
            prepare_clock(p.rhythm.bpm, rate),
            prepare_collision(p.collision, rate),
            prepare_drone_synth(p.drone_synth, rate),
            prepare_output(p.output, rate),
            prepare_oscillator(p.oscillator_b, rate)};
}
} // namespace grainmm
