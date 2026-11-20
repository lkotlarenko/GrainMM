// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument.hpp"
#include "grainmm/instrument_export.hpp"
#include "grainmm/patch_store.hpp"
#include "grainmm/preset.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
namespace fs = std::filesystem;
namespace {
void check(bool ok) {
    if (!ok)
        throw std::runtime_error("instrument export/pattern storage regression");
}
template <class F> void rejects(F f) {
    bool failed = false;
    try {
        f();
    } catch (const std::exception &) {
        failed = true;
    }
    check(failed);
}
std::string bytes(const fs::path &p) {
    std::ifstream in(p, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}
} // namespace
int main() {
    char name[] = "/tmp/grainmm-export-XXXXXX";
    const auto created = ::mkdtemp(name);
    if (!created)
        return 1;
    const fs::path root(created);
    try {
        grainmm::Preset preset;
        preset.source = grainmm::SampleSource::Tone;
        auto &p = preset.parameters;
        p.oscillator.level = .1F;
        p.oscillator_b.level = .05F;
        p.oscillator_b.note = 64;
        p.synth_color = {.75F, 1.5F, .2F, .4F, .05F, 2};
        p.groove = {.5F, 7, 2, 2, .6F, 3};
        p.drone_synth.level = .2F;
        p.drone_synth.event_mix = .3F;
        p.delay.mix = .15F;
        p.reverb.mix = .2F;
        p.output.gain_db = 6;
        p.output.limiter = 1;
        p.slice_pattern.manual = true;
        p.slice_pattern.steps[2] = {4, false, true, .4F};
        p.slice_pattern.widths[1] = .7F;
        grainmm::save_preset(root.string(), preset);
        check(grainmm::load_preset(root.string())->parameters.slice_pattern == p.slice_pattern);
        grainmm::PatchStore patches(root.string());
        patches.save(0, {"PATTERN", preset}, false);
        check(patches.load(0)->preset.parameters.slice_pattern == p.slice_pattern);
        auto encoded = grainmm::encode_preset(preset);
        std::istringstream modern(encoded);
        std::string header, fields, source;
        std::getline(modern, header);
        std::getline(modern, fields);
        std::getline(modern, source);
        std::istringstream scalars(fields);
        std::string legacy_fields, scalar;
        for (unsigned i = 0; i < 132; ++i) {
            scalars >> scalar;
            legacy_fields += scalar + " ";
        }
        auto old = "GrainMM 18\n" + legacy_fields + "\n" + source + "\n";
        const auto legacy17 = "GrainMM 17\n" + legacy_fields + "\n" + source + "\n" +
                              encoded.substr(encoded.find("Pattern 1"));
        const auto previous = grainmm::decode_preset(legacy17, 17);
        check(previous.parameters.slice_pattern == p.slice_pattern);
        check(previous.parameters.oscillator_b.level == 0 &&
              previous.parameters.synth_color.sub_level == 0 &&
              previous.parameters.groove.length == 16);
        old.replace(0, 10, "GrainMM 16");
        check(!grainmm::decode_preset(old, 16).parameters.slice_pattern.manual);
        fs::rename(root / "parameters.v18", root / "new-save.backup");
        std::ofstream(root / "parameters.v16") << old;
        const auto migrated = grainmm::load_preset(root.string());
        check(migrated && !migrated->parameters.slice_pattern.manual);
        grainmm::save_preset(root.string(), *migrated);
        check(bytes(root / "parameters.v16") == old);
        std::ofstream(root / "parameters.v18") << "corrupt modern data";
        rejects([&] { grainmm::load_preset(root.string()); });
        grainmm::save_preset(root.string(), preset);
        check(grainmm::load_preset(root.string())->parameters.synth_color.table_mix == .75F);
        check(patches.load(0)->preset.parameters.groove.length == 7);
        rejects([&] { grainmm::decode_preset(encoded.substr(0, encoded.size() - 5)); });
        auto corrupt = encoded;
        corrupt.replace(corrupt.find("Pattern 1"), 9, "Pattern 2");
        rejects([&] { grainmm::decode_preset(corrupt); });
        p.slice_pattern.widths[1] = -1;
        rejects([&] { grainmm::encode_preset(preset); });
        p.slice_pattern.widths[1] = .7F;
        grainmm::InstrumentExport job(root.string(), nullptr, p, 1, 8000);
        check(job.progress() == 0 && !job.complete());
        while (!job.complete())
            job.advance(1);
        check(job.progress() == 100);
        const auto rendered = bytes(root / "exports" / job.filename());
        check(rendered.size() == 44 + 8000 * 4);
        grainmm::Instrument reference(nullptr, 8000);
        reference.set_parameters(p);
        unsigned at = 0;
        while (at < 8000) {
            std::array<std::int16_t, 512> pcm{};
            const unsigned frames = std::min(256U, at < 7840 ? 7840 - at : 8000 - at);
            reference.render(pcm.data(), frames, 2, at < 7840);
            for (unsigned i = 0; i < frames * 2; ++i) {
                const auto offset = 44 + at * 4 + i * 2;
                const auto value = static_cast<std::uint16_t>(
                    static_cast<unsigned char>(rendered[offset]) |
                    (static_cast<unsigned>(static_cast<unsigned char>(rendered[offset + 1])) << 8));
                check(value == static_cast<std::uint16_t>(pcm[i]));
            }
            at += frames;
        }
        grainmm::InstrumentExport repeat(root.string(), nullptr, p, 1, 8000);
        while (!repeat.complete())
            repeat.advance(7);
        check(bytes(root / "exports" / repeat.filename()) == rendered);
        job.cancel(); // Completed files must survive cancellation/destruction.
        check(bytes(root / "exports" / job.filename()) == rendered);
        {
            grainmm::InstrumentExport cancel(root.string(), nullptr, p, 1, 8000);
            const auto path = root / "exports" / cancel.filename();
            cancel.advance();
            cancel.cancel();
            check(!fs::exists(path));
        }
        {
            grainmm::InstrumentExport replace(root.string(), nullptr, p, 1, 8000);
            const auto path = root / "exports" / replace.filename();
            fs::rename(path, root / "preserved-partial.wav");
            std::ofstream(path) << "owner data";
            rejects([&] { replace.cancel(); });
            check(bytes(path) == "owner data");
        }
        fs::create_directory(root / "unsafe");
        fs::create_directory_symlink(root / "exports", root / "unsafe/exports");
        rejects(
            [&] { grainmm::InstrumentExport unsafe((root / "unsafe").string(), nullptr, p, 1); });
        rejects([&] { grainmm::InstrumentExport invalid(root.string(), nullptr, p, 0); });
        rejects([&] { repeat.advance(33); });
        std::puts("full graph export: reference PCM, deterministic blocks, cancellation, paths and "
                  "pattern storage pass");
        fs::remove_all(root); // Exclusive mkdtemp fixture only.
    } catch (const std::exception &e) {
        std::fprintf(stderr, "%s\n", e.what());
        fs::remove_all(root);
        return 1;
    }
}
