// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/parameter_schema.hpp"
#include <limits>
#include <stdexcept>
namespace grainmm {
namespace {
template <auto Group, auto Member> float read(const InstrumentParameters &p) noexcept {
    return (p.*Group).*Member;
}
template <auto Group, auto Member> float &write(InstrumentParameters &p) noexcept {
    return (p.*Group).*Member;
}
template <auto Group, auto Member>
std::pair<float, float> range(const InstrumentParameters &p) noexcept {
    auto lo = p.*Group, hi = lo;
    lo.*Member = -std::numeric_limits<float>::max();
    hi.*Member = std::numeric_limits<float>::max();
    lo = bounded(lo);
    hi = bounded(hi);
    return {lo.*Member, hi.*Member};
}
template <auto Group, auto Member>
constexpr ParameterDefinition field(const char *label, float step,
                                    const char *const *choices = nullptr, std::size_t count = 0,
                                    float display_scale = 1) {
    return {label,   step,  read<Group, Member>, write<Group, Member>,
            choices, count, display_scale,       range<Group, Member>};
}
template <std::size_t Index> float eq_read(const InstrumentParameters &p) noexcept {
    return p.eq_db[Index];
}
template <std::size_t Index> float &eq_write(InstrumentParameters &p) noexcept {
    return p.eq_db[Index];
}
constexpr const char *groove_orders[] = {"FORWARD", "REVERSE", "BOUNCE"};
constexpr const char *waves[] = {"SINE", "TRIANGLE", "SAW", "PULSE"};
constexpr const char *divisions[] = {"HALF", "QUARTER", "EIGHTH", "16TH"};
constexpr const char *shapes[] = {"SINE", "TRIANGLE"};
constexpr const char *chords[] = {"OFF", "OCTAVES", "FIFTH", "MAJOR", "MINOR", "SUS4"};
constexpr const char *orders[] = {"UP", "DOWN", "BOUNCE", "RANDOM"};
constexpr const char *moods[] = {"MINOR",  "MAJOR", "OPEN",    "DARK",
                                 "LYDIAN", "PENTA", "MAJOR 6", "SUSPEND"};
constexpr const char *patterns[] = {"DRIFT", "FORWARD", "REVERSE", "BOUNCE", "HOLD"};
constexpr const char *drone_modes[] = {"OFF", "WAVE", "DRIFT", "BLEND"};
constexpr const char *holds[] = {"RUN", "HOLD"};
constexpr const char *switches[] = {"OFF", "ON"};
constexpr const char *scales[] = {"MINOR", "MAJOR", "PENTA", "CHROMATIC"};
constexpr const char *slice_modes[] = {"OFF", "FORWARD", "REVERSE", "PING-PONG", "SHUFFLE"};
constexpr const char *slice_divisions[] = {"QUARTER", "EIGHTH", "16TH", "32ND"};
constexpr const char *slice_variations[] = {"STRAIGHT", "SPARSE", "ALT REVERSE", "CHANCE"};
constexpr const char *filters[] = {"LOWPASS", "HIGHPASS", "BANDPASS"};
constexpr auto pages = std::array{
    PageDefinition{
        "GRAIN",
        true,
        {{
            field<&InstrumentParameters::grain, &GrainParameters::size_ms>("SIZE MS", 5),
            field<&InstrumentParameters::grain, &GrainParameters::density>("DENSITY HZ", 1),
            field<&InstrumentParameters::grain, &GrainParameters::pitch>("PITCH", 0.05F),
            field<&InstrumentParameters::grain, &GrainParameters::position>("POSITION", 0.02F),
            field<&InstrumentParameters::grain, &GrainParameters::spread>("SPREAD", 0.02F),
            field<&InstrumentParameters::grain, &GrainParameters::gain>("GAIN", 0.01F),
        }}},
    PageDefinition{
        "DELAY",
        false,
        {{
            field<&InstrumentParameters::delay, &DelayParameters::time_ms>("TIME MS", 10),
            field<&InstrumentParameters::delay, &DelayParameters::feedback>("FEEDBACK", 0.05F),
            field<&InstrumentParameters::delay, &DelayParameters::mix>("MIX %", 0.05F, nullptr, 0,
                                                                       100),
            field<&InstrumentParameters::delay, &DelayParameters::ping_pong>("PING PONG", 1),
            field<&InstrumentParameters::delay, &DelayParameters::spread>("SPREAD", 0.05F),
            field<&InstrumentParameters::delay, &DelayParameters::lowpass_hz>("LOWPASS HZ", 500),
        }},
        {0, 1, 3, 4, 5, 2}},
    PageDefinition{"EQ",
                   false,
                   {{
                       ParameterDefinition{"LOW 80 HZ DB", 1, eq_read<0>, eq_write<0>},
                       ParameterDefinition{"LMID 250 HZ DB", 1, eq_read<1>, eq_write<1>},
                       ParameterDefinition{"MID 700 HZ DB", 1, eq_read<2>, eq_write<2>},
                       ParameterDefinition{"HMID 2 KHZ DB", 1, eq_read<3>, eq_write<3>},
                       ParameterDefinition{"HIGH 6 KHZ DB", 1, eq_read<4>, eq_write<4>},
                       ParameterDefinition{"AIR 12 KHZ DB", 1, eq_read<5>, eq_write<5>},
                   }}},
    PageDefinition{
        "REVERB",
        false,
        {{
            field<&InstrumentParameters::reverb, &ReverbParameters::mix>("MIX %", 0.05F, nullptr, 0,
                                                                         100),
            field<&InstrumentParameters::reverb, &ReverbParameters::size>("ROOM SIZE", 0.05F),
            field<&InstrumentParameters::reverb, &ReverbParameters::decay_s>("DECAY SEC", 0.5F),
            field<&InstrumentParameters::reverb, &ReverbParameters::damp_hz>("DAMP HZ", 500),
            field<&InstrumentParameters::reverb, &ReverbParameters::predelay_ms>("PREDELAY MS", 5),
            field<&InstrumentParameters::reverb, &ReverbParameters::width>("WIDTH", 0.05F),
        }},
        {1, 2, 3, 4, 5, 0}},
    PageDefinition{
        "MOTION",
        true,
        {{
            field<&InstrumentParameters::grain, &GrainParameters::reverse>("REVERSE CHANCE", 0.1F),
            field<&InstrumentParameters::grain, &GrainParameters::pitch_jitter>("PITCH RAND ST", 1),
            field<&InstrumentParameters::grain, &GrainParameters::size_jitter>("SIZE RANDOM", 0.1F),
            field<&InstrumentParameters::grain, &GrainParameters::timing_jitter>("TIME RANDOM",
                                                                                 0.1F),
            field<&InstrumentParameters::grain, &GrainParameters::pan_spread>("PAN SPREAD", 0.1F),
            field<&InstrumentParameters::grain, &GrainParameters::scan_hz>("SCAN HZ", 0.05F),
        }}},
    PageDefinition{
        "OSC",
        false,
        {{
            field<&InstrumentParameters::oscillator, &OscillatorParameters::level>("LEVEL", 0.01F),
            field<&InstrumentParameters::oscillator, &OscillatorParameters::note>("MIDI NOTE", 1),
            field<&InstrumentParameters::oscillator, &OscillatorParameters::wave>("WAVE", 1, waves,
                                                                                  4),
            field<&InstrumentParameters::oscillator, &OscillatorParameters::detune_cents>(
                "DETUNE CT", 1),
            field<&InstrumentParameters::oscillator, &OscillatorParameters::pulse_width>(
                "PULSE WIDTH", 0.05F),
            field<&InstrumentParameters::oscillator, &OscillatorParameters::vibrato_cents>(
                "VIBRATO CT", 5),
        }}},
    PageDefinition{
        "COMP",
        false,
        {{
            field<&InstrumentParameters::compressor, &CompressorParameters::threshold_db>(
                "THRESHOLD DB", 1),
            field<&InstrumentParameters::compressor, &CompressorParameters::ratio>("RATIO", 1),
            field<&InstrumentParameters::compressor, &CompressorParameters::attack_ms>("ATTACK MS",
                                                                                       1),
            field<&InstrumentParameters::compressor, &CompressorParameters::release_ms>(
                "RELEASE MS", 10),
            field<&InstrumentParameters::compressor, &CompressorParameters::makeup_db>("MAKEUP DB",
                                                                                       1),
            field<&InstrumentParameters::compressor, &CompressorParameters::mix>("MIX %", 0.05F,
                                                                                 nullptr, 0, 100),
        }}},
    PageDefinition{
        "TAPE",
        false,
        {{
            field<&InstrumentParameters::tape, &TapeParameters::drive_db>("DRIVE DB", 1),
            field<&InstrumentParameters::tape, &TapeParameters::tone_hz>("TONE HZ", 500),
            field<&InstrumentParameters::tape, &TapeParameters::wow_ms>("WOW MS", 0.1F),
            field<&InstrumentParameters::tape, &TapeParameters::flutter_ms>("FLUTTER MS", 0.025F),
            field<&InstrumentParameters::tape, &TapeParameters::mix>("MIX %", 0.05F, nullptr, 0,
                                                                     100),
            field<&InstrumentParameters::tape, &TapeParameters::output_db>("OUTPUT DB", 1),
        }},
        {0, 1, 2, 3, 5, 4}},
    PageDefinition{
        "RESONATE",
        false,
        {{
            field<&InstrumentParameters::resonator, &ResonatorParameters::note>("MIDI NOTE", 1),
            field<&InstrumentParameters::resonator, &ResonatorParameters::decay_s>("DECAY SEC",
                                                                                   0.05F),
            field<&InstrumentParameters::resonator, &ResonatorParameters::brightness>("BRIGHTNESS",
                                                                                      0.05F),
            field<&InstrumentParameters::resonator, &ResonatorParameters::inharmonic>("INHARMONIC",
                                                                                      0.05F),
            field<&InstrumentParameters::resonator, &ResonatorParameters::width>("WIDTH", 0.05F),
            field<&InstrumentParameters::resonator, &ResonatorParameters::mix>("MIX %", 0.05F,
                                                                               nullptr, 0, 100),
        }}},
    PageDefinition{
        "RHYTHM",
        false,
        {{
            field<&InstrumentParameters::rhythm, &RhythmParameters::bpm>("TEMPO BPM", 5),
            field<&InstrumentParameters::rhythm, &RhythmParameters::division>("DIVISION", 1,
                                                                              divisions, 4),
            field<&InstrumentParameters::rhythm, &RhythmParameters::tremolo>("TREMOLO", 0.1F),
            field<&InstrumentParameters::rhythm, &RhythmParameters::pan>("AUTO PAN", 0.1F),
            field<&InstrumentParameters::rhythm, &RhythmParameters::shape>("SHAPE", 1, shapes, 2),
            field<&InstrumentParameters::rhythm, &RhythmParameters::stereo_degrees>("STEREO DEG",
                                                                                    15),
        }}},
    PageDefinition{
        "SAMPLE",
        true,
        {{
            field<&InstrumentParameters::grain, &GrainParameters::start>("START", 0.01F),
            field<&InstrumentParameters::grain, &GrainParameters::end>("END", 0.01F),
            field<&InstrumentParameters::grain, &GrainParameters::attack>("ATTACK", 0.02F),
            field<&InstrumentParameters::grain, &GrainParameters::release>("RELEASE", 0.02F),
            field<&InstrumentParameters::grain, &GrainParameters::curve>("ENV BLEND", 0.1F),
            field<&InstrumentParameters::grain, &GrainParameters::width>("STEREO WIDTH", 0.1F),
        }}},
    PageDefinition{
        "HARMONY",
        true,
        {{
            field<&InstrumentParameters::harmony, &HarmonyParameters::transpose>("TRANSPOSE ST", 1),
            field<&InstrumentParameters::harmony, &HarmonyParameters::chord>("CHORD", 1, chords, 6),
            field<&InstrumentParameters::harmony, &HarmonyParameters::order>("ORDER", 1, orders, 4),
            field<&InstrumentParameters::harmony, &HarmonyParameters::octaves>("OCTAVES", 1),
            field<&InstrumentParameters::harmony, &HarmonyParameters::chance>("CHANCE", 0.1F),
            field<&InstrumentParameters::harmony, &HarmonyParameters::detune_cents>("DETUNE CT", 5),
        }}},
    PageDefinition{
        "CRUSH",
        false,
        {{
            field<&InstrumentParameters::crusher, &CrusherParameters::bits>("BITS", 1),
            field<&InstrumentParameters::crusher, &CrusherParameters::hold>("HOLD FRAMES", 1),
            field<&InstrumentParameters::crusher, &CrusherParameters::drive_db>("DRIVE DB", 1),
            field<&InstrumentParameters::crusher, &CrusherParameters::tone_hz>("TONE HZ", 500),
            field<&InstrumentParameters::crusher, &CrusherParameters::mix>("MIX %", 0.05F, nullptr,
                                                                           0, 100),
            field<&InstrumentParameters::crusher, &CrusherParameters::output_db>("OUTPUT DB", 1),
        }},
        {0, 1, 2, 3, 5, 4}},
    PageDefinition{
        "FILTER",
        false,
        {{
            field<&InstrumentParameters::filter, &FilterParameters::cutoff_hz>("CUTOFF HZ", 100),
            field<&InstrumentParameters::filter, &FilterParameters::resonance>("RESONANCE", 0.25F),
            field<&InstrumentParameters::filter, &FilterParameters::mode>("MODE", 1, filters, 3),
            field<&InstrumentParameters::filter, &FilterParameters::drive_db>("DRIVE DB", 1),
            field<&InstrumentParameters::filter, &FilterParameters::mix>("MIX %", 0.05F, nullptr, 0,
                                                                         100),
            field<&InstrumentParameters::filter, &FilterParameters::output_db>("OUTPUT DB", 1),
        }},
        {0, 1, 2, 3, 5, 4}},
    PageDefinition{
        "CHORUS",
        false,
        {{
            field<&InstrumentParameters::chorus, &ChorusParameters::rate_hz>("RATE HZ", 0.1F),
            field<&InstrumentParameters::chorus, &ChorusParameters::depth_ms>("DEPTH MS", 0.5F),
            field<&InstrumentParameters::chorus, &ChorusParameters::delay_ms>("DELAY MS", 1),
            field<&InstrumentParameters::chorus, &ChorusParameters::feedback>("FEEDBACK", 0.05F),
            field<&InstrumentParameters::chorus, &ChorusParameters::mix>("MIX %", 0.05F, nullptr, 0,
                                                                         100),
            field<&InstrumentParameters::chorus, &ChorusParameters::width>("WIDTH", 0.1F),
        }},
        {0, 1, 2, 3, 5, 4}},
    PageDefinition{
        "SLICE",
        true,
        {{field<&InstrumentParameters::slice, &SliceParameters::mode>("MODE", 1, slice_modes, 5),
          field<&InstrumentParameters::slice, &SliceParameters::regions>("REGIONS", 1),
          field<&InstrumentParameters::slice, &SliceParameters::division>("DIVISION", 1,
                                                                          slice_divisions, 4),
          field<&InstrumentParameters::slice, &SliceParameters::gate>("GATE", .05F),
          field<&InstrumentParameters::slice, &SliceParameters::variation>("VARIATION", 1,
                                                                           slice_variations, 4),
          field<&InstrumentParameters::slice, &SliceParameters::reverse>("REVERSE CHANCE", .1F)}}},

    PageDefinition{
        "BALLS",
        false,
        {{field<&InstrumentParameters::collision, &CollisionParameters::enabled>("MODE", 1,
                                                                                 switches, 2),
          field<&InstrumentParameters::collision, &CollisionParameters::count>("BALLS", 1),
          field<&InstrumentParameters::collision, &CollisionParameters::gravity>("GRAVITY", .05F),
          field<&InstrumentParameters::collision, &CollisionParameters::rotation>("ROTATE", .05F),
          field<&InstrumentParameters::collision, &CollisionParameters::bounce>("BOUNCE", .05F),
          field<&InstrumentParameters::collision, &CollisionParameters::level>("LEVEL", .01F)}}},
    PageDefinition{
        "BOWL",
        false,
        {{field<&InstrumentParameters::collision, &CollisionParameters::note>("MIDI NOTE", 1),
          field<&InstrumentParameters::collision, &CollisionParameters::scale>("SCALE", 1, scales,
                                                                               4),
          field<&InstrumentParameters::collision, &CollisionParameters::octave>("OCTAVE", 1),
          field<&InstrumentParameters::collision, &CollisionParameters::decay>("DECAY SEC", .1F),
          field<&InstrumentParameters::collision, &CollisionParameters::brightness>("BRIGHTNESS",
                                                                                    .05F),
          field<&InstrumentParameters::collision, &CollisionParameters::excite>("EXCITE", .05F)}}},

    PageDefinition{
        "EVOLVE",
        true,
        {{field<&InstrumentParameters::drone, &DroneParameters::mode>("MODE", 1, drone_modes, 4),
          field<&InstrumentParameters::drone, &DroneParameters::rate_hz>("RATE HZ", .01F),
          field<&InstrumentParameters::drone, &DroneParameters::position_depth>("POSITION DEPTH",
                                                                                .05F),
          field<&InstrumentParameters::drone, &DroneParameters::pitch_cents>("PITCH CENTS", 5),
          field<&InstrumentParameters::drone, &DroneParameters::spread_depth>("SPREAD DEPTH", .05F),
          field<&InstrumentParameters::drone, &DroneParameters::hold>("SCAN HOLD", 1, holds, 2)}}},
    PageDefinition{
        "DRONE",
        false,
        {{field<&InstrumentParameters::drone_synth, &DroneSynthParameters::level>("LEVEL %", .05F,
                                                                                  nullptr, 0, 100),
          field<&InstrumentParameters::drone_synth, &DroneSynthParameters::root>("ROOT NOTE", 1),
          field<&InstrumentParameters::drone_synth, &DroneSynthParameters::mood>("MOOD", 1, moods,
                                                                                 8),
          field<&InstrumentParameters::drone_synth, &DroneSynthParameters::evolve_seconds>(
              "EVOLVE SEC", 1),
          field<&InstrumentParameters::drone_synth, &DroneSynthParameters::texture>("TEXTURE",
                                                                                    .05F),
          field<&InstrumentParameters::drone_synth, &DroneSynthParameters::movement>("MOVEMENT",
                                                                                     .05F)}}},

    PageDefinition{
        "GENERATE",
        false,
        {{field<&InstrumentParameters::drone_synth, &DroneSynthParameters::event_mix>("EVENT MIX",
                                                                                      .05F),
          field<&InstrumentParameters::drone_synth, &DroneSynthParameters::event_rate>(
              "EVENT RATE HZ", .05F),
          field<&InstrumentParameters::drone_synth, &DroneSynthParameters::glide_seconds>(
              "GLIDE SEC", .05F),
          field<&InstrumentParameters::drone_synth, &DroneSynthParameters::stereo>("STEREO WIDTH",
                                                                                   .05F),
          field<&InstrumentParameters::drone_synth, &DroneSynthParameters::seed>("SEED", 1),
          field<&InstrumentParameters::drone_synth, &DroneSynthParameters::pattern>("PATTERN", 1,
                                                                                    patterns, 5)}}},

    PageDefinition{
        "OUTPUT",
        false,
        {{field<&InstrumentParameters::output, &OutputParameters::gain_db>("GAIN DB", 1),
          field<&InstrumentParameters::output, &OutputParameters::limiter>("LIMITER", 1, switches,
                                                                           2),
          field<&InstrumentParameters::output, &OutputParameters::ceiling_db>("CEILING DB", 1),
          field<&InstrumentParameters::output, &OutputParameters::release_ms>("RELEASE MS", 10),
          field<&InstrumentParameters::output, &OutputParameters::balance>("BALANCE", .1F),
          field<&InstrumentParameters::output, &OutputParameters::mono>("MONO", 1, switches, 2)}}},

    PageDefinition{
        "OSC B",
        false,
        {{
            field<&InstrumentParameters::oscillator_b, &OscillatorParameters::level>(
                "LEVEL %", .01F, nullptr, 0, 100),
            field<&InstrumentParameters::oscillator_b, &OscillatorParameters::note>("MIDI NOTE", 1),
            field<&InstrumentParameters::oscillator_b, &OscillatorParameters::wave>("WAVE", 1,
                                                                                    waves, 4),
            field<&InstrumentParameters::oscillator_b, &OscillatorParameters::detune_cents>(
                "DETUNE CT", 1),
            field<&InstrumentParameters::oscillator_b, &OscillatorParameters::pulse_width>(
                "PULSE WIDTH", .05F),
            field<&InstrumentParameters::oscillator_b, &OscillatorParameters::vibrato_cents>(
                "VIBRATO CT", 5),
        }}},

    PageDefinition{"TIMBRE",
                   false,
                   {{
                       field<&InstrumentParameters::synth_color, &SynthColorParameters::table_mix>(
                           "TABLE MIX %", .05F, nullptr, 0, 100),
                       field<&InstrumentParameters::synth_color, &SynthColorParameters::position>(
                           "TABLE POSITION", .1F),
                       field<&InstrumentParameters::synth_color, &SynthColorParameters::scan_hz>(
                           "SCAN HZ", .05F),
                       field<&InstrumentParameters::synth_color, &SynthColorParameters::scan_depth>(
                           "SCAN DEPTH %", .05F, nullptr, 0, 100),
                       field<&InstrumentParameters::synth_color, &SynthColorParameters::sub_level>(
                           "SUB LEVEL %", .01F, nullptr, 0, 100),
                       field<&InstrumentParameters::synth_color, &SynthColorParameters::sub_octave>(
                           "SUB OCTAVES", 1),
                   }}},

    PageDefinition{"GROOVE",
                   true,
                   {{
                       field<&InstrumentParameters::groove, &GrooveParameters::swing>(
                           "SWING %", .05F, nullptr, 0, 100),
                       field<&InstrumentParameters::groove, &GrooveParameters::length>("STEPS", 1),
                       field<&InstrumentParameters::groove, &GrooveParameters::rotate>("ROTATE", 1),
                       field<&InstrumentParameters::groove, &GrooveParameters::order>(
                           "STEP ORDER", 1, groove_orders, 3),
                       field<&InstrumentParameters::groove, &GrooveParameters::accent>(
                           "ACCENT %", .05F, nullptr, 0, 100),
                       field<&InstrumentParameters::groove, &GrooveParameters::accent_every>(
                           "ACCENT EVERY", 1),
                   }}},

};
constexpr bool valid_display_orders() {
    for (const auto &page : pages) {
        std::array<bool, 6> seen{};
        for (const auto index : page.display_order) {
            if (index >= seen.size() || seen[index])
                return false;
            seen[index] = true;
        }
    }
    return true;
}
static_assert(valid_display_orders(), "display order must include every field exactly once");
static_assert(pages.size() == parameter_page_count, "page schema count mismatch");
} // namespace
const std::array<PageDefinition, parameter_page_count> &parameter_pages() noexcept { return pages; }
const ParameterDefinition &parameter_definition(std::size_t i) {
    if (i >= parameter_count)
        throw std::out_of_range("parameter index outside schema");
    return pages[i / 6].fields[i % 6];
}
std::size_t parameter_count_for_version(int version) {
    constexpr std::array<std::size_t, 19> counts{0,  6,  12,  12,  24,  30,  42,  48,  66, 72,
                                                 90, 96, 108, 114, 120, 126, 132, 132, 150};
    static_assert(counts.back() == parameter_count,
                  "new parameters need an explicit schema version");
    if (version < 1 || version > 18)
        throw std::runtime_error("unsupported preset schema");
    return counts[static_cast<std::size_t>(version)];
}
} // namespace grainmm
