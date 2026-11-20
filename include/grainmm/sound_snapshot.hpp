// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/instrument_parameters.hpp"
#include <cstdint>
#include <optional>
namespace grainmm {
// Control-thread-only temporary settings. No samples, effect history, transport or
// filesystem state. A successful source replacement must advance source_generation.
class SoundSnapshot {
  public:
    enum class Result { Empty, SourceChanged, Restored };
    void capture(const InstrumentParameters &values, std::uint64_t source_generation) noexcept {
        values_ = bounded(values);
        source_generation_ = source_generation;
    }
    Result recall(std::uint64_t source_generation,
                  InstrumentParameters &destination) const noexcept {
        if (!values_)
            return Result::Empty;
        if (source_generation != source_generation_)
            return Result::SourceChanged;
        destination = *values_;
        return Result::Restored;
    }
    bool captured() const noexcept { return values_.has_value(); }

  private:
    std::optional<InstrumentParameters> values_;
    std::uint64_t source_generation_ = 0;
};
} // namespace grainmm
