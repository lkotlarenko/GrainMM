// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/ui_theme.hpp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
namespace grainmm {
namespace {
class Fd {
  public:
    explicit Fd(int n) : n_(n) {
        if (n < 0)
            throw std::runtime_error("UI preference file unavailable");
    }
    Fd(const Fd &) = delete;
    Fd &operator=(const Fd &) = delete;
    ~Fd() { ::close(n_); }
    int get() const { return n_; }

  private:
    int n_;
};
std::string anchor(int fd) { return "/proc/self/fd/" + std::to_string(fd) + "/"; }
void check_file(const std::string &p) {
    struct stat st{};
    if (::lstat(p.c_str(), &st) == 0) {
        if (!S_ISREG(st.st_mode))
            throw std::runtime_error("Unsafe UI preference destination");
    } else if (errno != ENOENT)
        throw std::runtime_error("Cannot inspect UI preference destination");
}
} // namespace
unsigned load_theme(const char *directory) {
    Fd dir(::open(directory, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    const auto path = anchor(dir.get()) + "theme.v1";
    const int opened = ::open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
    if (opened < 0 && errno == ENOENT)
        return 0;
    Fd file(opened);
    struct stat st{};
    if (::fstat(file.get(), &st) != 0 || !S_ISREG(st.st_mode) || st.st_size != 14)
        throw std::runtime_error("Invalid UI preference file");
    std::string data(14, '\0');
    std::size_t at = 0;
    while (at < data.size()) {
        auto n = ::read(file.get(), data.data() + at, data.size() - at);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            throw std::runtime_error("Incomplete UI preferences");
        at += static_cast<std::size_t>(n);
    }
    if (data.substr(0, 12) != "GrainMM UI1\n" || data[13] != '\n' || data[12] < '0' ||
        data[12] >= '0' + static_cast<int>(themes.size()))
        throw std::runtime_error("Corrupt UI preferences");
    return static_cast<unsigned>(data[12] - '0');
}
void save_theme(const char *directory, unsigned theme) {
    if (theme >= themes.size())
        throw std::runtime_error("Invalid UI theme");
    Fd dir(::open(directory, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    const auto base = anchor(dir.get()), path = base + "theme.v1",
               pending = base + "theme.v1.pending";
    check_file(path);
    const std::string data = "GrainMM UI1\n" + std::to_string(theme) + "\n";
    Fd out(::open(pending.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0644));
    std::size_t at = 0;
    while (at < data.size()) {
        auto n = ::write(out.get(), data.data() + at, data.size() - at);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            throw std::runtime_error("UI preference write failed");
        at += static_cast<std::size_t>(n);
    }
    if (::fsync(out.get()) != 0 || ::rename(pending.c_str(), path.c_str()) != 0 ||
        ::fsync(dir.get()) != 0)
        throw std::runtime_error("UI preference commit failed");
}
} // namespace grainmm
