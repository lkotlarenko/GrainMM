// SPDX-License-Identifier: GPL-3.0-only
#pragma once

namespace grainmm {
// Process-local platform defaults and SDL lifetime. Own this before Display/SdlAudio.
class SdlRuntime {
  public:
    SdlRuntime();
    ~SdlRuntime();
    SdlRuntime(const SdlRuntime &) = delete;
    SdlRuntime &operator=(const SdlRuntime &) = delete;
};
} // namespace grainmm
