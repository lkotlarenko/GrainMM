// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/drone_scenes.hpp"
#include <stdexcept>
namespace grainmm {
const std::array<DroneScene, 8> &drone_scenes() noexcept {
    static constexpr std::array<DroneScene, 8> scenes{
        {{"DAYBREAK",
          "WARM MAJOR / SLOW RISING CHORDS",
          {0, 60, 1, 16, .3F, .55F, .3F, .45F, 1.2F, 1, 42, 1}},
         {"TIDELINES",
          "OPEN FIFTHS / WIDE GENTLE MOTION",
          {0, 55, 2, 24, .2F, .8F, .2F, .2F, 2, 1, 81, 3}},
         {"GLASSHOUSE",
          "BRIGHT LYDIAN / SCATTERED NOTES",
          {0, 67, 4, 8, .85F, .4F, .65F, 1.1F, .4F, .8F, 314, 0}},
         {"NIGHTGLOW",
          "LOW MINOR / SOFT DRIFTING LIGHT",
          {0, 48, 0, 30, .15F, .45F, .2F, .15F, 2.5F, 1, 128, 0}},
         {"OPEN SKY",
          "AIRY PENTATONIC / FLOATING HARMONY",
          {0, 62, 5, 12, .65F, .7F, .45F, .65F, 1.5F, 1, 256, 3}},
         {"EMBER",
          "DARK CLUSTER / SLOW BREATHING",
          {0, 43, 3, 40, .75F, .9F, .15F, .1F, 3, .6F, 512, 0}},
         {"SLOW ORBIT",
          "MAJOR SIXTH / REVERSE CHORD CYCLE",
          {0, 57, 6, 20, .5F, .85F, .4F, .4F, 2, 1, 999, 2}},
         {"STILL WATER",
          "SUSPENDED CHORD / SPARSE REFLECTIONS",
          {0, 60, 7, 60, .25F, .3F, .35F, .25F, 1, .75F, 1024, 4}}}};
    return scenes;
}
InstrumentParameters apply_drone_scene(InstrumentParameters current, unsigned scene,
                                       unsigned seed) {
    if (scene >= drone_scenes().size() || seed < 1 || seed > 9999)
        throw std::invalid_argument("Invalid drone scene or seed");
    const float level = current.drone_synth.level;
    current.drone_synth = drone_scenes()[scene].sound;
    current.drone_synth.level = level;
    current.drone_synth.seed = static_cast<float>(seed);
    return current;
}
} // namespace grainmm
