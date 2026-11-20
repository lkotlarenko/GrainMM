// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/parameter_schema.hpp"
#include "grainmm/sound_snapshot.hpp"
#include <cstdio>
#include <limits>
int main() {
    using Snapshot = grainmm::SoundSnapshot;
    Snapshot s;
    grainmm::InstrumentParameters before, result;
    before.grain.position = .7F;
    before.grain.pitch = .5F;
    before.reverb.mix = .4F;
    before.collision.enabled = 1;
    before.collision.note = 63;
    result.grain.position = .1F;
    if (s.captured() || s.recall(0, result) != Snapshot::Result::Empty ||
        result.grain.position != .1F)
        return 1;
    s.capture(before, 7);
    before.grain.position = .2F; // The snapshot owns values, never an editor reference.
    if (!s.captured() || s.recall(8, result) != Snapshot::Result::SourceChanged ||
        result.grain.position != .1F)
        return 1;
    if (s.recall(7, result) != Snapshot::Result::Restored || result.grain.position != .7F ||
        result.collision.note != 63 || result.reverb.mix != .4F)
        return 1;
    const auto saved = result;
    result = {};
    if (s.recall(7, result) != Snapshot::Result::Restored)
        return 1;
    for (std::size_t i = 0; i < grainmm::parameter_count; ++i)
        if (grainmm::parameter_definition(i).read(result) !=
            grainmm::parameter_definition(i).read(saved))
            return 1;
    before.grain.pitch = std::numeric_limits<float>::infinity();
    s.capture(before, 8); // Capturing again deliberately replaces the temporary slot.
    if (s.recall(7, result) != Snapshot::Result::SourceChanged ||
        s.recall(8, result) != Snapshot::Result::Restored ||
        result.grain.pitch != grainmm::bounded(before).grain.pitch)
        return 1;
    std::puts("Snapshot: owned bounded values, empty/source rejection, overwrite and all-field "
              "recall passed");
}
