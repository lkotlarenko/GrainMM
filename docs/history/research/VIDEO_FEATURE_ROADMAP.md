# GrainBrick video review and next development batches

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


Reviewed2026-11-18 against owner-supplied transcript and preserved local reference.
Video: https://www.youtube.com/watch?v=fdpsIyv9rx0 . The page could not be fetched
in this environment; no video frames or audio were inspected. Descriptions below
come from the supplied transcript, cross-checked where possible against the local
Beta2 reference. The video's release version/date is not independently verified.
No upstream code/assets were copied. Credit remains GrainBrick by ETRNLIAM and
GrainMM design/native implementation/Miyoo adaptation by elly (@lkotlarenko).

## Product direction

More effect pages alone will not close the gap. Deliver playable workflows with
visible feedback: sequence a sample, see its active region/beat, experiment and
recall a sound, then play a separate collision-excited instrument. Keep high-cost
visualizers optional rather than leaving every screen static until feature parity.
Preserve reliable A play/pause and MENU save/exit on Onion; the transcript's A-edit,
MENU-rewind and START+SELECT-exit mappings conflict with our accepted controls.
Do not silently change them while fixing parameter order. Any new edit mode needs
one coherent control specification and physical testing, not a mix of both schemes.

## Comparison and concrete implementation

| Transcript feature | Verified GrainMM gap | Implementation direction |
| --- | --- | --- |
| Consistent controls/editing | Wet/dry MIX appeared in1st/3rd/5th/6th positions | Implemented display-only order: MIX sixth/bottom-right on all eight wet/dry pages; output/makeup fifth when present. Core shaping first, secondary shaping next; EQ stays low→high. Storage order unchanged |
| Temporary snapshot/recall | Named patches exist; no quick undo point | Single bounded parameter snapshot, no WAV duplication or SD writes. Keep play/pause independent; handle changed sources explicitly. SELECT+A/B are candidates only after chord/tap arbitration tests; show captured/empty/recalled feedback |
| Three grain pages/tuning | Core/motion/harmony/trim exist; no sample-note-aware tuning | Group navigation by source/effects/perform. Add sample root/scale mapping with independent pitch-ratio tests; no key-detection claims |
| Rhythm editor/regions/variations | RHYTHM is only tremolo/autopan | Separate slice model and audio-owned sequencer, shared sample buffer, per-step region/reverse/probability and variation data; visual step/region cursor follows audio clock |
| Global BPM/metronome | One local LFO tempo; no shared musical clock | Audio-frame clock with fractional phase, no drift across block sizes; tempo edits without phase jumps. Metronome defaults off, bounded gain; later tempo-synced delay/LFO |
| Link/tempo matching | Grain pitch/scan are not independent time stretching | Define source BPM and target duration first; evaluate bounded granular duration control and its quality before onset detection or a new stretch library. Do not confuse this “Link” with verified Ableton Link support |
| Dual wavetable/shape/synth/hybrid | One basic-wave oscillator | Second oscillator and original generated tables/morph, fixed voice/storage budget, then optional input shaping. Bass-follow needs a real detector with cost/latency tests |
| Orbix bouncing balls/resonator | Current RESONATE is a single processing bank | Real bounded collision excitation feeding an independent resonator source. Ball state is the instrument state, not random animation pretending to drive audio. Add excitation families/voice control incrementally |
| Orbix independent FX/send | One shared fixed rack | Explicit second source/bus and gain budget; selective independent effects/send before duplicating memory-heavy reverb/delay. Document which controls are shared |
| Rearrangeable rack | Graph fixed at compile time | Fixed-size validated effect-ID order with each module once, preserve ownership/history and serialize separately from scalar values; smooth/mute the reconfiguration. Test noncommutative order and no dropped/duplicate processors |
| MIDI clock/USB/BLE manager | No verified MIDI input/backend or Bluetooth capability | Capability discovery and optional adapters later. Wi-Fi does not establish BLE support; no fake device screens |

## Immediate completed usability work

UI presentation now has its own validated six-position permutation per page.
The typed schema still owns the original labels/accessors/codec order; the editor
maps display positions for values, choices, bounds, editing and reset together.
No preset-version bump or saved-value migration is needed. Full SDL probes were
updated to press the actual new positions; codec golden fixtures remain independent.
No additional screenshot inspection is needed for this deterministic reorder.

## Next coherent batch: rhythm performance and visible feedback

1. Add an audio-frame musical clock and immutable slice-pattern model. Start with
   equal regions,16steps and bounded variations; keep a path to manual boundaries.
   Pattern changes become prepared bounded messages, never file/UI work in audio.
