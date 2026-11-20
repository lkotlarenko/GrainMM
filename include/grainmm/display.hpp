// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "grainmm/audio_visuals.hpp"
#include "grainmm/page_navigator.hpp"
#include "grainmm/parameter_schema.hpp"
#include "grainmm/reset_scope.hpp"
#include "grainmm/ui_theme.hpp"
#include <SDL.h>
#include <array>
#include <string>
namespace grainmm {
class ParameterEditor;
class PatchMenu;
class Waveform;
class Display {
  public:
    Display();
    ~Display();
    Display(const Display &) = delete;
    Display &operator=(const Display &) = delete;
    void draw(const std::string &sample, bool playing, const ParameterEditor &editor,
              bool has_sample, const std::string &error = {}, unsigned sequence = 0,
              const Waveform *waveform = nullptr, AudioVisuals visuals = {});
    void draw_navigation(const ParameterEditor &editor, const PageNavigator &navigator);
    void set_theme(unsigned index);
    void set_transport(bool playing) noexcept {
        if (transport_ != playing) {
            transport_ = playing;
            invalidate();
        }
    }
    void draw_style(unsigned selected, const std::string &message);
    void draw_drone_scenes(unsigned selected, unsigned seed, float level);
    void draw_pattern(const InstrumentParameters &parameters, const PatternEditor &editor,
                      const Waveform *wave, unsigned sequence, bool playing);
    void draw_export(unsigned seconds, unsigned progress, const std::string &message,
                     bool rendering);
    unsigned frame_delay() const noexcept;
    unsigned layouts() const noexcept { return layouts_; }
    unsigned scenes() const noexcept { return scenes_; }
    void draw_patches(const PatchMenu &menu);
    void draw_waveform(const Waveform &waveform, const ParameterEditor &editor,
                       const std::string &name, bool playing);
    void draw_help(const ParameterEditor &editor, bool editing = true,
                   bool reset_confirmation = false, ResetScope scope = ResetScope::Modulation);
    void draw_browser(const std::string &path, const std::string &previous,
                      const std::string &selected, const std::string &next,
                      const std::string &message);
    // Control-thread diagnostic: exclusively create a BMP of the current SDL surface.
    // Raw target pixels retain the panel orientation; no automatic capture loop.
    void save_snapshot(const std::string &filename) const;
    void invalidate() noexcept { invalid_ = true; }
    unsigned requests() const noexcept { return requests_; }
    unsigned presentations() const noexcept { return presentations_; }

  private:
    enum Tone : unsigned { Background, Panel, Ink, Muted, Track, Accent, Signal, Red, Dark };
    Uint32 color(Tone t) const noexcept { return palette_[t]; }
    static void copy_pixels(SDL_Surface *from, SDL_Surface *to);
    void rect(int x, int y, int w, int h, Uint32 color);
    void line(int x, int y, int xx, int yy, Uint32 color);
    void circle(int x, int y, int radius, Uint32 color);
    void chrome(const char *title, unsigned group, int playing = -1);
    void footer(const char *primary, const char *secondary);
    void scene(const ParameterEditor &, const Waveform *, bool has_sample, unsigned drone_state);
    void dynamics(const ParameterEditor &, bool playing, unsigned sequence, const AudioVisuals &);
    static std::string label(const std::string &input, std::size_t width);
    void present();
    void credits(Uint32 green, Uint32 white);
    enum class View {
        Parameters,
        Browser,
        Help,
        Patches,
        Waveform,
        Navigation,
        Style,
        DroneScenes,
        Pattern,
        Export
    };
    View view_ = View::Parameters;
    void fill(SDL_Rect rectangle, Uint32 color);
    void text(int x, int y, const std::string &value, Uint32 color, int scale = 3);
    bool transport_ = false;
    bool rotate_ = false;
    bool invalid_ = true;
    unsigned requests_ = 0, presentations_ = 0;
    std::array<std::string, 10> previous_{};
    SDL_Surface *surface_ = nullptr, *backdrop_ = nullptr;
    std::array<Uint32, 9> palette_{};
    std::array<std::string, 10> base_{};
    std::array<float, 2> meters_{};
    std::array<float, 4> drone_levels_{}, event_levels_{};
    std::array<float, parameter_count> cached_values_{};
    SlicePattern cached_pattern_;
    unsigned cached_page_ = 0, cached_selection_ = 0, scenes_ = 0;
    bool cached_sample_ = false, cached_playing_ = false;
    const Waveform *cached_wave_ = nullptr;
    unsigned layouts_ = 0, last_animation_ = 0, last_sequence_ = 0, last_peaks_ = 0;
    AudioVisuals last_visuals_{};
#ifndef GRAINMM_SDL1
    SDL_Window *window_ = nullptr;
#endif
};
} // namespace grainmm
