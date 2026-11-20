// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/patch_menu.hpp"
#include "grainmm/waveform.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
namespace fs = std::filesystem;
namespace {
void check(bool ok, const char *why) {
    if (!ok)
        throw std::runtime_error(why);
}
template <class F> void rejects(F f) {
    bool rejected = false;
    try {
        f();
    } catch (const std::exception &) {
        rejected = true;
    }
    check(rejected, "unsafe operation was accepted");
}
std::string bytes(const fs::path &p) {
    std::ifstream f(p);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}
} // namespace
int main() {
    char pattern[] = "/tmp/grainmm-library-XXXXXX";
    const char *created = ::mkdtemp(pattern);
    if (!created)
        return 1;
    const fs::path root(created);
    try {
        grainmm::PatchStore store(root.string());
        check(!store.load(0), "new slot empty");
        grainmm::Preset p;
        p.source = grainmm::SampleSource::File;
        p.sample = "bank/input.wav";
        p.parameters.grain.start = .25F;
        p.parameters.grain.end = .75F;
        p.parameters.oscillator.level = .1F;
        grainmm::Patch patch{"CLOUD 01", p};
        store.save(0, patch);
        const auto original = bytes(root / "patches/patch-01.gmm");
        auto loaded = store.load(0);
        check(loaded && loaded->name == patch.name && loaded->preset.sample == p.sample &&
                  loaded->preset.parameters.grain.start == .25F,
              "roundtrip complete patch");
        rejects([&] { store.save(0, {"REPLACE", p}); });
        check(bytes(root / "patches/patch-01.gmm") == original,
              "unconfirmed replacement changed patch");
        std::ofstream(root / "patches/patch-01.gmm.pending") << "interrupted";
        rejects([&] { store.save(0, {"REPLACE", p}, true); });
        check(bytes(root / "patches/patch-01.gmm") == original, "pending save lost old patch");
        fs::remove(root / "patches/patch-01.gmm.pending");
        store.save(0, {"REPLACE", p}, true);
        check(store.load(0)->name == "REPLACE", "confirmed replace");
        for (const auto *name :
             {"", "bad lower", "../ESCAPE", "PAD\nB", " PAD", "PAD ", "ABCDEFGHIJKLMNOPQRSTU"})
            rejects([&] { store.save(1, {name, p}); });
        rejects([&] { store.load(16); });
        rejects([&] { store.save(16, patch); });
        std::ofstream(root / "protected") << "KEEP";
        fs::create_symlink(root / "protected", root / "patches/patch-02.gmm");
        rejects([&] { store.load(1); });
        rejects([&] { store.save(1, patch, true); });
        check(bytes(root / "protected") == "KEEP", "symlink target changed");
        std::ofstream(root / "patches/patch-03.gmm") << "broken user patch";
        const auto listing = store.list();
        check(listing[0].valid && listing[0].occupied && !listing[1].valid && !listing[2].valid &&
                  !listing[3].occupied,
              "listing corrupt/empty slots");
        std::istringstream current(grainmm::encode_preset(p));
        std::string header;
        std::getline(current, header);
        std::ostringstream legacy;
        legacy << "GrainMM Patch 1\n\"LEGACY PATCH\"\nGrainMM 8\n";
        for (unsigned i = 0; i < 66; ++i) {
            std::string value;
            current >> value;
            legacy << value << ' ';
        }
        legacy << "\n2 \"bank/input.wav\"\n";
        std::ofstream(root / "patches/patch-04.gmm") << legacy.str();
        const auto v8 = store.load(3);
        check(v8 && v8->name == "LEGACY PATCH" && v8->preset.parameters.harmony.chord == 0 &&
                  v8->preset.parameters.grain.start == .25F,
              "v8 patch compatible without rewrite");
        const auto saved = grainmm::encode_preset(p);
        check(grainmm::decode_preset(saved).sample == p.sample, "codec shared roundtrip");
        rejects([&] { grainmm::decode_preset(saved + "extra"); });
        rejects([&] { grainmm::decode_preset(std::string(2049, 'X')); });
        fs::create_symlink(root, root / "alias");
        rejects([&] { grainmm::PatchStore bad((root / "alias").string()); });
        grainmm::PatchMenu menu(listing);
        check(menu.input(grainmm::PatchKey::Save) == grainmm::PatchAction::None,
              "first Save must only ask");
        check(menu.input(grainmm::PatchKey::Back) == grainmm::PatchAction::None,
              "cancel confirmation stays in menu");
        check(menu.input(grainmm::PatchKey::Save) == grainmm::PatchAction::None,
              "cancel clears overwrite authorization");
        check(menu.input(grainmm::PatchKey::Save) == grainmm::PatchAction::Save && menu.replacing(),
              "second separate Save confirms");
        menu.input(grainmm::PatchKey::Name);
        check(menu.naming(), "naming enters");
        menu.input(grainmm::PatchKey::Next);
        menu.input(grainmm::PatchKey::Accept);
        check(!menu.naming() && menu.name() != "REPLACE" &&
                  grainmm::PatchStore::valid_name(menu.name()),
              "name editor accepts draft");
        menu.input(grainmm::PatchKey::Name);
        menu.input(grainmm::PatchKey::Back);
        check(!menu.naming(), "name cancellation");
        menu.input(grainmm::PatchKey::Previous);
        check(menu.selection() == 15, "slot wrap");
        check(menu.input(grainmm::PatchKey::Save) == grainmm::PatchAction::Save &&
                  !menu.replacing(),
              "empty slot direct save");
        const grainmm::Sample singleton(8000, 1, {.5F});
        const grainmm::Waveform tiny(singleton);
        for (const auto &peak : tiny.peaks())
            check(peak.low == .5F && peak.high == .5F, "one-frame waveform");
        std::vector<float> signal(1024);
        for (std::size_t i = 0; i < signal.size(); ++i)
            signal[i] = i % 2 ? .75F : -.5F;
        const grainmm::Sample stereo(48000, 2, std::move(signal));
        const grainmm::Waveform view(stereo);
        for (const auto &peak : view.peaks())
            check(peak.low == -.5F && peak.high == .75F, "stereo min/max preserve peaks");
        check(view.seconds() > 0 && view.seconds() < .02, "duration");
        // An anchored store must survive renaming its parent, unlike string paths.
        fs::rename(root / "patches", root / "moved");
        store.save(4, patch);
        check(fs::exists(root / "moved/patch-05.gmm"), "directory anchor lost");
        fs::remove_all(root);
        std::cout << "patch recovery/menu/waveform tests passed\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        fs::remove_all(root);
        return 1;
    }
}
