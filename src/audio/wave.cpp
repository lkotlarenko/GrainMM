// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/wave.hpp"
#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace grainmm {
namespace {
class File {
  public:
    File(int fd, const char *operation) : fd_(fd) {
        if (fd < 0) {
            const int error = errno;
            throw std::runtime_error(std::string(operation) + ": " + std::strerror(error));
        }
    }
    ~File() { ::close(fd_); }
    File(const File &) = delete;
    File &operator=(const File &) = delete;
    int get() const { return fd_; }

  private:
    int fd_;
};
std::uint16_t u16(const unsigned char *p) {
    return static_cast<std::uint16_t>(p[0] | (static_cast<unsigned>(p[1]) << 8));
}
std::uint32_t u32(const unsigned char *p) {
    return p[0] | (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24);
}
void read_at(int fd, unsigned char *out, std::size_t size, std::uint32_t offset) {
    while (size != 0) {
        const auto count = ::pread(fd, out, size, static_cast<off_t>(offset));
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            throw std::runtime_error("truncated or unreadable WAV");
        const auto n = static_cast<std::size_t>(count);
        out += n;
        size -= n;
        offset += static_cast<std::uint32_t>(n);
    }
}
void write_all(int fd, const unsigned char *data, std::size_t size) {
    while (size != 0) {
        const auto count = ::write(fd, data, size);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            throw std::runtime_error("WAV write failed; output may be incomplete");
        data += count;
        size -= static_cast<std::size_t>(count);
    }
}
void put16(unsigned char *p, std::uint16_t value) {
    p[0] = static_cast<unsigned char>(value);
    p[1] = static_cast<unsigned char>(value >> 8);
}
void put32(unsigned char *p, std::uint32_t value) {
    for (unsigned i = 0; i < 4; ++i)
        p[i] = static_cast<unsigned char>(value >> (8 * i));
}
} // namespace

Sample load_wave(const std::string &directory, const std::string &name) {
    if (name.empty() || name.size() > 128 || name == "." || name == ".." ||
        name.find_first_of("/\\") != std::string::npos ||
        std::any_of(name.begin(), name.end(), [](unsigned char c) { return c < 32 || c == 127; }))
        throw std::invalid_argument("sample name must be a single safe basename");
    const File dir(::open(directory.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW),
                   "open sample directory");
    // Onion's inherited padsp wrapper ignores openat's dirfd. Linux procfs keeps
    // this path anchored to our live directory handle even if its name changes.
    // O_NOFOLLOW still rejects a symlink at the final sample basename.
    const auto anchored = "/proc/self/fd/" + std::to_string(dir.get()) + "/" + name;
    // O_NONBLOCK prevents a FIFO substituted at this path from hanging before fstat.
    const File file(::open(anchored.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK),
                    "open sample file");
    struct stat info{};
    if (::fstat(file.get(), &info) != 0 || !S_ISREG(info.st_mode) || info.st_size < 12 ||
        info.st_size > 32 * 1024 * 1024)
        throw std::runtime_error("WAV must be a regular file between 12 bytes and 32 MiB");
    const auto length = static_cast<std::uint32_t>(info.st_size);
    std::array<unsigned char, 16> header{};
    read_at(file.get(), header.data(), 12, 0);
    if (std::memcmp(header.data(), "RIFF", 4) != 0 ||
        std::memcmp(header.data() + 8, "WAVE", 4) != 0 ||
        static_cast<std::uint64_t>(u32(header.data() + 4)) + 8 != length)
        throw std::runtime_error("expected complete little-endian RIFF/WAVE");
    std::uint32_t rate = 0, data_offset = 0, data_size = 0;
    unsigned channels = 0, chunks = 0;
    bool have_format = false, have_data = false;
    for (std::uint32_t at = 12; at < length;) {
        if (length - at < 8 || ++chunks > 4096)
            throw std::runtime_error("invalid or excessive WAV chunk headers");
        read_at(file.get(), header.data(), 8, at);
        const auto size = u32(header.data() + 4);
        const auto next = static_cast<std::uint64_t>(at) + 8 + size + (size & 1U);
        if (next > length)
            throw std::runtime_error("WAV chunk exceeds RIFF boundary");
        if (std::memcmp(header.data(), "fmt ", 4) == 0) {
            if (have_format || size < 16)
                throw std::runtime_error("duplicate or short WAV format");
            read_at(file.get(), header.data(), 16, at + 8);
            channels = u16(header.data() + 2);
            rate = u32(header.data() + 4);
            if (u16(header.data()) != 1 || (channels != 1 && channels != 2) || rate < 8000 ||
                rate > 96000 || u16(header.data() + 14) != 16 ||
                u16(header.data() + 12) != channels * 2 ||
                u32(header.data() + 8) != rate * channels * 2)
                throw std::runtime_error(
                    "only aligned mono/stereo PCM16 at 8000..96000 Hz is supported");
            have_format = true;
        } else if (std::memcmp(header.data(), "data", 4) == 0) {
            if (have_data)
                throw std::runtime_error("duplicate WAV data chunk");
            have_data = true;
            data_offset = at + 8;
            data_size = size;
        }
        at = static_cast<std::uint32_t>(next);
    }
    if (!have_format || !have_data || data_size == 0 || data_size % (channels * 2) != 0 ||
        data_size / 2 > Sample::max_values)
        throw std::runtime_error("missing, unaligned or oversized WAV audio");
    std::vector<float> values(data_size / 2);
    std::array<unsigned char, 4096> bytes{};
    for (std::uint32_t at = 0; at < data_size;) {
        const auto count =
            std::min<std::uint32_t>(static_cast<std::uint32_t>(bytes.size()), data_size - at);
        read_at(file.get(), bytes.data(), count, data_offset + at);
        for (unsigned i = 0; i < count; i += 2) {
            const int raw = u16(bytes.data() + i);
            values[(at + i) / 2] = static_cast<float>(raw >= 32768 ? raw - 65536 : raw) / 32768.0F;
        }
        at += count;
    }
    return Sample(static_cast<int>(rate), static_cast<int>(channels), std::move(values));
}

