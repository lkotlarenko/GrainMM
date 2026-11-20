// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/sample_library.hpp"
#include "grainmm/sample_path.hpp"
#include "grainmm/wave.hpp"
#include <algorithm>
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
namespace grainmm {
namespace {
class Descriptor {
  public:
    explicit Descriptor(int value) : value_(value) {
        if (value < 0)
            throw std::runtime_error("Cannot open sample folder");
    }
    ~Descriptor() {
        if (value_ >= 0)
            ::close(value_);
    }
    Descriptor(Descriptor &&other) noexcept : value_(std::exchange(other.value_, -1)) {}
    Descriptor(const Descriptor &) = delete;
    Descriptor &operator=(const Descriptor &) = delete;
    int get() const noexcept { return value_; }

  private:
    int value_;
};
std::string anchor(int fd) { return "/proc/self/fd/" + std::to_string(fd) + "/"; }
bool safe_component(const std::string &name) noexcept {
    return !name.empty() && name.size() <= 128 && name != "." && name != ".." &&
           name.find_first_of("/\\") == std::string::npos &&
           std::all_of(name.begin(), name.end(),
                       [](unsigned char c) { return c >= 32 && c != 127; });
}
bool wave_extension(const std::string &name) noexcept {
    if (name.size() < 5)
        return false;
    const auto at = name.size() - 4;
    return name[at] == '.' && (name[at + 1] == 'w' || name[at + 1] == 'W') &&
           (name[at + 2] == 'a' || name[at + 2] == 'A') &&
           (name[at + 3] == 'v' || name[at + 3] == 'V');
}
std::vector<SampleEntry> scan(int fd) {
    // dup shares a directory offset: rewind the owned stream before each scan.
    const int duplicate = ::fcntl(fd, F_DUPFD_CLOEXEC, 0);
    if (duplicate < 0)
        throw std::runtime_error("Cannot inspect sample folder");
    DIR *opened = ::fdopendir(duplicate);
    if (!opened) {
        ::close(duplicate);
        throw std::runtime_error("Cannot list sample folder");
    }
    const auto closer = [](DIR *directory) { ::closedir(directory); };
    std::unique_ptr<DIR, decltype(closer)> directory(opened, closer);
    ::rewinddir(directory.get());
    std::vector<SampleEntry> result;
    unsigned scanned = 0;
    for (;;) {
        errno = 0;
        const auto *entry = ::readdir(directory.get());
        if (!entry) {
            if (errno != 0)
                throw std::runtime_error("Cannot finish sample listing");
            break;
        }
        if (++scanned > 4096)
            throw std::runtime_error("Too many files - use smaller sample folders");
        const std::string name(entry->d_name);
        if (!safe_component(name) || name.front() == '.')
            continue;
        struct stat info{};
        const auto anchored = anchor(fd) + name;
        if (::lstat(anchored.c_str(), &info) != 0)
            continue; // A concurrently removed entry is not loadable.
        const bool folder = S_ISDIR(info.st_mode);
        if (!folder && !(S_ISREG(info.st_mode) && wave_extension(name)))
            continue;
        if (result.size() == 128)
            throw std::runtime_error("Too many samples - use subfolders");
        result.push_back({name, folder});
    }
    std::sort(result.begin(), result.end(), [](const SampleEntry &a, const SampleEntry &b) {
        return a.directory != b.directory ? a.directory : a.name < b.name;
    });
    return result;
}
} // namespace
struct SampleLibrary::State {
    struct Level {
        Descriptor folder;
        std::string name;
        std::size_t selected = 0;
    };
    std::vector<Level> levels;
    std::vector<SampleEntry> entries;
};
SampleLibrary::SampleLibrary(const std::string &root, bool scan_now)
    : state_(std::make_unique<State>()) {
    std::string pathname = root;
    while (pathname.size() > 1 && pathname.back() == '/')
        pathname.pop_back();
    state_->levels.push_back(
        {Descriptor(::open(pathname.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC)), "",
         0});
    if (scan_now)
        refresh();
}
SampleLibrary::~SampleLibrary() = default;
void SampleLibrary::refresh() {
    auto next = scan(state_->levels.back().folder.get());
    state_->entries = std::move(next);
    auto &selected = state_->levels.back().selected;
    selected = state_->entries.empty() ? 0 : std::min(selected, state_->entries.size() - 1);
}
void SampleLibrary::move(int direction) noexcept {
    const auto size = state_->entries.size();
    if (size && direction) {
        auto &selected = state_->levels.back().selected;
        selected = (selected + (direction < 0 ? size - 1 : 1)) % size;
    }
}
bool SampleLibrary::enter() {
    if (state_->entries.empty() || !state_->entries[selected()].directory)
        return false;
    if (state_->levels.size() >= 8)
        throw std::runtime_error("Sample folder depth limit reached");
    const auto name = state_->entries[selected()].name;
    const auto pathname = anchor(state_->levels.back().folder.get()) + name;
    Descriptor next(::open(pathname.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    auto entries = scan(next.get());
    state_->levels.push_back({std::move(next), name, 0});
    state_->entries = std::move(entries);
    return true;
}
bool SampleLibrary::parent() {
    if (state_->levels.size() == 1)
        return false;
    auto entries = scan(state_->levels[state_->levels.size() - 2].folder.get());
    state_->levels.pop_back();
    state_->entries = std::move(entries);
    auto &selected = state_->levels.back().selected;
    selected = state_->entries.empty() ? 0 : std::min(selected, state_->entries.size() - 1);
    return true;
}
const std::vector<SampleEntry> &SampleLibrary::entries() const noexcept { return state_->entries; }
std::size_t SampleLibrary::selected() const noexcept { return state_->levels.back().selected; }
std::string SampleLibrary::path() const {
    std::string result;
    for (std::size_t i = 1; i < state_->levels.size(); ++i) {
        if (!result.empty())
            result += '/';
        result += state_->levels[i].name;
    }
    return result;
}
std::string SampleLibrary::selected_path() const {
    if (state_->entries.empty())
        return {};
    const auto prefix = path();
    return prefix.empty() ? state_->entries[selected()].name
                          : prefix + '/' + state_->entries[selected()].name;
}
bool SampleLibrary::safe_relative(const std::string &name) noexcept {
    return safe_sample_path(name);
}
Sample SampleLibrary::load(const std::string &relative_name) const {
    if (!safe_relative(relative_name))
        throw std::invalid_argument("Invalid sample selection path");
    auto directory = std::make_unique<Descriptor>(
        ::fcntl(state_->levels.front().folder.get(), F_DUPFD_CLOEXEC, 0));
    std::size_t start = 0;
    for (;;) {
        const auto end = relative_name.find('/', start);
        if (end == std::string::npos)
            return load_wave(anchor(directory->get()) + '.', relative_name.substr(start));
        const auto component = relative_name.substr(start, end - start);
        const auto pathname = anchor(directory->get()) + component;
        auto next = std::make_unique<Descriptor>(
            ::open(pathname.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
        directory = std::move(next);
        start = end + 1;
    }
}
} // namespace grainmm
