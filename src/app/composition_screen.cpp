// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/composition_screen.hpp"
#include "grainmm/display.hpp"
#include "grainmm/parameter_editor.hpp"
#include "grainmm/sdl_audio.hpp"
#include <SDL.h>
#include <stdexcept>
namespace grainmm {
namespace {
constexpr unsigned durations[] = {5, 15, 30, 60};
}
CompositionScreen::CompositionScreen(Display &display, ParameterEditor &editor, SdlAudio &audio)
    : display_(display), editor_(editor), audio_(audio) {}
void CompositionScreen::open_pattern(const Waveform *wave) {
    ++pattern_views_;
    wave_ = wave;
    view_ = View::Pattern;
    display_.invalidate();
}
void CompositionScreen::open_export(std::string directory, const Sample *sample) {
    audio_.fade_and_pause();
    directory_ = std::move(directory);
    sample_ = sample;
    message_ = "CHOOSE DURATION / A TO RENDER";
    export_.reset();
    view_ = View::Export;
    display_.invalidate();
}
void CompositionScreen::close() {
    if (rendering())
        ++cancelled_;
    export_.reset(); // Destructor removes only its own unfinished output.
    if (view_ == View::Export)
        audio_.start(); // Playback remains muted.
    view_ = View::Closed;
    display_.invalidate();
}
bool CompositionScreen::input(int key, unsigned multiplier) {
    if (key == SDLK_LCTRL) {
        if (export_ && !export_->complete()) {
            ++cancelled_;
            try {
                export_->cancel();
                export_.reset();
                message_ = "CANCELLED / PARTIAL FILE REMOVED";
            } catch (const std::exception &e) {
                export_.reset();
                message_ = e.what();
            }
        } else
            close();
        return false;
    }
    if (view_ == View::Pattern) {
        if (key == SDLK_SPACE) {
            audio_.set_playing(!audio_.playing());
            return false;
        }
        PatternKey action;
        switch (key) {
        case SDLK_e:
            action = PatternKey::Previous;
            break;
        case SDLK_t:
            action = PatternKey::Next;
            break;
        case SDLK_UP:
            action = PatternKey::Up;
            break;
        case SDLK_DOWN:
            action = PatternKey::Down;
            break;
        case SDLK_LEFT:
            action = PatternKey::Left;
            break;
        case SDLK_RIGHT:
            action = PatternKey::Right;
            break;
        case SDLK_LSHIFT:
            action = PatternKey::Rest;
            break;
        case SDLK_LALT:
            action = PatternKey::Mode;
            break;
        default:
            return false;
        }
        auto values = editor_.snapshot();
        if (!pattern_.input(action, values.slice_pattern,
                            static_cast<unsigned>(values.slice.regions), multiplier))
            return false;
        if (values.slice_pattern.manual && values.slice.mode == 0)
            values.slice.mode = 1;
        editor_.restore(values);
        display_.invalidate();
        return true;
    }
    if (view_ == View::Export) {
        if (export_ && !export_->complete())
            return false;
        if (key == SDLK_LEFT || key == SDLK_RIGHT) {
            duration_ = (duration_ + (key == SDLK_LEFT ? 3U : 1U)) % 4;
            export_.reset();
            message_ = "A TO RENDER A NEW WAV";
        }
        if (key == SDLK_SPACE) {
            try {
                export_.reset();
                export_ = std::make_unique<InstrumentExport>(
                    directory_, sample_, editor_.snapshot(), durations[duration_]);
                message_ = "RENDERING / B CANCELS";
            } catch (const std::exception &e) {
                message_ = e.what();
            }
        }
    }
    return false;
}
void CompositionScreen::draw() {
    if (view_ == View::Pattern) {
        display_.draw_pattern(editor_.snapshot(), pattern_, wave_, audio_.sequence_view(),
                              audio_.playing());
        return;
    }
    if (view_ != View::Export)
        return;
    if (export_ && !export_->complete()) {
        try {
            export_->advance();
            if (export_->complete()) {
                ++completed_;
                message_ = export_->filename() +
                           (export_->clipped_samples() ? " / CLIPS: LOWER OUTPUT" : " / SAVED");
            }
        } catch (const std::exception &e) {
            export_.reset();
            message_ = e.what();
        }
    }
    display_.draw_export(durations[duration_], export_ ? export_->progress() : 0, message_,
                         export_ && !export_->complete());
}
} // namespace grainmm