WaveWriter::WaveWriter(const std::string &path, int rate, int channels, std::uint32_t frames)
    : channels_(channels), remaining_(frames) {
    if (rate < 8000 || rate > 96000 || (channels != 1 && channels != 2) || frames == 0)
        throw std::invalid_argument("invalid WAV output geometry");
    const auto bytes = static_cast<std::uint64_t>(frames) * static_cast<unsigned>(channels) * 2;
    if (bytes > 0xffffffffULL - 36)
        throw std::invalid_argument("invalid WAV output geometry");
    fd_ = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0644);
    if (fd_ < 0)
        throw std::runtime_error(std::string("cannot exclusively create output: ") +
                                 std::strerror(errno));
    std::array<unsigned char, 44> header{};
    std::memcpy(header.data(), "RIFF", 4);
    put32(header.data() + 4, static_cast<std::uint32_t>(bytes) + 36);
    std::memcpy(header.data() + 8, "WAVEfmt ", 8);
    put32(header.data() + 16, 16);
    put16(header.data() + 20, 1);
    put16(header.data() + 22, static_cast<std::uint16_t>(channels));
    put32(header.data() + 24, static_cast<std::uint32_t>(rate));
    put32(header.data() + 28, static_cast<std::uint32_t>(rate * channels * 2));
    put16(header.data() + 32, static_cast<std::uint16_t>(channels * 2));
    put16(header.data() + 34, 16);
    std::memcpy(header.data() + 36, "data", 4);
    put32(header.data() + 40, static_cast<std::uint32_t>(bytes));
    try {
        write_all(fd_, header.data(), header.size());
    } catch (...) {
        ::close(fd_);
        fd_ = -1;
        throw;
    }
}
WaveWriter::~WaveWriter() {
    if (fd_ >= 0)
        ::close(fd_);
}
std::pair<std::uint64_t, std::uint64_t> WaveWriter::file_identity() const {
    struct stat info{};
    if (fd_ < 0 || ::fstat(fd_, &info) != 0 || !S_ISREG(info.st_mode))
        throw std::runtime_error("cannot identify open WAV output");
    return {static_cast<std::uint64_t>(info.st_dev), static_cast<std::uint64_t>(info.st_ino)};
}
void WaveWriter::write(const std::int16_t *samples, std::size_t frames) {
    if (fd_ < 0 || frames > remaining_ || (frames != 0 && !samples))
        throw std::invalid_argument("WAV write exceeds declared output");
    std::array<unsigned char, 4096> bytes{};
    const auto values = frames * static_cast<unsigned>(channels_);
    for (std::size_t at = 0; at < values;) {
        const auto count = std::min<std::size_t>(bytes.size() / 2, values - at);
        for (std::size_t i = 0; i < count; ++i)
            put16(bytes.data() + i * 2, static_cast<std::uint16_t>(samples[at + i]));
        write_all(fd_, bytes.data(), count * 2);
        at += count;
    }
    remaining_ -= static_cast<std::uint32_t>(frames);
}
void WaveWriter::finish() {
    if (fd_ < 0 || remaining_ != 0)
        throw std::runtime_error("incomplete WAV output");
    const auto fd = fd_;
    fd_ = -1;
    const int flushed = ::fsync(fd);
    const int closed = ::close(fd);
    if (flushed != 0 || closed != 0)
        throw std::runtime_error("WAV flush/close failed; output may be incomplete");
}
} // namespace grainmm
