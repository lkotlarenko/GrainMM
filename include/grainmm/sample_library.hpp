// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/sample.hpp"
#include <memory>
#include <string>
#include <vector>
namespace grainmm {
struct SampleEntry {
    std::string name;
    bool directory;
};
// Read-only library under an explicitly chosen root. No file changes. Reject
// the root final symlink and all relative symlink components; bound scanning,
// names, entries and depth. Control thread only.
class SampleLibrary {
  public:
    explicit SampleLibrary(const std::string &root, bool scan_now = true);
    ~SampleLibrary();
    SampleLibrary(const SampleLibrary &) = delete;
    SampleLibrary &operator=(const SampleLibrary &) = delete;
    void refresh();
    void move(int direction) noexcept;
    bool enter();  // Selected directory; false for a file/empty library.
    bool parent(); // False at root; no escape above it.
    const std::vector<SampleEntry> &entries() const noexcept;
    std::size_t selected() const noexcept;
    std::string path() const;
    std::string selected_path() const;
    Sample load(const std::string &relative_name) const;
    static bool safe_relative(const std::string &name) noexcept;

  private:
    struct State;
    std::unique_ptr<State> state_;
};
} // namespace grainmm
