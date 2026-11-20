# Next useful instrument work

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


Updated2026-11-19, alpha23. GrainMM by elly(@lkotlarenko); original GrainBrick by
ETRNLIAM. Current goal is a useful sound instrument, not DSP-identical reproduction.

## Corrected ambient direction

The owner clarified that DRONE means **a self-running ambient synthesizer you can
steer and process with our effects**, without loading a sample. Alpha20 implemented
WAV movement instead. That tool is retained as EVOLVE; it does not satisfy the
standalone-source requirement by itself.

The supplied [M5Burner reference](https://burner.m5stack.com/share/firmware/3NRPTY)
is MIASMORA, confirmed by the matching link in its
[author README](https://github.com/oldtokage-collab/MIASMORA). It demonstrates a
sample-free generative soundscape with multiple layers, moods, drift and export.
Reviewed public behavior only. No source/assets/presets copied; GrainMM's DSP is
original. The reference's noncommercial/share-alike terms are not applied to or
included in our GPL implementation.

Alpha21 implements the first standalone DRONE source: four sustained stereo
voices, seeded harmonic changes, smooth tuning/weight transitions, breathing/pan/
pitch movement and quiet air. ROOT/MOOD/EVOLVE SEC/TEXTURE/MOVEMENT steer it, LEVEL
enables it and the shared rack processes it. This is an initial generative pad,
not the reference's full multilayer soundscape/event/recording system. Exact target
acceptance and performance are recorded in RESUME.md and ALPHA21_BASELINE.md.

## What is already useful

- Granular WAV playback:16voices, trim/envelopes, scanning/randomness, harmony.
- EVOLVE: periodic/drift/blended grain-position/pitch/spread modulation and scan hold.
- DRONE: independent self-playing sound source, no sample needed.
- BALLS/BOWL: real wall-collision-excited modal sound; basic oscillator for layering.
- Generated16step sample slicing with an audio-frame clock.
- Shared EQ/filter/compression/tape/crush/chorus/resonance/delay/reverb processing.
- WAV folders/visual trim,16named patches, source-safe temporary capture/recall.
- Grouped studio UI, actual audio telemetry, persistent palettes and held controls.

Snapshot and icon are owner-confirmed. Alpha18 remains the verified rollback until
new physical acceptance. A launch or dummy test cannot establish clean listening.

## Highest-value next batches

| Priority | Deliverable | Why it matters | Acceptance |
| --- | --- | --- | --- |
| Implemented23 | Eight original ambient scenes and visible seed selection | Faster musical starting points | Host apply/cancel/retained-level/save tests pass; native acceptance pending |
| 2 | Additional quiet texture/excitation family and layer balance | Adds atmosphere and events beyond sustained chords | One meaningful engine at a time, explicit voice/energy budget, shared FX, measured native cost |
| 3 | Manual slice patterns and cut editor | Lets users make deliberate beats instead of only generated variations | Per-step region/rest/reverse/probability, cut bounds, variation persistence, stable timing |
| 4 | Full-instrument offline WAV export | Lets users take sounds into songs or share them | Current settings, bounded duration, exclusive filenames, space/cancel checks; no callback writes |
| 5 | General dual/wavetable oscillator, modulation and routing | Closes major GrainBrick synthesis/rack gaps | Original/generated tables, interpolation/alias tests, fixed storage/voices, native headroom |

The DRONE tables are a compact internal sound generator; they do not establish a
full dual-wavetable editor/catalog or input-shaping engine. A few original scenes
should use generated synthesis or a user-selected WAV, never copied reference
samples/presets. Capturing an evolving performance is separate from exporting a
deterministic settings render; label the first exporter honestly.

## Engineering plan

For scene/seed work, define a versioned bounded model separate from the scalar
codec rather than hiding arrays in unused fields. Keep the musical base separate
from automation, prepare expensive design outside audio and preserve failed loads.
Snapshot recall should retain transport; scenes should clearly state source/seed
changes. No sample duplication or implicit file loading behind a performance key.

For manual slicing, reuse immutable sample ownership and the existing two-voice
source. Validate every cut, ramp boundaries and send bounded pattern snapshots.
Time-stretch, automatic onset/tempo detection and external sync are separate tasks.

For export, the current grainmm-render is a basic granular diagnostic. Extend a
portable offline full-instrument renderer first. Any live recorder needs a fixed
producer/writer queue, overflow/backpressure reporting and explicit SD write limits.
Never write audio files in the callback. Keep recordings inside the approved root.

For soundscape textures/routing, prefer one expressive exciter/noise/resonator
family over another large rack. The existing shared effects avoid duplicated heavy
delay/reverb memory. Independent sends and reorderable effects need explicit
ownership/state changes and transition tests. Shimmer needs a genuine pitch-shift
feedback path, filtering and a measured CPU budget; do not relabel an octave tone.

## Performance and release work alongside features

Keep the fixed voice budgets and cached20Hz UI. Native all-FX workloads/scheduling
still show documented outliers. Compare equivalent bounded off/on workloads under
recorded conditions; no clock/mixer/service changes or speculative SIMD rewrite.
Do not infer zeroXRUNs from callback hints. Longer listening/headphones/power and
VNC-off checks remain part of release preparation.

Continue one coherent host→sanitizer→ARM→native→owner-listen batch at a time. Keep
only current plus verified rollback on SD; archive exact builds/logs on the host.
Use logs/private input sequences and owner feedback, not repeated screenshots.
Native source/workflows/binaries stay private under the owner's publication policy.

For broader gaps and reference provenance, see FEATURE_COMPARISON.md and
VIDEO_FEATURE_ROADMAP.md. Live input/MIDI/BLE require verified target capabilities;
none is promised from generic device claims. The next composition batch after alpha23 hardware validation is manual pattern/cut
editing, followed by offline export. Seed/scene starting points are implemented.

Alpha22 adds brighter moods/register, visible seed/patterns and four bounded melodic
event voices. Alpha23 adds eight original transactional starting scenes and master
output trim/limiting. Next useful composition work remains manual slice step/cut
editing, then full-instrument offline export and dual/wavetable synthesis. Source
acceptance/timing gates stay in RESUME.md; presets do not establish full arrangements.