2. Render slice playback with attack/release ramps, reverse and deterministic step
   probability. Reuse the sample; keep the total grain/voice budget explicit.
   Ordinary grains and slicing must have an explicit mode relationship, not both
   accidentally scheduling unlimited voices.
3. Add a pattern editor with active step/region on the waveform and a compact
   performance view. Publish decimated audio telemetry through bounded lock-free
   values/queue; UI never reads mutable DSP voices or copies a sample per frame.
4. Include the temporary parameter snapshot if it fits this batch cleanly; no
   source-file loading or deletion behind a recall chord. Add visible feedback
   for empty/captured/recalled state and test input conflicts with help/reset.
5. Run focused clock/slice references while developing; then host/sanitizer/ARM,
   native bounded fixtures and one combined owner play/navigation/save check.
   Keep a candidate only after it is musically usable, not one version per widget.

Clock acceptance: exact step counts/positions at several BPM/rates/block sizes,
long virtual-run drift bounds, phase-continuous tempo changes and bounded wrap.
Slice acceptance: correct region bounds/reverse, no out-of-range reads, click ramps,
chunk invariance, deterministic variations, no render allocations, save/patch roundtrip.
Display acceptance: real playhead/step data, paused state freezes correctly, no
repeated full sample scans, cache static UI and cap animation initially15FPS.
Measure audio/CPU with view off/on under the same workload; no unbounded stress.

## Following batch: collision instrument and richer sources

Create a small fixed ball simulation and original resonator excitation, with
bounded timestamped events and a separate instrument state. The local reference
ui/orbix_res_editor.py reads shared ball positions (up to12) and
engine/orbix_res_integration.py has independent resonator preset state. Its native
collision/resonator DSP is still unavailable ARM64 code; observing its UI does not
provide a rebuildable algorithm. Start with a smaller measured ball/voice cap and
clear off/low visual settings. Keep drawing and physics separate from audio work.

First prove collision energy→audible excitation and a matching displayed event;
then add resonator models/exciters/send routing. Pair this with dual oscillator/
original wavetable work as separate reviewable modules, without rewriting the
already-tested granular engine. Add shimmer only with a measured feedback/quality
budget. Limiter/transient protection remains a useful PocketTracker-inspired
candidate, explicitly different from the current compressor/PCM clamp.

## Delivery discipline

Preserve accepted alpha15 on hardware while alpha16 develops. Do not spend a
release only renaming parameters or adding ornamental controls. Track completed
workflows, actual missing engines and audio/visual acceptance; no page-count parity
percentage. Batch code/test/deploy cycles, preserve user state, keep only current
and verified rollback on SD, and keep native publication private until authorized.


## Owner screenshots reviewed2026-11-19

One inspection of eight supplied reference screenshots; no screen captures from
our device and no copied upstream images/assets in the repository. Image1 shows
waveform plus grouped grain/roam/motion controls;2 shows waveform regions, active
steps and grouped playback/rhythm controls.3 is settings,4 a compact per-module
FX overview,5–6 dual oscillator/response/modulation pages. These suggest grouped
source/effects/performance navigation and contextual secondary controls, rather
than an ever-longer flat page cycle.

Image7 shows a distinct Orbix generative instrument: key/scale/instrument, polygon,
ball count/gravity/rotation/bounce, octave/reverb/volume and on/off. Image8 shows
a separate resonator editor with model/polyphony/preset, structure/brightness/
damping/position and excite/mod/master groups. This supports designing collision
simulation, excitation, resonator and UI as separate bounded modules. Static images
do not verify collision/audio timing, CPU, algorithms, mapping or sound quality.
The photographed footer identifies BETA1.0; it does not establish a release date
or identify the supplied video's exact version. Retain the local Beta2 audit as a
separate reference, and do not infer version chronology from the photos.

Alpha16 implements generated slices/clock/step telemetry; alpha17/18 implement
wall-collision synthesis, grouped studio views, smoother pacing and color profiles.
Manual step editing and richer resonator families remain open. Preserve the useful “instrument scene +
focused control groups + FX overview” interaction pattern in future UI design,
with original GrainMM visuals legible at640×480. Do not trace/copy upstream assets.


The active post-alpha18 delivery order is in
[NEXT_INSTRUMENT_REPORT.md](NEXT_INSTRUMENT_REPORT.md). Earlier “next” sections
above preserve the original video-derived plan; generated slicing and the initial
collision instrument have since been implemented and owner-tested.
