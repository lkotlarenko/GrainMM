// SPDX-License-Identifier: GPL-3.0-only
#pragma once
namespace grainmm {
enum class ProbeMode {
    None,
    Basic,
    Browser,
    Synth,
    Rack,
    Sample,
    Library,
    PatchFailure,
    Harmony,
    Texture,
    FxReset,
    Shortcut,
    Repeat,
    SoundReset,
    Slice,
    Scene,
    Snapshot,
    Evolve,
    Drone,
    Ambient,
    Pattern,
    Tools,
    Performance
};
// Bounded control-thread diagnostics. Only used after dummy-audio validation.
class LifecycleProbe {
  public:
    explicit LifecycleProbe(ProbeMode mode, unsigned page = 0) : mode_(mode), legacy_page_(page) {}
    void inject(unsigned elapsed_ms);
    bool complete() const noexcept { return stage_ == steps(); }

  private:
    unsigned steps() const noexcept;
    ProbeMode mode_;
    unsigned stage_ = 0, legacy_page_ = 0;
};
} // namespace grainmm
