// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/patch_store.hpp"
#include <cerrno>
#include <fcntl.h>
#include <iomanip>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
namespace grainmm {
namespace {
class Descriptor {
  public:
    explicit Descriptor(int fd) : fd_(fd) {
        if (fd < 0)
            throw std::runtime_error("patch file operation failed");
    }
    ~Descriptor() { ::close(fd_); }
    Descriptor(const Descriptor &) = delete;
    Descriptor &operator=(const Descriptor &) = delete;
    int get() const noexcept { return fd_; }

  private:
    int fd_;
};
std::string anchor(int fd) { return "/proc/self/fd/" + std::to_string(fd) + "/"; }
bool exists_regular(const std::string &path) {
    struct stat info{};
    if (::lstat(path.c_str(), &info) == 0) {
        if (!S_ISREG(info.st_mode))
            throw std::runtime_error("patch destination is not a regular file");
        return true;
    }
    if (errno != ENOENT)
        throw std::runtime_error("cannot inspect patch destination");
    return false;
}
} // namespace
PatchStore::PatchStore(const std::string &state_directory) {
    Descriptor parent(
        ::open(state_directory.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    const auto child = anchor(parent.get()) + "patches";
    if (::mkdir(child.c_str(), 0700) != 0 && errno != EEXIST)
        throw std::runtime_error("cannot create patch directory");
    directory_ = ::open(child.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (directory_ < 0)
        throw std::runtime_error("cannot open safe patch directory");
}
PatchStore::~PatchStore() {
    if (directory_ >= 0)
        ::close(directory_);
}
std::string PatchStore::path(unsigned slot) const {
    if (slot >= slots)
        throw std::out_of_range("patch slot outside1..16");
    return anchor(directory_) + "patch-" + (slot < 9 ? "0" : "") + std::to_string(slot + 1) +
           ".gmm";
}
std::string PatchStore::default_name(unsigned slot) {
    if (slot >= slots)
        throw std::out_of_range("invalid patch slot");
    return "PATCH " + (slot < 9 ? std::string("0") : std::string{}) + std::to_string(slot + 1);
}
bool PatchStore::valid_name(const std::string &name) noexcept {
    if (name.empty() || name.size() > 20 || name.front() == ' ' || name.back() == ' ')
        return false;
    for (unsigned char c : name)
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ' ' || c == '-' || c == '_'))
            return false;
    return true;
}
std::optional<Patch> PatchStore::load(unsigned slot) const {
    const auto filename = path(slot);
    int opened = ::open(filename.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
    if (opened < 0 && errno == ENOENT)
        return std::nullopt;
    Descriptor file(opened);
    struct stat info{};
    if (::fstat(file.get(), &info) != 0 || !S_ISREG(info.st_mode) || info.st_size < 1 ||
        info.st_size > 8192)
        throw std::runtime_error("invalid patch file type or size");
    std::string data(static_cast<std::size_t>(info.st_size), '\0');
    std::size_t at = 0;
    while (at < data.size()) {
        const auto n = ::read(file.get(), &data[at], data.size() - at);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            throw std::runtime_error("incomplete patch read");
        at += static_cast<std::size_t>(n);
    }
    std::istringstream input(data);
    input.imbue(std::locale::classic());
    std::string magic;
    std::getline(input, magic);
    Patch result;
    if (magic != "GrainMM Patch 1" || input.peek() != '"' || !(input >> std::quoted(result.name)) ||
        !valid_name(result.name))
        throw std::runtime_error("corrupt or unsupported patch");
    if (input.get() != '\n')
        throw std::runtime_error("invalid patch header");
    const auto offset = input.tellg();
    if (offset < 0)
        throw std::runtime_error("missing patch parameters");
    result.preset = decode_preset(data.substr(static_cast<std::size_t>(offset)));
    return result;
}
std::array<PatchInfo, PatchStore::slots> PatchStore::list() const {
    std::array<PatchInfo, slots> result{};
    for (unsigned slot = 0; slot < slots; ++slot) {
        try {
            if (const auto patch = load(slot)) {
                result[slot] = {true, true, patch->name};
            }
        } catch (const std::exception &) {
            result[slot] = {true, false, "INVALID PATCH"};
        }
    }
    return result;
}
void PatchStore::save(unsigned slot, const Patch &patch, bool replace) const {
    if (!valid_name(patch.name))
        throw std::runtime_error("patch name must use1..20letters/numbers");
    const auto filename = path(slot), pending = filename + ".pending";
    const auto encoded = encode_preset(patch.preset);
    const auto data = "GrainMM Patch 1\n\"" + patch.name + "\"\n" + encoded;
    if (exists_regular(filename) && !replace)
        throw std::runtime_error("confirm replacing this patch first");
    Descriptor file(
        ::open(pending.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600));
    std::size_t at = 0;
    while (at < data.size()) {
        const auto n = ::write(file.get(), data.data() + at, data.size() - at);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            throw std::runtime_error("patch write failed; previous file preserved");
        at += static_cast<std::size_t>(n);
    }
    if (::fsync(file.get()) != 0)
        throw std::runtime_error("patch flush failed; previous file preserved");
    if (exists_regular(filename) && !replace)
        throw std::runtime_error("patch appeared; previous file preserved");
    if (::rename(pending.c_str(), filename.c_str()) != 0)
        throw std::runtime_error("patch replacement failed");
    if (::fsync(directory_) != 0 && errno != EINVAL)
        throw std::runtime_error("patch directory flush failed");
}
} // namespace grainmm
