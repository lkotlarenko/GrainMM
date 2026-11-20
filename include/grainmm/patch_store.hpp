// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/preset.hpp"
#include <array>
#include <optional>
namespace grainmm {
struct Patch {
    std::string name;
    Preset preset;
};
struct PatchInfo {
    bool occupied = false;
    bool valid = true;
    std::string name;
};
// Control-thread storage. Fixed16slots under an explicit existing state directory.
// Opens/creates only its patches child; retains a directory fd for padsp safety.
class PatchStore {
  public:
    static constexpr unsigned slots = 16;
    explicit PatchStore(const std::string &state_directory);
    ~PatchStore();
    PatchStore(const PatchStore &) = delete;
    PatchStore &operator=(const PatchStore &) = delete;
    std::optional<Patch> load(unsigned slot) const;
    std::array<PatchInfo, slots> list() const;
    void save(unsigned slot, const Patch &patch, bool replace = false) const;
    static bool valid_name(const std::string &name) noexcept;
    static std::string default_name(unsigned slot);

  private:
    std::string path(unsigned slot) const;
    int directory_ = -1;
};
} // namespace grainmm
