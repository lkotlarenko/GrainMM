// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace grainmm {
struct CollisionParameters {
    float enabled = 0, count = 4, gravity = 1, rotation = 0, bounce = .8F, level = .06F;
    float note = 60, scale = 0, octave = 0, decay = .8F, brightness = .4F, excite = .7F;
};
CollisionParameters bounded(CollisionParameters p) noexcept;
struct CollisionMode {
    float coefficient = 0, radius2 = 0, impulse = 0;
};
struct PreparedCollision {
    CollisionParameters values;
    std::array<std::array<CollisionMode, 2>, 6> notes{};
};
PreparedCollision prepare_collision(CollisionParameters p, int rate);
// Fixed120Hz audio-owned hexagon physics; collisions excite eight two-mode voices.
class CollisionSynth {
  public:
    explicit CollisionSynth(int rate);
    void apply(const PreparedCollision &p) noexcept;
    void add(float *out, std::size_t frames, int channels, bool playing) noexcept;
    unsigned hits() const noexcept { return hits_; }
    bool enabled() const noexcept { return target_.values.enabled != 0; }
    std::array<unsigned, 6> positions() const noexcept;

  private:
    struct Ball {
        float x = 0, y = 0, vx = 0, vy = 0;
    };
    struct Voice {
        std::array<float, 2> a{}, b{};
        std::array<CollisionMode, 2> modes{};
    };
    void reset_physics() noexcept;
    void physics() noexcept;
    void strike(unsigned note, float energy) noexcept;
    int rate_;
    PreparedCollision target_;
    std::array<Ball, 6> balls_{};
    std::array<Voice, 8> voices_{};
    unsigned accumulator_ = 0, hits_ = 0;
    float phase_ = 0, gain_ = 0;
    bool playing_ = false;
};
} // namespace grainmm
