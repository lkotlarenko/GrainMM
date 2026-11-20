// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/patch_menu.hpp"
#include <algorithm>
#include <string_view>
namespace grainmm {
PatchMenu::PatchMenu(std::array<PatchInfo, PatchStore::slots> entries)
    : entries_(std::move(entries)) {
    select_name();
}
void PatchMenu::select_name() {
    name_ = entries_[selection_].occupied && entries_[selection_].valid
                ? entries_[selection_].name
                : PatchStore::default_name(selection_);
}
void PatchMenu::refresh(std::array<PatchInfo, PatchStore::slots> entries) {
    entries_ = std::move(entries);
    confirm_ = false;
    select_name();
}
PatchAction PatchMenu::input(PatchKey key) {
    replace_ = false;
    if (naming_) {
        constexpr std::string_view alphabet = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";
        if (key == PatchKey::Back) {
            naming_ = false;
            message_.clear();
            return PatchAction::None;
        }
        if (key == PatchKey::Left)
            cursor_ = (cursor_ + 19) % 20;
        if (key == PatchKey::Right)
            cursor_ = (cursor_ + 1) % 20;
        if (key == PatchKey::Previous || key == PatchKey::Next) {
            auto at = alphabet.find(edit_[cursor_]);
            if (at == std::string_view::npos)
                at = 0;
            const auto jump = key == PatchKey::Previous ? alphabet.size() - 1 : 1;
            edit_[cursor_] = alphabet[(at + jump) % alphabet.size()];
        }
        if (key == PatchKey::Accept) {
            std::string text(edit_.data(), edit_.size());
            const auto first = text.find_first_not_of(' '), last = text.find_last_not_of(' ');
            if (first == std::string::npos) {
                message_ = "NAME NEEDS A LETTER OR NUMBER";
                return PatchAction::None;
            }
            text = text.substr(first, last - first + 1);
            if (!PatchStore::valid_name(text)) {
                message_ = "INVALID PATCH NAME";
                return PatchAction::None;
            }
            name_ = std::move(text);
            naming_ = false;
            message_ = "X SAVES THIS NAME AND CURRENT SOUND";
        }
        return PatchAction::None;
    }
    if (key == PatchKey::Save) {
        if (entries_[selection_].occupied && !confirm_) {
            confirm_ = true;
            message_ = "X AGAIN: REPLACE PATCH   B: CANCEL";
            return PatchAction::None;
        }
        replace_ = entries_[selection_].occupied;
        confirm_ = false;
        return PatchAction::Save;
    }
    if (key == PatchKey::Back && confirm_) {
        confirm_ = false;
        message_.clear();
        return PatchAction::None;
    }
    confirm_ = false;
    message_.clear();
    if (key == PatchKey::Back)
        return PatchAction::Close;
    if (key == PatchKey::Previous || key == PatchKey::Next) {
        selection_ = (selection_ + (key == PatchKey::Previous ? PatchStore::slots - 1 : 1)) %
                     PatchStore::slots;
        select_name();
    }
    if (key == PatchKey::Name) {
        edit_.fill(' ');
        std::copy(name_.begin(), name_.end(), edit_.begin());
        cursor_ = 0;
        naming_ = true;
        message_ = "NAME FOR NEXT SAVE. A: ACCEPT B: CANCEL";
    }
    if (key == PatchKey::Accept) {
        if (!entries_[selection_].occupied)
            message_ = "EMPTY SLOT - X TO SAVE CURRENT SOUND";
        else if (!entries_[selection_].valid)
            message_ = "INVALID PATCH - KEEP A BACKUP";
        else
            return PatchAction::Load;
    }
    return PatchAction::None;
}
} // namespace grainmm
