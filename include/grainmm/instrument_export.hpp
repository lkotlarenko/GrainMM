// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/instrument_parameters.hpp"
#include <memory>
#include <string>
namespace grainmm {
// UI/offline owner only. Sample must outlive the job; live audio must be paused.
// Starts a fresh deterministic graph, not a recording of live effect history.
// Writes only a new numbered WAV in the explicit state directory's exports child.
class InstrumentExport {
  public:
    InstrumentExport(const std::string &state_directory, const Sample *sample,
                     InstrumentParameters parameters, unsigned seconds, int rate = 48000);
    ~InstrumentExport();
    InstrumentExport(const InstrumentExport &) = delete;
    InstrumentExport &operator=(const InstrumentExport &) = delete;
    void advance(unsigned blocks = 8); // 1..32 bounded blocks of at most 256 frames.
    void cancel(); // Removes only this incomplete file after inode/type verification.
    bool complete() const noexcept;
    unsigned progress() const noexcept;
    unsigned clipped_samples() const noexcept;
    const std::string &filename() const noexcept;

  private:
    struct State;
    std::unique_ptr<State> state_;
};
} // namespace grainmm
