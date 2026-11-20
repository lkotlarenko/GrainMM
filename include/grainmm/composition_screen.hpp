// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/instrument_export.hpp"
namespace grainmm {
class Display;
class ParameterEditor;
class SdlAudio;
class Waveform;
// Control-thread controller for the pattern/export screens. The application
// adapter supplies edge-filtered keys; neither controller nor writer runs in audio.
class CompositionScreen {
  public:
    CompositionScreen(Display &display, ParameterEditor &editor, SdlAudio &audio);
    void open_pattern(const Waveform *wave);
    void open_export(std::string directory, const Sample *sample);
    bool input(int key, unsigned multiplier = 1); // Returns a parameter change.
    void draw();
    void close();
    bool active() const noexcept { return view_ != View::Closed; }
    bool rendering() const noexcept { return export_ && !export_->complete(); }
    unsigned exports_completed() const noexcept { return completed_; }
    unsigned exports_cancelled() const noexcept { return cancelled_; }
    unsigned pattern_views() const noexcept { return pattern_views_; }

  private:
    enum class View { Closed, Pattern, Export };
    Display &display_;
    ParameterEditor &editor_;
    SdlAudio &audio_;
    View view_ = View::Closed;
    PatternEditor pattern_;
    const Waveform *wave_ = nullptr;
    const Sample *sample_ = nullptr;
    std::string directory_, message_;
    unsigned duration_ = 1;
    unsigned completed_ = 0, cancelled_ = 0, pattern_views_ = 0;
    std::unique_ptr<InstrumentExport> export_;
};
} // namespace grainmm
