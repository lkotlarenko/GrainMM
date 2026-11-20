// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/patch_store.hpp"
#include <utility>
namespace grainmm {
enum class PatchKey { Previous, Next, Left, Right, Accept, Save, Name, Back };
enum class PatchAction { None, Load, Save, Close };
// UI-owned dialog; no SDL/files/audio. Disk replacement needs two separate Save
// actions. Adapter suppresses held-button repeats before passing Save/Accept.
class PatchMenu {
  public:
    explicit PatchMenu(std::array<PatchInfo, PatchStore::slots> entries);
    PatchAction input(PatchKey key);
    void refresh(std::array<PatchInfo, PatchStore::slots> entries);
    void message(std::string text) { message_ = std::move(text); }
    const std::string &message() const noexcept { return message_; }
    unsigned selection() const noexcept { return selection_; }
    bool replacing() const noexcept { return replace_; }
    bool naming() const noexcept { return naming_; }
    unsigned cursor() const noexcept { return cursor_; }
    const std::string &name() const noexcept { return name_; }
    std::string edit_text() const { return std::string(edit_.data(), 20); }
    const std::array<PatchInfo, PatchStore::slots> &entries() const noexcept { return entries_; }

  private:
    void select_name();
    std::array<PatchInfo, PatchStore::slots> entries_;
    std::array<char, 20> edit_{};
    std::string name_, message_;
    unsigned selection_ = 0, cursor_ = 0;
    bool naming_ = false, confirm_ = false, replace_ = false;
};
} // namespace grainmm
