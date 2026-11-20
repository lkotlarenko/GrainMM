// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/sample.hpp"
#include <cstdint>
#include <string>
#include <utility>

namespace grainmm {
// Control thread only. Trusted directory, single relative basename; never follows a file symlink.
// PCM16 RIFF/WAVE only; file <=32 MiB, decoded audio <=16 MiB, <=4096 chunks.
Sample load_wave(const std::string &directory, const std::string &name);

// Offline/control thread only. Exclusively creates output; never overwrites an existing path.
class WaveWriter {
  public:
    WaveWriter(const std::string &path, int rate, int channels, std::uint32_t frames);
    ~WaveWriter();
    WaveWriter(const WaveWriter &) = delete;
    WaveWriter &operator=(const WaveWriter &) = delete;
    void write(const std::int16_t *samples, std::size_t frames);
    void finish();
    // Control-thread identity of the open output, independent of path replacements.
    std::pair<std::uint64_t, std::uint64_t> file_identity() const;

  private:
    int fd_ = -1, channels_;
    std::uint32_t remaining_;
};
} // namespace grainmm
