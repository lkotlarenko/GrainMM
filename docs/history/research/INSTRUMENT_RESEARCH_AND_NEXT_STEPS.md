# Instrument comparison and recommended next work

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


Research: 2026-11-19. GrainMM by elly (@lkotlarenko); original GrainBrick by ETRNLIAM.
This report compares observed primary documentation with the actual local native
implementation. Recommendations are engineering judgments, not promises or parity
percentages. A listed upstream feature is not proof of its performance on Miyoo.

## Where GrainMM stands

Update after the research sprint: alpha25 now implements a single editable
16-step pattern with cuts and full-instrument offline WAV export. See
[its baseline](../baselines/ALPHA25_BASELINE.md) for tests and the requested navigation
follow-up. The original comparison below informed that implementation.

The device now has alpha24 after bounded native checks, incorporating alpha23
OUTPUT/scenes and subsequent hardening. The owner confirms readability/responsiveness,
clean audio and saved settings. See [the handoff](../../development/RESUME.md) for exact evidence.
The native engine already has granular playback/harmony, sample evolution, generated
slices, basic synthesis, collision synthesis, a generative pad/event source and a
substantial shared rack. WAV folders, trim, named patches and temporary snapshots
are present. The main weakness is completing musical workflows: deliberate pattern
creation, capturing results, deeper synthesis and validated performance under load.

Adding more effect names alone will not close that gap. Bitcrusher, filter, chorus,
tape, compression, tuned resonance, rhythm, delay and reverb already exist locally.

## What the references teach

### GrainBrick — close the instrument workflows

