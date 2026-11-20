// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/sample_library.hpp"
#include "grainmm/wave.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
namespace fs = std::filesystem;
namespace {
void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}
template <typename F> void rejects(F operation) {
    bool failed = false;
    try {
        operation();
    } catch (const std::exception &) {
        failed = true;
    }
    require(failed, "unsafe library operation accepted");
}
void wave(const fs::path &path) {
    const std::int16_t values[] = {16384, -16384};
    grainmm::WaveWriter writer(path.string(), 48000, 1, 2);
    writer.write(values, 2);
    writer.finish();
}
std::size_t descriptors() {
    std::size_t count = 0;
    for (const auto &entry : fs::directory_iterator("/proc/self/fd")) {
        (void)entry;
        ++count;
    }
    return count;
}
} // namespace
int main() {
    char pattern[] = "/tmp/grainmm-library-XXXXXX";
    const char *created = ::mkdtemp(pattern);
    if (!created)
        return 1;
    const fs::path root(created);
    const auto original_descriptors = descriptors();
    try {
        fs::create_directory(root / "bank");
        wave(root / "A.wav");
        wave(root / "b.WAV");
        wave(root / ".hidden.wav");
        wave(root / "bank/nested.wav");
        std::ofstream(root / "ignored.txt") << "not audio";
        fs::create_symlink(root / "A.wav", root / "symlink.wav");
        fs::create_directory_symlink(root / "bank", root / "alias");
        require(::mkfifo((root / "fifo.wav").c_str(), 0600) == 0, "cannot create test FIFO");
        {
            grainmm::SampleLibrary library(root.string());
            require(library.entries().size() == 3, "library filtering failed");
            require(library.entries()[0].directory && library.entries()[0].name == "bank",
                    "folder sorting");
            library.move(-1);
            require(library.selected_path() == "b.WAV", "selection wrap");
            library.move(1);
            require(library.enter() && library.path() == "bank", "cannot enter folder");
            require(library.selected_path() == "bank/nested.wav", "relative path contract");
            auto sample = library.load(library.selected_path());
            require(sample.frames() == 2 && sample.value(0, 0) == 0.5F, "decoded sample mismatch");
            for (int i = 0; i < 10; ++i)
                library.refresh();
            require(library.entries().size() == 1, "shared directory offset lost entries");
            fs::rename(root / "bank", root / "bank-moved");
            fs::create_directory_symlink(root / "bank-moved", root / "bank");
            rejects([&] { library.load("bank/nested.wav"); });
            rejects([&] { library.load("symlink.wav"); });
            require(library.parent() && !library.parent(), "root escape allowed");
            require(library.load("bank-moved/nested.wav").frames() == 2, "nested anchored load");
            rejects([&] { library.load("fifo.wav"); });
            for (const std::string bad : {"", "/A.wav", "../A.wav", "bank/../A.wav", "./A.wav",
                                          "a//A.wav", "a\\A.wav", "A.wav/"}) {
                require(!grainmm::SampleLibrary::safe_relative(bad), "unsafe relative path");
                rejects([&] { library.load(bad); });
            }
            require(!grainmm::SampleLibrary::safe_relative(std::string(129, 'x')),
                    "long name accepted");
            require(!grainmm::SampleLibrary::safe_relative("bank/line\n.wav"),
                    "control character accepted");
        }
        require(descriptors() == original_descriptors, "library leaked directory descriptors");
        fs::create_directory(root / "deep");
        fs::path nested = root / "deep";
        for (unsigned i = 0; i < 8; ++i) {
            nested /= "d";
            fs::create_directory(nested);
        }
        {
            grainmm::SampleLibrary depth((root / "deep").string());
            for (unsigned i = 0; i < 7; ++i)
                require(depth.enter(), "valid depth rejected");
            rejects([&] { depth.enter(); });
        }
        require(descriptors() == original_descriptors, "depth error leaked descriptors");
        fs::create_directory(root / "too-many");
        for (unsigned i = 0; i < 129; ++i)
            std::ofstream(root / "too-many" / (std::to_string(i) + ".wav"));
        for (int i = 0; i < 5; ++i)
            rejects([&] { grainmm::SampleLibrary too_many((root / "too-many").string()); });
        rejects([&] { grainmm::SampleLibrary linked((root / "alias").string()); });
        rejects([&] { grainmm::SampleLibrary linked((root / "alias").string() + "/"); });
        require(descriptors() == original_descriptors, "failed constructor leaked descriptors");
        fs::remove_all(root); // Exclusively test-created mkdtemp tree.
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        fs::remove_all(root);
        return 1;
    }
}
