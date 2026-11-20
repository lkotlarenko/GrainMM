// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/preset.hpp"
#include "grainmm/sample_path.hpp"
#include <cerrno>
#include <fcntl.h>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

namespace grainmm {
namespace {
constexpr const char *filename = "parameters.v18";
class Descriptor {
  public:
    explicit Descriptor(int value) : value_(value) {
        if (value < 0)
            throw std::runtime_error("preset file operation failed");
    }
    ~Descriptor() { ::close(value_); }
    Descriptor(const Descriptor &) = delete;
    Descriptor &operator=(const Descriptor &) = delete;
    int get() const noexcept { return value_; }

  private:
    int value_;
};
std::string anchor(int fd) { return "/proc/self/fd/" + std::to_string(fd) + "/"; }
void check_destination(const std::string &path) {
    struct stat info{};
    if (::lstat(path.c_str(), &info) == 0) {
        if (!S_ISREG(info.st_mode))
            throw std::runtime_error("preset destination is not a regular file");
    } else if (errno != ENOENT)
        throw std::runtime_error("cannot inspect preset destination");
}
} // namespace
std::optional<Preset> load_preset(const std::string &directory) {
    Descriptor dir(::open(directory.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    const auto path = anchor(dir.get()) + filename;
    int expected_version = 18;
    int opened = ::open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
    for (const int version : {17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1}) {
        if (opened >= 0 || errno != ENOENT)
            break;
        expected_version = version;
        const auto legacy = anchor(dir.get()) + "parameters.v" + std::to_string(version);
        opened = ::open(legacy.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
    }
    if (opened < 0 && errno == ENOENT)
        return std::nullopt;
    Descriptor file(opened);
    struct stat info{};
    if (::fstat(file.get(), &info) != 0 || !S_ISREG(info.st_mode) || info.st_size < 1 ||
        info.st_size > 4096)
        throw std::runtime_error("invalid preset file type or size");
    std::string data(static_cast<std::size_t>(info.st_size), '\0');
    std::size_t offset = 0;
    while (offset < data.size()) {
        const auto amount = ::read(file.get(), &data[offset], data.size() - offset);
        if (amount < 0 && errno == EINTR)
            continue;
        if (amount <= 0)
            throw std::runtime_error("incomplete preset read");
        offset += static_cast<std::size_t>(amount);
    }
    return decode_preset(data, expected_version);
}

void save_preset(const std::string &directory, const Preset &p) {
    Descriptor dir(::open(directory.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    const auto base = anchor(dir.get());
    const auto target = base + filename;
    const auto pending = base + "parameters.v18.pending";
    check_destination(target);
    const auto data = encode_preset(p);
    Descriptor file(
        ::open(pending.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600));
    // On failure preserve the unfinished transaction and previous preset for recovery.
    std::size_t offset = 0;
    while (offset < data.size()) {
        const auto amount = ::write(file.get(), data.data() + offset, data.size() - offset);
        if (amount < 0 && errno == EINTR)
            continue;
        if (amount <= 0)
            throw std::runtime_error("preset write failed; previous preset preserved");
        offset += static_cast<std::size_t>(amount);
    }
    if (::fsync(file.get()) != 0)
        throw std::runtime_error("preset flush failed; previous preset preserved");
    check_destination(target);
    if (::rename(pending.c_str(), target.c_str()) != 0)
        throw std::runtime_error("preset replacement failed; previous preset preserved");
    if (::fsync(dir.get()) != 0 && errno != EINVAL)
        throw std::runtime_error("preset directory flush failed");
}
} // namespace grainmm
