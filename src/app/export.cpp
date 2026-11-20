// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument_export.hpp"
#include "grainmm/preset.hpp"
#include "grainmm/sample_library.hpp"
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <stdexcept>
int main(int argc, char **argv) {
    try {
        std::string state, samples;
        unsigned seconds = 15;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--help") {
                std::puts(
                    "grainmm-export --state-dir DIR [--sample-dir DIR] [--seconds 1..60]\n"
                    "Restart saved sources + full FX + OUTPUT as a numbered WAV in DIR/exports.\n"
                    "No audio/display device is opened. Existing files are never replaced.");
                return 0;
            }
            if (++i >= argc)
                throw std::invalid_argument("missing argument value");
            const std::string value = argv[i];
            if (arg == "--state-dir")
                state = value;
            else if (arg == "--sample-dir")
                samples = value;
            else if (arg == "--seconds") {
                char *end = nullptr;
                const auto n = std::strtoul(value.c_str(), &end, 10);
                if (value.empty() || *end || n < 1 || n > 60)
                    throw std::invalid_argument("seconds must be 1..60");
                seconds = static_cast<unsigned>(n);
            } else
                throw std::invalid_argument("unknown export argument");
        }
        if (state.empty())
            throw std::invalid_argument("--state-dir is required");
        const auto preset = grainmm::load_preset(state);
        if (!preset)
            throw std::runtime_error("no saved instrument settings");
        std::optional<grainmm::Sample> sample;
        if (preset->source != grainmm::SampleSource::Tone) {
            grainmm::SampleLibrary library(samples.empty() ? state : samples, false);
            sample.emplace(library.load(
                preset->source == grainmm::SampleSource::Legacy ? "sample.wav" : preset->sample));
        }
        grainmm::InstrumentExport job(state, sample ? &*sample : nullptr, preset->parameters,
                                      seconds);
        while (!job.complete())
            job.advance();
        std::printf("export=%s seconds=%u rate=48000 stereo=1 clipped_samples=%u\n",
                    job.filename().c_str(), seconds, job.clipped_samples());
    } catch (const std::exception &e) {
        std::fprintf(stderr, "grainmm-export: %s\n", e.what());
        return 1;
    }
}