The author's current README describes extended grain tuning, manual rhythmic sample
work, dual wavetable modes, richer ORBIX excitation, tempo tools, live input and MIDI,
plus multiple visualizers. Its output boost is also relevant to listening usability.
GrainMM has practical equivalents for some source/effect functions, not those full
workflows or algorithms. The local preserved Beta2 payload informed the earlier audit;
the public repository is not a complete rebuildable DSP implementation.
[Primary README](https://github.com/ETRNLIAM/GrainBrick).

Our gaps: editable slice patterns/cuts, independent source routing, dual wavetable
synthesis, expanded resonator models, richer tempo/scale work and complete export.
Hardware input/MIDI/BLE capabilities require investigation on this platform first.

### PocketTracker — make an idea finishable

PocketTracker documents a small-screen phrase/chain/song workflow, per-step edits,
sample manipulation, mixing, resampling and WAV/stem export. It provides a separate
Miyoo package. Its capability list spans platforms; it is not a measured budget for
GrainMM. Useful UI lessons are predictable modifiers, local help, visible playback
position and clear sound/mix states.
[Primary README](https://github.com/conanizer/pockettracker).

Its detailed feature guide describes per-step automation, groove/scale tools and
quantized live launching. Those suggest composition controls with explicit musical
timing. We should adopt a small coherent subset around our existing slice clock,
not turn GrainMM into an eight-track tracker. Master processing and sends are useful
references; branded bus effects are not substitutes for defined DSP behavior.
[Feature guide](https://github.com/conanizer/pockettracker/blob/main/docs/features.md).

### MIASMORA — audible evolution and a way to keep it

The author's overview describes sample-free layers, evolving intensity/drift and
WAV/stem export. GrainMM's four pad/four event voices and starting scenes address
only part of that soundscape workflow. A slow intensity arc, controllable event
families and offline export would add value without requiring a sample recorder.
This is behavioral inspiration only; no code/assets or presets were imported.
[Primary project](https://github.com/oldtokage-collab/MIASMORA).

### Little Piggy Tracker — small-screen discipline and platform honesty

LGPT documents a long-standing handheld tracker interface and Miyoo support. Its
platform table explicitly distinguishes MIDI availability rather than assuming
that every device has the same capabilities. Learn navigation consistency,
project compatibility and clear documentation; do not infer USB/MIDI support for
GrainMM from another platform's feature list.
[Primary project](https://github.com/djdiskmachine/LittleGPTracker).

### Teenage Engineering PO-33 — immediate performance actions

The official guide pairs a compact sequencer with contextual controls, step entry,
pattern operations and momentary effects. The useful lesson is a clear relationship
between action and musical result. GrainMM can offer quantized retrigger/reverse or
scene transitions once its pattern model exists. Copying its branding/graphics or
hiding more shortcuts would not solve usability.
[Official guide](https://teenage.engineering/guides/po-33/en).

## Priority and implementation approach

| Order | Deliverable | Existing foundation | Concrete completion gate |
| --- | --- | --- | --- |
| 0 | Complete remaining release validation | Owner-accepted alpha24, rollback, bounded probes | Longer stress/headphone/power checks and comparable resource evidence with VNC off |
| 1 | Manual 16-step slice editor and cut boundaries | Q48 frame clock, immutable WAV, two slice voices | Edit region/rest/reverse/chance, hear exact steps, persist and recover patterns |
| 2 | Full-instrument offline WAV export | Portable Instrument, source/state codecs, diagnostic writer | Render sources + rack + master with explicit duration, exclusive filename, cancel/space checks |
| 3 | Dual/generated-wavetable synth | Basic oscillator and prepared tables | Two bounded voices/banks, morph/tuning/modulation, alias/continuity tests and native CPU limits |
| 4 | Ambient intensity arc and additional exciters | DRONE seed/progression/events and original scenes | Steerable repeatable evolution, clear base/effective values, bounded event/energy budgets |
| 5 | Rack order and lightweight sends | Independent stateful processors in a fixed graph | Explicit topology model, stable transitions, preserved state, measured before/after cost |
| 6 | Richer resonator and shimmer | Modal collision voices and shared space | Genuine model/pitch-feedback behavior, stability/headroom and controlled target profiling |

### Pattern model

Do not hide step arrays in spare scalar fields. Define a fixed-size pattern value
with validated region indexes/rests, reverse and probability. Send immutable prepared
snapshots through the existing bounded queue; keep sample ownership unchanged.
A separate versioned pattern payload and explicit named-patch migration are needed.
Begin with one pattern, then add variations after persistence and editing are reliable.

The editor should show the sounding step separately from the cursor. Prefer normal
D-pad navigation and an explicit edit action, with visible context. Keep existing
A/play behavior unless an intentional, documented control migration is tested.
No audio-frame scheduling should depend on UI refresh or key repeat.

### Export before live recording

Extend the portable offline graph to render the selected WAV/built-in sources,
all effects and OUTPUT. The current grainmm-render is a granular diagnostic, not a
full mix exporter. Use bounded durations, a stopped/offline owner, exclusive output
creation and available-space checks. Keep files in the approved app directory.
Label deterministic settings rendering separately from recording a live performance.
A recorder would additionally need a fixed producer/writer queue and overflow policy;
writing files in the callback remains prohibited.

### Synthesis and space

Generate or license tables explicitly; keep a small fixed bank, interpolate safely
and select band-limited data where appropriate. Prepare costly pitch/filter data
outside audio. Cap voice count before adding modulation. Retain shared delay/reverb
storage until measured routing requirements justify more memory. Shimmer requires
a real pitch-shifting feedback path and filtering; an octave oscillator is not enough.

## Performance and QA strategy

The UI pass removes repeated scene work for selection changes, renders only the two
affected cards, merges glyph fill spans and computes slider bounds within the
relevant parameter group. Typed cache keys also fix stale hidden-parameter scenes.
MIX uses percentage presentation without changing stored/DSP units. The 20 Hz cap,
raw software cache copy and physical rotation are retained; no extra full surface
or visualizer DSP was added. Exact results are in UI_QA_ALPHA24.md.

Next profiling should isolate combined-FX callback work from scheduling delay under
matched workloads. Record stock clock policy, background processes, SSH/VNC state,
block sizes and software clip/late counters. Lateness is not a hardware XRUN count.
Do not change system services/mixer/governor or claim host speedups as Miyoo results.
Avoid an FFT visualizer while basic source activity and pattern tools need attention.

## Suggested release sequence

1. Preserve accepted alpha25; finish longer/matched native profiling.
2. Keep the implemented manual pattern/persistence/export workflows stable.
3. Add dual/generated-wavetable synthesis with bounded voice/headroom measurements.
4. Extend patterns, ambient steering and routing in focused musical increments.

Keep the README short and user-facing; link detailed controls and development here.
Preserve author credits and original/third-party terms. No reference code or assets
were copied for this report or UI work. Native publication remains subject to the
owner's existing documentation-only policy and technical release gates.
