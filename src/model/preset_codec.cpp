// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/parameter_schema.hpp"
#include "grainmm/preset.hpp"
#include "grainmm/sample_path.hpp"
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
namespace grainmm {
namespace {
std::array<float, parameter_count> values(const InstrumentParameters &p) {
    std::array<float, parameter_count> result{};
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = parameter_definition(i).read(p);
    return result;
}
void check_parameters(const Preset &p) {
    const auto actual = values(p.parameters), safe = values(bounded(p.parameters));
    if (p.parameters.slice_pattern != bounded(p.parameters.slice_pattern))
        throw std::runtime_error("invalid slice pattern");
    if (p.source != SampleSource::Legacy && p.source != SampleSource::Tone &&
        p.source != SampleSource::File)
        throw std::runtime_error("unsupported sample selection mode");
    if (p.source == SampleSource::File ? !safe_sample_path(p.sample) : !p.sample.empty())
        throw std::runtime_error("invalid preset sample selection");
    for (std::size_t i = 0; i < actual.size(); ++i)
        if (!std::isfinite(actual[i]) || actual[i] != safe[i])
            throw std::runtime_error("preset parameter outside supported range");
}
} // namespace
Preset decode_preset(const std::string &data, int expected_version) {
    if (data.empty() || data.size() > 4096 || expected_version < 0 || expected_version > 18)
        throw std::runtime_error("invalid encoded preset size or version");
    std::istringstream input(data);
    input.imbue(std::locale::classic());
    std::string magic;
    int version = 0;
    Preset preset;
    auto &p = preset.parameters;
    if (!(input >> magic >> version) || magic != "GrainMM" || version < 1 || version > 18 ||
        (expected_version != 0 && version != expected_version))
        throw std::runtime_error("corrupt or unsupported GrainMM preset");
    for (std::size_t i = 0; i < parameter_count_for_version(version); ++i)
        if (!(input >> parameter_definition(i).write(p)))
            throw std::runtime_error("incomplete preset parameters");
    if (version >= 3) {
        int source = 0;
        if (!(input >> source))
            throw std::runtime_error("missing preset sample mode");
        input >> std::ws;
        if (input.peek() != '"' || !(input >> std::quoted(preset.sample)))
            throw std::runtime_error("invalid preset sample name");
        preset.source = static_cast<SampleSource>(source);
    }
    if (version < 15)
        p.drone_synth.event_mix = 0; // Old scenes keep their original pad-only mix.
    if (version >= 17) {
        int pattern_version = 0, manual = 0;
        if (!(input >> magic >> pattern_version >> manual) || magic != "Pattern" ||
            pattern_version != 1 || (manual != 0 && manual != 1))
            throw std::runtime_error("invalid pattern header");
        p.slice_pattern.manual = manual != 0;
        for (unsigned i = 0; i < p.slice_pattern.steps.size(); ++i) {
            auto &s = p.slice_pattern.steps[i];
            int enabled = 0, reverse = 0;
            if (!(input >> s.region >> enabled >> reverse >> s.probability >>
                  p.slice_pattern.widths[i]) ||
                (enabled != 0 && enabled != 1) || (reverse != 0 && reverse != 1))
                throw std::runtime_error("invalid pattern step");
            s.enabled = enabled != 0;
            s.reverse = reverse != 0;
        }
    }
    input >> std::ws;
    if (!input.eof())
        throw std::runtime_error("unexpected preset data");
    check_parameters(preset);
    return preset;
}
std::string encode_preset(const Preset &p) {
    check_parameters(p);
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << "GrainMM 18\n" << std::setprecision(std::numeric_limits<float>::max_digits10);
    for (const float value : values(p.parameters))
        output << value << ' ';
    output << '\n' << static_cast<int>(p.source) << ' ' << std::quoted(p.sample) << '\n';
    output << "Pattern 1 " << p.parameters.slice_pattern.manual << '\n';
    for (unsigned i = 0; i < p.parameters.slice_pattern.steps.size(); ++i) {
        const auto &s = p.parameters.slice_pattern.steps[i];
        output << s.region << ' ' << s.enabled << ' ' << s.reverse << ' ' << s.probability << ' '
               << p.parameters.slice_pattern.widths[i] << '\n';
    }
    const auto data = output.str();
    if (data.size() > 4096)
        throw std::runtime_error("encoded preset exceeds supported size");
    return data;
}
} // namespace grainmm
