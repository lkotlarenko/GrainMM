// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/instrument_parameters.hpp"
#include <optional>
#include <string>
namespace grainmm {
enum class SampleSource { Legacy = 0, Tone = 1, File = 2 };
struct Preset {
    InstrumentParameters parameters;
    SampleSource source = SampleSource::Legacy;
    std::string sample;
};
// Strict bounded codec shared by autosave and named patches; control thread only.
// Expected0 accepts the declared supported version; storage requires its filename version.
Preset decode_preset(const std::string &data, int expected_version = 0);
std::string encode_preset(const Preset &preset);
// Control-thread only. Fixed, versioned file in an explicit existing directory.
// Missing is normal; corrupt/unsupported/unsafe files throw and are preserved.
std::optional<Preset> load_preset(const std::string &directory);
// Atomic replacement; refuses unsafe files and an unfinished prior transaction.
void save_preset(const std::string &directory, const Preset &parameters);
} // namespace grainmm
