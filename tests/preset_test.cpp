// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/parameter_schema.hpp"
#include "grainmm/preset.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
namespace fs = std::filesystem;
void require(bool ok) {
    if (!ok)
        throw std::runtime_error("preset regression");
}
template <typename F> void rejects(F operation) {
    bool failed = false;
    try {
        operation();
    } catch (const std::exception &) {
        failed = true;
    }
    require(failed);
}
int main() {
    char pattern[] = "/tmp/grainmm-preset-XXXXXX";
    const char *created = ::mkdtemp(pattern);
    if (!created)
        return 1;
    const fs::path root(created);
    try {
        require(!grainmm::load_preset(root.string()));
        grainmm::Preset p;
        p.parameters.grain.pitch = 1.35F;
        p.parameters.grain.position = 0.713F;
        p.source = grainmm::SampleSource::File;
        p.sample = "bank/sound.wav";
        p.parameters.delay.mix = 0.35F;
        p.parameters.delay.time_ms = 712.3F;
        p.parameters.eq_db[2] = -7;
        p.parameters.reverb.mix = 0.35F;
        p.parameters.grain.reverse = 0.8F;
        p.parameters.grain.pitch_jitter = 7;
        p.parameters.grain.scan_hz = -0.2F;
        p.parameters.oscillator.level = 0.1F;
        p.parameters.oscillator.wave = 2;
        p.parameters.compressor.mix = 0.5F;
        p.parameters.tape = {12, 4000, 1.2F, 0.1F, 0.75F, -6};
        p.parameters.resonator = {60, .7F, .8F, .2F, .5F, .5F};
        p.parameters.rhythm = {135, 2, .7F, .4F, 1, 90};
        p.parameters.grain.start = .1F;
        p.parameters.grain.end = .8F;
        p.parameters.grain.attack = .1F;
        p.parameters.grain.release = .3F;
        p.parameters.grain.curve = .8F;
        p.parameters.grain.width = .6F;
        p.parameters.crusher = {6, 8, 6, 5000, .5F, -6};
        p.parameters.slice = {3, 12, 1, .6F, 2, .4F};
        p.parameters.filter = {900, 1.5F, 2, 3, .4F, -3};
        p.parameters.chorus = {1.2F, 5, 8, .3F, .4F, .8F};
        p.parameters.harmony = {12, 3, 2, 2, .7F, 15};
        grainmm::save_preset(root.string(), p);
        auto loaded = grainmm::load_preset(root.string());
        require(loaded && loaded->parameters.crusher.bits == 6 &&
                loaded->parameters.filter.mode == 2 && loaded->parameters.chorus.depth_ms == 5);
        require(loaded && loaded->parameters.harmony.chord == 3 &&
                loaded->parameters.harmony.transpose == 12 &&
                loaded->parameters.harmony.detune_cents == 15);
        require(loaded && loaded->parameters.oscillator.level == 0.1F &&
                loaded->parameters.oscillator.wave == 2 &&
                loaded->parameters.compressor.mix == 0.5F);
        require(loaded && loaded->parameters.tape.drive_db == 12 &&
                loaded->parameters.tape.tone_hz == 4000 && loaded->parameters.tape.wow_ms == 1.2F &&
                loaded->parameters.tape.flutter_ms == 0.1F &&
                loaded->parameters.tape.mix == 0.75F && loaded->parameters.tape.output_db == -6);
        require(loaded && loaded->parameters.resonator.note == 60 &&
                loaded->parameters.resonator.mix == .5F && loaded->parameters.rhythm.bpm == 135 &&
                loaded->parameters.rhythm.stereo_degrees == 90 &&
                loaded->parameters.grain.start == .1F && loaded->parameters.grain.end == .8F &&
                loaded->parameters.grain.curve == .8F);
        require(loaded && loaded->source == p.source && loaded->sample == p.sample);
        require(loaded && loaded->parameters.grain.pitch == p.parameters.grain.pitch &&
                loaded->parameters.grain.position == p.parameters.grain.position);
        require(loaded->parameters.delay.mix == p.parameters.delay.mix &&
                loaded->parameters.delay.time_ms == p.parameters.delay.time_ms);
        require(loaded->parameters.grain.reverse == p.parameters.grain.reverse &&
                loaded->parameters.grain.scan_hz == p.parameters.grain.scan_hz &&
                loaded->parameters.grain.pitch_jitter == 7);
        require(loaded->parameters.eq_db[2] == -7 && loaded->parameters.reverb.mix == 0.35F);
        std::ofstream(root / "parameters.v18.pending") << "interrupted";
        p.parameters.grain.pitch = 0.5F;
        rejects([&] { grainmm::save_preset(root.string(), p); });
        require(grainmm::load_preset(root.string())->parameters.grain.pitch ==
                loaded->parameters.grain.pitch);
        fs::remove(root / "parameters.v18.pending");
        grainmm::save_preset(root.string(), p);
        require(grainmm::load_preset(root.string())->parameters.grain.pitch ==
                p.parameters.grain.pitch);
        for (const char *invalid :
             {"GrainMM 2\n60 12 1 0 0 0.2\n", "", "GrainMM 1\n60 12 1 0 0 0.2 extra",
              "GrainMM 1\n60 12 999 0 0 0.2", "GrainMM 1\n60 12 nan 0 0 0.2"}) {
            std::ofstream(root / "parameters.v18") << invalid;
            rejects([&] { grainmm::load_preset(root.string()); });
        }
        fs::remove(root / "parameters.v18");
        std::ofstream(root / "protected") << "preserved";
        fs::create_symlink(root / "protected", root / "parameters.v18");
        rejects([&] { grainmm::load_preset(root.string()); });
        rejects([&] { grainmm::save_preset(root.string(), p); });
        require(fs::file_size(root / "protected") == 9);
        fs::remove(root / "parameters.v18");
        std::ofstream(root / "parameters.v1") << "GrainMM 1\n80 18 1.25 0.6 0.2 0.3\n";
        const auto legacy = grainmm::load_preset(root.string());
        require(legacy && legacy->parameters.grain.density == 18 &&
                legacy->parameters.delay.mix == 0);
        grainmm::save_preset(root.string(), *legacy);
        require(fs::exists(root / "parameters.v1") && fs::exists(root / "parameters.v18"));
        std::ofstream(root / "parameters.v18") << "broken modern preset";
        rejects([&] { grainmm::load_preset(root.string()); }); // Never mask v2 corruption with v1.
        fs::remove(root / "parameters.v18");
        std::ofstream(root / "parameters.v2")
            << "GrainMM 2\n80 19 1 0.5 0.1 0.2 450 0.35 0.3 0 1 9000\n";
        auto v2 = grainmm::load_preset(root.string());
        require(v2 && v2->parameters.grain.density == 19 &&
                v2->source == grainmm::SampleSource::Legacy);
        const std::string v3_text =
            "GrainMM 3\n80 21 1 0.5 0.1 0.2 450 0.35 0.3 0 1 9000\n2 \"bank/kept.wav\"\n";
        std::ofstream(root / "parameters.v3") << v3_text;
        const auto v3 = grainmm::load_preset(root.string());
        require(v3 && v3->parameters.grain.density == 21 && v3->sample == "bank/kept.wav" &&
                v3->parameters.eq_db[0] == 0 && v3->parameters.reverb.mix == 0);
        grainmm::save_preset(root.string(), *v3);
        std::ifstream old_file(root / "parameters.v3");
        const std::string preserved((std::istreambuf_iterator<char>(old_file)),
                                    std::istreambuf_iterator<char>());
        require(preserved == v3_text);
        fs::remove(root / "parameters.v18");
        const std::string v4_text = "GrainMM 4\n80 23 1 0.5 0.1 0.2 450 0.35 0.3 0 1 9000 1 2 3 4 "
                                    "5 6 0.4 0.7 6 7000 20 1\n2 \"bank/kept.wav\"\n";
        std::ofstream(root / "parameters.v4") << v4_text;
        const auto v4 = grainmm::load_preset(root.string());
        require(v4 && v4->parameters.grain.density == 23 && v4->parameters.reverb.mix == 0.4F &&
                v4->parameters.grain.reverse == 0 && v4->parameters.eq_db[5] == 6);
        grainmm::save_preset(root.string(), *v4);
        std::ifstream old_v4(root / "parameters.v4");
        require(std::string((std::istreambuf_iterator<char>(old_v4)),
                            std::istreambuf_iterator<char>()) == v4_text);
        fs::remove(root / "parameters.v18");
        const std::string v5_text =
            "GrainMM 5\n80 25 1 0.5 0.1 0.2 450 0.35 0.3 0 1 9000 1 2 3 4 5 6 0.4 0.7 6 7000 20 1 "
            "1 5 0.3 0.2 0.5 -0.1\n2 \"bank/kept.wav\"\n";
        std::ofstream(root / "parameters.v5") << v5_text;
        const auto v5 = grainmm::load_preset(root.string());
        require(v5 && v5->parameters.grain.density == 25 && v5->parameters.grain.reverse == 1 &&
                v5->parameters.oscillator.level == 0 && v5->parameters.compressor.mix == 0);
        grainmm::save_preset(root.string(), *v5);
        std::ifstream old_v5(root / "parameters.v5");
        require(std::string((std::istreambuf_iterator<char>(old_v5)),
                            std::istreambuf_iterator<char>()) == v5_text);
        fs::remove(root / "parameters.v18");
        const std::string v6_text =
            "GrainMM 6\n80 25 1 0.5 0.1 0.2 450 0.35 0.3 0 1 9000 1 2 3 4 5 6 0.4 0.7 6 7000 20 1 "
            "1 5 0.3 0.2 0.5 -0.1 0.05 60 2 3 0.5 5 -24 6 10 150 0 0.4\n2 \"bank/kept.wav\"\n";
        std::ofstream(root / "parameters.v6") << v6_text;
        const auto v6 = grainmm::load_preset(root.string());
        require(v6 && v6->parameters.oscillator.note == 60 &&
                v6->parameters.compressor.mix == 0.4F && v6->parameters.tape.mix == 0);
        grainmm::save_preset(root.string(), *v6);
        std::ifstream old_v6(root / "parameters.v6");
        require(std::string((std::istreambuf_iterator<char>(old_v6)),
                            std::istreambuf_iterator<char>()) == v6_text);
        fs::remove(root / "parameters.v18");
        const std::string v7_text =
            "GrainMM 7\n80 12 1 0.5 0.1 0.2 450 0.35 0 0 1 9000 0 0 0 0 0 0 0 0.7 6 7000 20 1 0 0 "
            "0 0 0 0 0.05 57 2 0 0.5 0 -18 4 10 150 0 0 6 9000 0.5 0.1 0.5 -6\n1 \"\"\n";
        std::ofstream(root / "parameters.v7") << v7_text;
        const auto v7 = grainmm::load_preset(root.string());
        require(v7 && v7->parameters.tape.drive_db == 6 && v7->parameters.resonator.mix == 0 &&
                v7->parameters.rhythm.tremolo == 0 && v7->parameters.grain.start == 0 &&
                v7->parameters.grain.end == 1);
        grainmm::save_preset(root.string(), *v7);
        std::ifstream old_v7(root / "parameters.v7");
        require(std::string((std::istreambuf_iterator<char>(old_v7)),
                            std::istreambuf_iterator<char>()) == v7_text);
        fs::remove(root / "parameters.v18");
        std::istringstream current(grainmm::encode_preset(p));
        std::string line;
        std::getline(current, line);
        std::ostringstream previous;
        previous << "GrainMM 8\n";
        for (unsigned i = 0; i < 66; ++i) {
            std::string value;
            current >> value;
            previous << value << ' ';
        }
        previous << "\n2 \"bank/sound.wav\"\n";
        const auto old_bytes = previous.str();
        std::ofstream(root / "parameters.v8") << old_bytes;
        const auto v8 = grainmm::load_preset(root.string());
        require(v8 && v8->parameters.harmony.chord == 0 && v8->parameters.harmony.octaves == 1 &&
                v8->parameters.grain.pitch == p.parameters.grain.pitch);
        grainmm::save_preset(root.string(), *v8);
        require(fs::exists(root / "parameters.v8") && fs::exists(root / "parameters.v18"));
        std::ifstream prior(root / "parameters.v8");
        require(std::string(std::istreambuf_iterator<char>(prior), {}) == old_bytes);
        std::ofstream(root / "parameters.v18") << "invalid current state";
        rejects([&] { grainmm::load_preset(root.string()); });
        fs::remove(root / "parameters.v18");
        std::istringstream now(grainmm::encode_preset(p));
        std::getline(now, line);
        std::ostringstream schema9;
        schema9 << "GrainMM 9\n";
        for (unsigned index = 0; index < 72; ++index) {
            std::string value;
            now >> value;
            schema9 << value << ' ';
        }
        schema9 << "\n2 \"bank/sound.wav\"\n";
        const auto v9_bytes = schema9.str();
        std::ofstream(root / "parameters.v9") << v9_bytes;
        const auto v9 = grainmm::load_preset(root.string());
        require(v9 && v9->parameters.harmony.chord == 3 && v9->parameters.crusher.mix == 0 &&
                v9->parameters.chorus.mix == 0);
        grainmm::save_preset(root.string(), *v9);
        std::ifstream prior9(root / "parameters.v9");
        require(std::string(std::istreambuf_iterator<char>(prior9), {}) == v9_bytes);
        // Independent old-format values: do not derive their order from the new registry.
        auto golden8 = v7_text.substr(0, v7_text.find("\n1 "));
        golden8.replace(0, 9, "GrainMM 8");
        golden8 += " 60 0.7 0.8 0.2 0.5 0.5 135 2 0.7 0.4 1 90 0.1 0.8 0.1 0.3 0.8 0.6\n2 "
                   "\"bank/sound.wav\"\n";
        const auto old8 = grainmm::decode_preset(golden8);
        require(old8.parameters.resonator.note == 60 && old8.parameters.rhythm.bpm == 135 &&
                old8.parameters.grain.start == .1F && old8.parameters.grain.width == .6F &&
                old8.parameters.harmony.chord == 0);
        auto golden9 = golden8.substr(0, golden8.find("\n2 "));
        golden9.replace(0, 9, "GrainMM 9");
        golden9 += " 12 3 2 2 0.7 15\n2 \"bank/sound.wav\"\n";
        const auto old9 = grainmm::decode_preset(golden9);
        require(old9.parameters.resonator.note == 60 && old9.parameters.harmony.transpose == 12 &&
                old9.parameters.harmony.chord == 3 && old9.parameters.harmony.detune_cents == 15 &&
                old9.parameters.crusher.mix == 0);
        // Schema10 migration preserves original bytes; new sequencing defaults off.
        fs::remove(root / "parameters.v18");
        std::istringstream newest(grainmm::encode_preset(p));
        std::getline(newest, line);
        std::ostringstream schema10;
        schema10 << "GrainMM 10\n";
        for (unsigned index = 0; index < 90; ++index) {
            std::string value;
            newest >> value;
            schema10 << value << ' ';
        }
        schema10 << "\n2 \"bank/sound.wav\"\n";
        const auto ten = schema10.str();
        std::ofstream(root / "parameters.v10") << ten;
        const auto migrated = grainmm::load_preset(root.string());
        require(migrated && migrated->parameters.slice.mode == 0 &&
                migrated->parameters.filter.mix == p.parameters.filter.mix);
        grainmm::save_preset(root.string(), *migrated);
        std::ifstream oldten(root / "parameters.v10");
        require(std::string(std::istreambuf_iterator<char>(oldten), {}) == ten);
        const auto roundtrip = grainmm::decode_preset(grainmm::encode_preset(p));
        require(roundtrip.parameters.slice.mode == 3 && roundtrip.parameters.slice.regions == 12 &&
                roundtrip.parameters.slice.reverse == .4F);
        fs::remove(root / "parameters.v18");
        std::istringstream twelve(grainmm::encode_preset(p));
        std::getline(twelve, line);
        std::ostringstream eleven;
        eleven << "GrainMM 11\n";
        for (unsigned i = 0; i < 96; ++i) {
            std::string value;
            twelve >> value;
            eleven << value << ' ';
        }
        eleven << "\n2 \"bank/sound.wav\"\n";
        const auto eleven_bytes = eleven.str();
        std::ofstream(root / "parameters.v11") << eleven_bytes;
        const auto oldeleven = grainmm::load_preset(root.string());
        require(oldeleven && oldeleven->parameters.collision.enabled == 0 &&
                oldeleven->parameters.slice.mode == 3);
        grainmm::save_preset(root.string(), *oldeleven);
        std::ifstream eleventext(root / "parameters.v11");
        require(std::string(std::istreambuf_iterator<char>(eleventext), {}) == eleven_bytes);
        p.parameters.collision.enabled = 1;
        p.parameters.collision.note = 64;
        const auto bells = grainmm::decode_preset(grainmm::encode_preset(p));
        require(bells.parameters.collision.enabled == 1 && bells.parameters.collision.note == 64);
        for (const char *bad : {"../escape.wav", "/absolute.wav", "bank/../x.wav", ""}) {
            p.sample = bad;
            rejects([&] { grainmm::save_preset(root.string(), p); });
        }
        p.sample = "valid.wav";
        fs::create_symlink(root, root / "alias");
        rejects([&] { grainmm::load_preset((root / "alias").string()); });
        p.parameters.grain.gain = std::numeric_limits<float>::infinity();
        rejects([&] { grainmm::save_preset(root.string(), p); });
        fs::remove(root / "parameters.v18");
        grainmm::Preset legacy12;
        legacy12.parameters.collision.enabled = 1;
        legacy12.parameters.collision.note = 67;
        std::ostringstream legacy12_stream;
        legacy12_stream << "GrainMM 12\n";
        for (std::size_t n = 0; n < 108; ++n)
            legacy12_stream << grainmm::parameter_definition(n).read(legacy12.parameters) << ' ';
        legacy12_stream << "\n1 \"\"\n";
        const auto legacy12_stream_bytes = legacy12_stream.str();
        std::ofstream(root / "parameters.v12") << legacy12_stream_bytes;
        auto upgraded = grainmm::load_preset(root.string());
        require(upgraded && upgraded->parameters.collision.note == 67 &&
                upgraded->parameters.drone.mode == 0);
        upgraded->parameters.drone.mode = 3;
        upgraded->parameters.drone.pitch_cents = 75;
        upgraded->parameters.drone.hold = 1;
        grainmm::save_preset(root.string(), *upgraded);
        auto drone_saved = grainmm::load_preset(root.string());
        require(drone_saved && drone_saved->parameters.drone.mode == 3 &&
                drone_saved->parameters.drone.pitch_cents == 75 &&
                drone_saved->parameters.drone.hold == 1);
        std::ifstream legacy12_stream_file(root / "parameters.v12");
        require(std::string(std::istreambuf_iterator<char>(legacy12_stream_file), {}) ==
                legacy12_stream_bytes);
        fs::remove(root / "parameters.v18");
        std::ostringstream thirteen;
        thirteen << "GrainMM 13\n";
        for (std::size_t n = 0; n < 114; ++n)
            thirteen << grainmm::parameter_definition(n).read(upgraded->parameters) << ' ';
        thirteen << "\n1 \"\"\n";
        const auto thirteen_bytes = thirteen.str();
        std::ofstream(root / "parameters.v13") << thirteen_bytes;
        auto old13 = grainmm::load_preset(root.string());
        require(old13 && old13->parameters.drone.mode == 3 &&
                old13->parameters.drone_synth.level == 0);
        old13->parameters.drone_synth = {.1F, 43, 3, 7, .6F, .7F};
        grainmm::save_preset(root.string(), *old13);
        auto ambient = grainmm::load_preset(root.string());
        require(ambient && ambient->parameters.drone_synth.level == .1F &&
                ambient->parameters.drone_synth.root == 43 &&
                ambient->parameters.drone_synth.mood == 3 &&
                ambient->parameters.drone_synth.evolve_seconds == 7 &&
                ambient->parameters.drone_synth.texture == .6F);
        std::ifstream old13file(root / "parameters.v13");
        require(std::string(std::istreambuf_iterator<char>(old13file), {}) == thirteen_bytes);
        fs::remove(root / "parameters.v18");
        std::ostringstream fourteen;
        fourteen << "GrainMM 14\n";
        for (std::size_t n = 0; n < 120; ++n)
            fourteen << grainmm::parameter_definition(n).read(ambient->parameters) << ' ';
        fourteen << "\n1 \"\"\n";
        const auto fourteen_bytes = fourteen.str();
        std::ofstream(root / "parameters.v14") << fourteen_bytes;
        auto old14 = grainmm::load_preset(root.string());
        require(old14 && old14->parameters.drone_synth.event_mix == 0 &&
                old14->parameters.drone_synth.level == .1F);
        old14->parameters.drone_synth.event_mix = .4F;
        old14->parameters.drone_synth.seed = 314;
        old14->parameters.drone_synth.pattern = 3;
        old14->parameters.drone_synth.level = .6F;
        grainmm::save_preset(root.string(), *old14);
        auto generated = grainmm::load_preset(root.string());
        require(generated && generated->parameters.drone_synth.event_mix == .4F &&
                generated->parameters.drone_synth.seed == 314 &&
                generated->parameters.drone_synth.pattern == 3 &&
                generated->parameters.drone_synth.level == .6F);
        std::ifstream old14file(root / "parameters.v14");
        require(std::string(std::istreambuf_iterator<char>(old14file), {}) == fourteen_bytes);
        fs::remove(root / "parameters.v18");
        std::ostringstream fifteen;
        fifteen << "GrainMM 15\n";
        for (std::size_t n = 0; n < 126; ++n)
            fifteen << grainmm::parameter_definition(n).read(generated->parameters) << ' ';
        fifteen << "\n1 \"\"\n";
        const auto fifteen_bytes = fifteen.str();
        std::ofstream(root / "parameters.v15") << fifteen_bytes;
        auto old15 = grainmm::load_preset(root.string());
        require(old15 && old15->parameters.drone_synth.event_mix == .4F &&
                old15->parameters.output.gain_db == 0 && old15->parameters.output.limiter == 0);
        old15->parameters.output = {6, 1, -2, 210, .1F, 1};
        grainmm::save_preset(root.string(), *old15);
        auto master = grainmm::load_preset(root.string());
        require(master && master->parameters.output.gain_db == 6 &&
                master->parameters.output.limiter == 1 &&
                master->parameters.output.ceiling_db == -2 && master->parameters.output.mono == 1);
        std::ifstream old15file(root / "parameters.v15");
        require(std::string(std::istreambuf_iterator<char>(old15file), {}) == fifteen_bytes);
        fs::remove_all(root); // Only this test's mkdtemp fixture.
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        fs::remove_all(root);
        return 1;
    }
}
