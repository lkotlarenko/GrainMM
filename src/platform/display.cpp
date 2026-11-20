// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/display.hpp"
#include "grainmm/parameter_editor.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace grainmm {
namespace {
// Independently drawn 5x7 glyphs. No bundled font dependency or upstream asset.
using Glyph = std::array<unsigned char, 7>;
Glyph glyph(char c) {
    switch (c) {
    case '%':
        return {24, 25, 2, 4, 8, 19, 3};
    case '#':
        return {10, 10, 31, 10, 31, 10, 10};
    case 'A':
        return {14, 17, 17, 31, 17, 17, 17};
    case 'B':
        return {30, 17, 17, 30, 17, 17, 30};
    case 'C':
        return {14, 17, 16, 16, 16, 17, 14};
    case 'D':
        return {30, 17, 17, 17, 17, 17, 30};
    case 'E':
        return {31, 16, 16, 30, 16, 16, 31};
    case 'F':
        return {31, 16, 16, 30, 16, 16, 16};
    case 'G':
        return {14, 17, 16, 23, 17, 17, 14};
    case 'H':
        return {17, 17, 17, 31, 17, 17, 17};
    case 'I':
        return {14, 4, 4, 4, 4, 4, 14};
    case 'J':
        return {7, 2, 2, 2, 18, 18, 12};
    case 'K':
        return {17, 18, 20, 24, 20, 18, 17};
    case 'L':
        return {16, 16, 16, 16, 16, 16, 31};
    case 'M':
        return {17, 27, 21, 21, 17, 17, 17};
    case 'N':
        return {17, 25, 25, 21, 19, 19, 17};
    case 'O':
        return {14, 17, 17, 17, 17, 17, 14};
    case 'P':
        return {30, 17, 17, 30, 16, 16, 16};
    case 'Q':
        return {14, 17, 17, 17, 21, 18, 13};
    case 'R':
        return {30, 17, 17, 30, 20, 18, 17};
    case 'S':
        return {15, 16, 16, 14, 1, 1, 30};
    case 'T':
        return {31, 4, 4, 4, 4, 4, 4};
    case 'U':
        return {17, 17, 17, 17, 17, 17, 14};
    case 'V':
        return {17, 17, 17, 17, 17, 10, 4};
    case 'W':
        return {17, 17, 17, 21, 21, 27, 17};
    case 'X':
        return {17, 17, 10, 4, 10, 17, 17};
    case 'Y':
        return {17, 17, 10, 4, 4, 4, 4};
    case 'Z':
        return {31, 1, 2, 4, 8, 16, 31};
    case '0':
        return {14, 17, 19, 21, 25, 17, 14};
    case '1':
        return {4, 12, 4, 4, 4, 4, 14};
    case '2':
        return {14, 17, 1, 2, 4, 8, 31};
    case '3':
        return {30, 1, 1, 14, 1, 1, 30};
    case '4':
        return {2, 6, 10, 18, 31, 2, 2};
    case '5':
        return {31, 16, 16, 30, 1, 1, 30};
    case '6':
        return {14, 16, 16, 30, 17, 17, 14};
    case '7':
        return {31, 1, 2, 4, 8, 8, 8};
    case '8':
        return {14, 17, 17, 14, 17, 17, 14};
    case '9':
        return {14, 17, 17, 15, 1, 1, 14};
    case '-':
        return {0, 0, 0, 31, 0, 0, 0};
    case '.':
        return {0, 0, 0, 0, 0, 12, 12};
    case ':':
        return {0, 12, 12, 0, 12, 12, 0};
    case '/':
        return {1, 2, 2, 4, 8, 8, 16};
    case '?':
        return {14, 17, 1, 2, 4, 0, 4};
    case '>':
        return {16, 8, 4, 2, 4, 8, 16};
    case '_':
        return {0, 0, 0, 0, 0, 0, 31};
    case '+':
        return {0, 4, 4, 31, 4, 4, 0};
    default:
        return {};
    }
}
} // namespace
std::string Display::label(const std::string &input, std::size_t width) {
    std::string result;
    for (const unsigned char raw : input) {
        if (result.size() == width)
            break;
        const char c =
            raw >= 'a' && raw <= 'z' ? static_cast<char>(raw - 'a' + 'A') : static_cast<char>(raw);
        const bool supported = (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                               std::strchr(" ./:-+_>", c) != nullptr;
        result += supported && raw != 0 ? c : '?';
    }
    if (input.size() > width && width >= 3)
        result.replace(width - 3, 3, "...");
    return result;
}
Display::Display() {
#ifdef GRAINMM_SDL1
    // The MM+ panel is mounted upside-down. Its vendor SDL has no 32-bit UD
    // shadow blitter, so transform our drawing rectangles without a frame copy.
    rotate_ = true;
    if (const char *rotation = std::getenv("GRAINMM_DISPLAY_ROTATION")) {
        if (std::strcmp(rotation, "0") != 0 && std::strcmp(rotation, "180") != 0)
            throw std::runtime_error("GRAINMM_DISPLAY_ROTATION must be 0 or 180");
        rotate_ = std::strcmp(rotation, "180") == 0;
    }
    surface_ = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
#else
    window_ =
        SDL_CreateWindow("GrainMM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    if (window_)
        surface_ = SDL_GetWindowSurface(window_);
#endif
    if (!surface_) {
#ifndef GRAINMM_SDL1
        if (window_)
            SDL_DestroyWindow(window_);
#endif
        throw std::runtime_error(SDL_GetError());
    }
    constexpr unsigned char rgb[9][3] = {{16, 18, 20},    {27, 30, 33},  {235, 235, 222},
                                         {142, 151, 156}, {52, 58, 62},  {255, 139, 67},
                                         {94, 214, 198},  {242, 83, 75}, {16, 18, 20}};
    for (unsigned i = 0; i < 9; ++i)
        palette_[i] = SDL_MapRGB(surface_->format, rgb[i][0], rgb[i][1], rgb[i][2]);
    backdrop_ = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, surface_->format->BitsPerPixel,
                                     surface_->format->Rmask, surface_->format->Gmask,
                                     surface_->format->Bmask, surface_->format->Amask);
    if (!backdrop_) {
#ifndef GRAINMM_SDL1
        SDL_DestroyWindow(window_);
#endif
        throw std::runtime_error(SDL_GetError());
    }
    set_theme(0);
    std::printf("display=%dx%d bpp=%u rotation=%d\n", surface_->w, surface_->h,
                surface_->format->BitsPerPixel, rotate_ ? 180 : 0);
}
void Display::fill(SDL_Rect rectangle, Uint32 color) {
    if (rotate_) {
        rectangle.x = static_cast<decltype(rectangle.x)>(surface_->w - rectangle.x - rectangle.w);
        rectangle.y = static_cast<decltype(rectangle.y)>(surface_->h - rectangle.y - rectangle.h);
    }
    SDL_FillRect(surface_, &rectangle, color);
}
Display::~Display() {
    SDL_FreeSurface(backdrop_);
#ifndef GRAINMM_SDL1
    SDL_DestroyWindow(window_);
#endif
}
void Display::text(int x, int y, const std::string &value, Uint32 color, int scale) {
    if (x < 0 || y < 0 || scale < 1 ||
        static_cast<std::size_t>(x) + value.size() * static_cast<unsigned>(scale) * 6 > 640 ||
        y + 7 * scale > 480)
        throw std::runtime_error("UI text exceeds the640x480 display");
    for (const char c : value) {
        const auto rows = glyph(c);
        // Merge adjacent set pixels into row spans. This preserves the exact
        // bitmap (including target rotation) with fewer software fill calls.
        for (int row = 0; row < 7; ++row) {
            int col = 0;
            while (col < 5) {
                if (!(rows[static_cast<std::size_t>(row)] & (1 << (4 - col)))) {
                    ++col;
                    continue;
                }
                const int start = col++;
                while (col < 5 && (rows[static_cast<std::size_t>(row)] & (1 << (4 - col))))
                    ++col;
                rect(x + start * scale, y + row * scale, (col - start) * scale, scale, color);
            }
        }
        x += scale * 6;
    }
}

void Display::copy_pixels(SDL_Surface *from, SDL_Surface *to) {
    if (from->w != to->w || from->h != to->h ||
        from->format->BytesPerPixel != to->format->BytesPerPixel ||
        from->format->Rmask != to->format->Rmask || from->format->Gmask != to->format->Gmask ||
        from->format->Bmask != to->format->Bmask || from->format->Amask != to->format->Amask)
        throw std::runtime_error("Incompatible UI cache surfaces");
    struct Lock {
        SDL_Surface *surface;
        bool needed;
        explicit Lock(SDL_Surface *s) : surface(s), needed(SDL_MUSTLOCK(s) != 0) {
            if (needed && SDL_LockSurface(s) != 0)
                throw std::runtime_error(SDL_GetError());
        }
        ~Lock() {
            if (needed)
                SDL_UnlockSurface(surface);
        }
    };
    Lock source(from), destination(to);
    const auto bytes = static_cast<std::size_t>(from->w) * from->format->BytesPerPixel;
    if (!from->pixels || !to->pixels || static_cast<std::size_t>(from->pitch) < bytes ||
        static_cast<std::size_t>(to->pitch) < bytes)
        throw std::runtime_error("Invalid UI pixel pitch");
    if (static_cast<std::size_t>(from->pitch) == bytes &&
        static_cast<std::size_t>(to->pitch) == bytes)
        std::memcpy(to->pixels, from->pixels, bytes * static_cast<unsigned>(from->h));
    else
        for (int row = 0; row < from->h; ++row)
            std::memcpy(static_cast<unsigned char *>(to->pixels) +
                            static_cast<std::size_t>(row) * to->pitch,
                        static_cast<const unsigned char *>(from->pixels) +
                            static_cast<std::size_t>(row) * from->pitch,
                        bytes);
}
void Display::rect(int x, int y, int w, int h, Uint32 value) {
    if (w <= 0 || h <= 0)
        return;
    SDL_Rect r{};
    r.x = static_cast<decltype(r.x)>(x);
    r.y = static_cast<decltype(r.y)>(y);
    r.w = static_cast<decltype(r.w)>(w);
    r.h = static_cast<decltype(r.h)>(h);
    fill(r, value);
}
void Display::line(int x, int y, int xx, int yy, Uint32 value) {
    const int dx = std::abs(xx - x), sx = x < xx ? 1 : -1, dy = -std::abs(yy - y),
              sy = y < yy ? 1 : -1;
    int error = dx + dy;
    for (;;) {
        rect(x, y, 1, 1, value);
        if (x == xx && y == yy)
            break;
        const int twice = error * 2;
        if (twice >= dy) {
            error += dy;
            x += sx;
        }
        if (twice <= dx) {
            error += dx;
            y += sy;
        }
    }
}
void Display::circle(int x, int y, int radius, Uint32 value) {
    int a = radius, b = 0, error = 1 - radius;
    while (a >= b) {
        for (int sx : {-1, 1})
            for (int sy : {-1, 1}) {
                rect(x + sx * a, y + sy * b, 1, 1, value);
                rect(x + sx * b, y + sy * a, 1, 1, value);
            }
        ++b;
        if (error < 0)
            error += 2 * b + 1;
        else {
            --a;
            error += 2 * (b - a) + 1;
        }
    }
}
void Display::chrome(const char *title, unsigned group, int playing) {
    if (playing >= 0)
        transport_ = playing != 0;
    SDL_FillRect(surface_, nullptr, color(Background));
    text(18, 17, "GRAINMM", color(Ink), 2);
    constexpr int xs[] = {128, 242, 304, 398};
    constexpr int widths[] = {104, 52, 84, 82};
    for (unsigned i = 0; i < 4; ++i) {
        if (i == group)
            rect(xs[i], 10, widths[i], 29, color(Accent));
        text(xs[i] + 10, 18, PageNavigator::group_label(i), color(i == group ? Dark : Muted), 2);
    }
    rect(502, 10, 120, 29, color(Panel));
    text(514, 18, transport_ ? "PLAYING" : "PAUSED", color(transport_ ? Signal : Muted), 2);
    rect(18, 46, 604, 1, color(Track));
    text(18, 60, title, color(Ink), 3);
}
void Display::footer(const char *primary, const char *secondary) {
    rect(18, 432, 604, 1, color(Track));
    text(18, 444, primary, color(Ink), 2);
    text(18, 468, secondary, color(Muted), 1);
}
void Display::credits(Uint32, Uint32) {
    text(18, 414, "GRAINMM BY ELLY / LKOTLARENKO - GRAINBRICK BY ETRNLIAM", color(Muted), 1);
}

void Display::set_theme(unsigned index) {
    if (index >= themes.size())
        index = 0;
    const auto &t = themes[index];
    palette_[Accent] = SDL_MapRGB(surface_->format, t.accent[0], t.accent[1], t.accent[2]);
    palette_[Signal] = SDL_MapRGB(surface_->format, t.signal[0], t.signal[1], t.signal[2]);
    invalidate();
}
unsigned Display::frame_delay() const noexcept {
    const auto elapsed = SDL_GetTicks() - last_animation_;
    return elapsed < 50 ? std::min(20U, 50 - elapsed) : 20U;
}
void Display::save_snapshot(const std::string &filename) const {
    auto *file = std::fopen(filename.c_str(), "wbx");
    if (!file)
        throw std::runtime_error("Snapshot destination must be a new writable file");
    auto *stream = SDL_RWFromFP(file, SDL_TRUE);
    if (!stream) {
        std::fclose(file);
        throw std::runtime_error(SDL_GetError());
    }
    if (SDL_SaveBMP_RW(surface_, stream, 1) != 0)
        throw std::runtime_error(SDL_GetError());
}
void Display::present() {
#ifdef GRAINMM_SDL1
    if (SDL_Flip(surface_) != 0)
#else
    if (SDL_UpdateWindowSurface(window_) != 0)
#endif
        throw std::runtime_error(SDL_GetError());
    ++presentations_;
}
} // namespace grainmm
