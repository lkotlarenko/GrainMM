// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/drone_scenes.hpp"
#include "grainmm/instrument.hpp"
#include "grainmm/preset.hpp"
#include <cstdio>
#include <stdexcept>
int main() {
    using namespace grainmm;
    InstrumentParameters current;
    current.drone_synth.level = .6F;
    current.output.gain_db = 6;
    current.output.limiter = 1;
    current.grain.pitch = 1.5F;
    current.oscillator.level = .05F;
    current.reverb.mix = .4F;
    for (unsigned i = 0; i < drone_scenes().size(); ++i) {
        auto changed = apply_drone_scene(current, i, 314);
        if (changed.drone_synth.level != .6F || changed.drone_synth.seed != 314 ||
            changed.output.gain_db != 6 || changed.output.limiter != 1 ||
            changed.grain.pitch != 1.5F || changed.oscillator.level != .05F ||
            changed.reverb.mix != .4F)
            return 1;
        auto expected = current;
        expected.drone_synth = changed.drone_synth;
        if (encode_preset({changed, SampleSource::File, "owner.wav"}) !=
            encode_preset({expected, SampleSource::File, "owner.wav"}))
            return 1;
        auto encoded = encode_preset({changed, SampleSource::Tone, {}});
        if (encode_preset(decode_preset(encoded)) != encoded)
            return 1;
        Instrument sound(nullptr, 8000);
        sound.set_parameters(changed);
        std::int16_t work[512]{};
        for (unsigned block = 0; block < 80; ++block)
            sound.render(work, 256, 2, true);
        if (sound.clipped_samples() != 0)
            return 1;
    }
    for (auto index : {8U, 999U}) {
        try {
            apply_drone_scene(current, index, 42);
            return 1;
        } catch (const std::invalid_argument &) {
        }
    }
    for (auto seed : {0U, 10000U}) {
        try {
            apply_drone_scene(current, 0, seed);
            return 1;
        } catch (const std::invalid_argument &) {
        }
    }
    current.drone_synth.level = 0;
    if (apply_drone_scene(current, 0, 42).drone_synth.level != 0)
        return 1;
    std::puts("Drone scenes: original scenes render, bounds, unchanged level/rack/source and "
              "preset roundtrip passed");
}
