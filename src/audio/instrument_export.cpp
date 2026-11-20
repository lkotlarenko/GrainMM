// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument_export.hpp"
#include "grainmm/instrument.hpp"
#include "grainmm/wave.hpp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
namespace grainmm {
namespace {
struct Descriptor {
    int fd = -1;
    ~Descriptor() {
        if (fd >= 0)
            ::close(fd);
    }
    void open_directory(const std::string &path) {
        fd = ::open(path.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (fd < 0)
            throw std::runtime_error("cannot open safe export directory");
    }
    std::string anchor() const { return "/proc/self/fd/" + std::to_string(fd) + "/"; }
};
} // namespace
struct InstrumentExport::State {
    Descriptor directory;
    std::unique_ptr<Instrument> instrument;
    std::unique_ptr<WaveWriter> writer;
    std::string filename, path;
    std::pair<std::uint64_t, std::uint64_t> created{};
    unsigned total = 0, rendered = 0, fade = 0;
    bool owned = false, complete = false, cancelled = false;
    bool remove_partial() noexcept {
        if (!owned || complete)
            return true;
        struct stat now{};
        if (::lstat(path.c_str(), &now) != 0)
            return errno == ENOENT;
        if (!S_ISREG(now.st_mode) || static_cast<std::uint64_t>(now.st_dev) != created.first ||
            static_cast<std::uint64_t>(now.st_ino) != created.second)
            return false;
        if (::unlink(path.c_str()) != 0)
            return false;
        owned = false;
        return true;
    }
    ~State() {
        writer.reset();
        remove_partial();
    }
};
InstrumentExport::InstrumentExport(const std::string &state_directory, const Sample *sample,
                                   InstrumentParameters parameters, unsigned seconds, int rate)
    : state_(std::make_unique<State>()) {
    if (seconds < 1 || seconds > 60 || rate < 8000 || rate > 96000)
        throw std::invalid_argument("export duration/rate outside supported bounds");
    auto &s = *state_;
    s.total = seconds * static_cast<unsigned>(rate);
    s.fade = s.total - static_cast<unsigned>(rate / 50);
    Descriptor parent;
    parent.open_directory(state_directory);
    const auto child = parent.anchor() + "exports";
    if (::mkdir(child.c_str(), 0700) != 0 && errno != EEXIST)
        throw std::runtime_error("cannot create exports directory");
    s.directory.open_directory(child);
    struct statvfs space{};
    if (::fstatvfs(s.directory.fd, &space) != 0 || !space.f_frsize ||
        static_cast<std::uint64_t>(space.f_bavail) <
            (static_cast<std::uint64_t>(s.total) * 4 + 44 + 1048576 + space.f_frsize - 1) /
                space.f_frsize)
        throw std::runtime_error("not enough space for export plus 1 MiB reserve");
    s.instrument = std::make_unique<Instrument>(sample, rate);
    s.instrument->set_parameters(parameters);
    for (unsigned number = 1; number <= 999; ++number) {
        char name[32]{};
        std::snprintf(name, sizeof(name), "GrainMM-%03u.wav", number);
        s.filename = name;
        s.path = s.directory.anchor() + s.filename;
        struct stat info{};
        if (::lstat(s.path.c_str(), &info) == 0)
            continue;
        if (errno != ENOENT)
            throw std::runtime_error("cannot inspect export filename");
        s.writer = std::make_unique<WaveWriter>(s.path, rate, 2, s.total);
        s.created = s.writer->file_identity();
        s.owned = true;
        return;
    }
    throw std::runtime_error("all 999 export names are occupied");
}
InstrumentExport::~InstrumentExport() = default;
void InstrumentExport::advance(unsigned blocks) {
    auto &s = *state_;
    if (blocks < 1 || blocks > 32)
        throw std::invalid_argument("invalid export work budget");
    if (s.complete || s.cancelled)
        return;
    std::array<std::int16_t, 512> pcm{};
    while (blocks-- && s.rendered < s.total) {
        const auto frames =
            std::min(256U, s.rendered < s.fade ? s.fade - s.rendered : s.total - s.rendered);
        s.instrument->render(pcm.data(), frames, 2, s.rendered < s.fade);
        s.writer->write(pcm.data(), frames);
        s.rendered += frames;
    }
    if (s.rendered == s.total) {
        s.writer->finish();
        s.writer.reset();
        s.complete = true;
        if (::fsync(s.directory.fd) != 0 && errno != EINVAL)
            throw std::runtime_error("export finished but directory flush failed");
    }
}
void InstrumentExport::cancel() {
    auto &s = *state_;
    s.writer.reset();
    s.cancelled = true;
    if (!s.remove_partial())
        throw std::runtime_error("partial export changed; preserved for recovery");
}
bool InstrumentExport::complete() const noexcept { return state_->complete; }
unsigned InstrumentExport::progress() const noexcept {
    return state_->rendered * 100U / state_->total;
}
unsigned InstrumentExport::clipped_samples() const noexcept {
    return state_->instrument->clipped_samples();
}
const std::string &InstrumentExport::filename() const noexcept { return state_->filename; }
} // namespace grainmm
