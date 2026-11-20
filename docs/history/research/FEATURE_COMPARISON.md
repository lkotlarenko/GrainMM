# GrainBrick comparison and faster delivery plan

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


Reference audit: 2026-11-18. Current implementation: alpha23 candidate; physical
checkpoint: alpha22. Historical audit findings remain dated below.
Reference: preserved GrainBrick Beta2 local payload, verified baseline hashes;
not a claim of access to missing native DSP source. Original GrainBrick by ETRNLIAM;
GrainMM native implementation and Miyoo adaptation by elly (@lkotlarenko).

## What “full feature” still needs

The reference README and actual engine schemas were inspected. Matching a page
name or adding more scalar controls does not establish matching functionality.
The previous76scalar inventory was incomplete: oscillator, rhythm and ORBIX have
separate schemas. Do not calculate a parity percentage from that count.

| Capability | GrainBrick reference evidence | GrainMM now / remaining work |
| --- | --- | --- |
| Granular playback | Core grains, roaming, motion and extended harmonic controls in engine/control.py | Fixed16voices, reverse/randomness/scan/trim/window/stereo/harmony; alpha20 adds wave/drift/blend position/pitch/spread and scan hold; missing independent layers and quality resampling |
| Oscillator | engine/oscillator_fx.py: SHAPE/SYNTH/HYBRID, two wavetable sources and catalog | One stereo basic-wave oscillator, detune/vibrato; missing dual engines, wavetable morph/catalog and richer modulation |
| Rhythm | engine/rhythm/model.py:4–64regions,1–32steps, variation masks/reverse/direction/probability; sequencer/slicer modules | RHYTHM remains tremolo/autopan; separate SLICE now has an audio-clock16step generated sequencer,2–16regions, order/reverse/gate/variations. Manual steps/boundaries and longer patterns remain |
| Resonator | engine/orbix_resonator.py:42native controls,6models, excitation/input/trigger and modulation routing | Processing resonator plus new original BALLS/BOWL wall-impact instrument with six balls/eight two-mode voices and note/scale/timbre. Missing model families, richer exciters, ball-to-ball collisions and independent routing; no ORBIX/Rings parity claim |
| Spatial/effects | EQ, compression, delay/reverb, tape and shimmer controls; routing options | Most basic rack functions work; alpha12 adds crush/filter/chorus. Missing shimmer and flexible routing; reverb/comp do not match upstream algorithms |
| Tempo | tempo_clock,detect,stretch,shared modules; link/time-stretch/metronome | Audio-frame clock and shared BPM for slicing/LFO; LFO still has independent phase. No detector, metronome, external sync or independent time stretch |
| Media/projects | Projects, recordings, sample metadata and separate resonator preset management | WAV folders,16named native patches and waveform trim; no original project import, file-based project exchange or recording/export |
| Live/MIDI | Live capture/USB record and MIDI learn/clock/transport/BLE modules | Not implemented; establish actual hardware/input/backend availability before committing to these features |
| Graphics | Multiple animated audio visualizers and original1024×768 interface | Original640×480 studio UI, grouped modules, cached waveform/scenes, meters/grain/slice feedback and real collision poses at up to20FPS. More scene families/quality presets remain |

Grain2/3 labels in the original are extended control pages, not proof of three
independent concurrently rendered sample layers. Do not infer engine topology
from names. The supplied main native DSP binaries are incompatible ARM64, while
GrainMM is an independently written ARMv7 application.

## Code findings and changes made now

- The audio graph already has explicit sample lifetime, preallocated processing,
  bounded controls, smoothing and tested platform adapters. Preserve that foundation.
- Adding fields used to require repeated editor switches plus an independent codec
  list/parser. That duplication was the main avoidable feature-development tax.
  Alpha12 introduces a typed append-only parameter schema: one descriptor controls
  labels, steps, accessors, enum choices and serialization order. No pointer offsets
  or type punning. Editor373→72lines; codec179→74lines, with shared225line metadata.
- Historical schema prefixes6/12/12/24/30/42/48/66/72/90/96 are explicit and tested.
  A new feature appends entries once; it must never reorder old saved values.
- application.cpp still owns several view/input branches (~560lines). Keep DSP and
  storage out of it. Extract a controller when adding a sequencer/page picker rather
  than continually growing its event switch.
- Audio scheduling outliers remain under real Onion runtime; late-callback hints
  are not XRUN counts. Do not use more voices or an expensive pitch shifter before
  workload measurements. No speculative SIMD/fixed-point rewrite is justified.

## Delivery sequence

| Batch | Concrete result | Acceptance gate |
| --- | --- | --- |
| Alpha12–14, implemented now | Bit depth/sample-hold crusher, multimode resonant filter, stereo chorus; confirmed dry reset; shared parameter schema | Reference/bounds/impulse/no-allocation/block tests, old saves/patches, native timings, one owner listening batch |
| Next: slice/tempo workflow | Real region sequencer, shared audio-clock timing, direction/reverse/probability and variation selection | Beat/slice boundary references; sound continues during navigation; save/reopen; native CPU |
| Next: source expansion | Second oscillator, original wavetable morphing and independent grain layer using shared sample ownership | Shared final FX rack, fixed total voice budget/quality profiles, no duplicate sample buffers; measured single/dual load |
| Next: spatial/modulation | Pitch-shifted feedback shimmer, additional resonator/exciter families and modulation targets | Fixed buffers, bounded feedback, impulse/continuity tests and real deadline/listening comparison |
| Product/release work | Patch exchange/export, repeated load/exit/power/headphone checks and release automation | Preserve user files, verified package/source, hosted CI and explicit source/binary publication approval |

Live input, BLE and original project import are capability/specification projects,
not cheap effect toggles. USB/network/microphone support is not assumed. Recording
could begin with bounded offline export before any device capture promise.

## Faster working method

Implement coherent batches on the host, run cheap focused checks while coding,
then one full host/sanitizer gate and one cross-build when the batch settles.
Deploy one candidate, run bounded native fixtures/benchmarks, request one combined
physical listening/control check and retain only candidate+verified rollback.
Use the typed schema instead of three hand-maintained lists. Keep numerical
references and old-format fixtures independent of the new implementation.

Start each next batch from the accepted artifact and this feature matrix. Record
actual gaps, not repeated source hunts or a new port architecture. Screenshots only
for an essential new-layout/rendering problem; logs and owner input cover ordinary
parameter pages. Code and artifacts remain private until the owner authorizes
native publication; major development progress is not a stable release claim.

Physical alpha12 feedback replaced the simultaneous reset chord with a sequential
SELECT → START → X confirmation in alpha13. This illustrates why native simulated
input alone cannot establish physical control usability.

Alpha14 adds native parameter cards, range/source/transport feedback, Apps branding
and held START+SELECT access requested by the owner. See ADR0016 for input semantics.

Alpha14 introduced direct held START+SELECT access; alpha15 adds explicit reset scopes
and accelerated D-pad repeat after owner usability feedback. Native simulation does
not replace physical control/listening acceptance. See RESUME.md for current artifact.

## Video follow-up and usability baseline

See [VIDEO_FEATURE_ROADMAP.md](VIDEO_FEATURE_ROADMAP.md) for the supplied transcript
comparison, concrete slice/clock/visible-feedback batch and collision-instrument plan.
Alpha16 separates display/persistence order and standardizes MIX/output positions.
The owner wants visible musical interaction alongside richer engines, not more
effect labels alone. Alpha18 is accepted; current device evidence is in RESUME.md.


## Alpha16 implementation checkpoint

FrameClock, generated slice patterns and a two-voice source are implemented and
pass host/sanitizer/ARM numerical and native app cases. New96field settings11
preserve older files. The SLICE strip displays real step/region telemetry. Manual
pattern editing/manual cut points remain open; alpha17 adds collision excitation;
no claim of complete upstream rhythm/Orbix parity. Owner listening is recorded in
RESUME.md. Retain this source/FX separation when adding future instrument scenes.


Alpha17 implements the owner-requested UI rework then the collision source. Grouped
navigation and all dialogs share an original instrument design. Wall impacts, not
ornamental animation, excite bounded modal voices. Screenshot reference informed
hierarchy; no upstream/commercial assets imported. No independent second FX bus,
ball-to-ball contact or full resonator-model parity is implied. See ADR0020/baseline.


Alpha18 fixes the owner-reported pacing, wall settling and group order, and adds
six color profiles. The owner reports substantially improved behavior. See
[NEXT_INSTRUMENT_REPORT.md](NEXT_INSTRUMENT_REPORT.md) for the prioritized ambient,
snapshot, synthesis, manual-pattern and export plan after reviewing the actual code.


Alpha20 implements the first DRONE workflow and settings13/114fields, retaining
base values/fixed grain count. Native tests pass; listening remains in RESUME.md.
Second tonal/wavetable voices, manual pattern/cut editing and export are next.

Alpha22 extends the original standalone drone with brighter register/eight moods,
four bounded melodic event voices and seed/rate/glide/width/progression steering.
It remains a compact pad/event instrument, not dual wavetable synthesis or a full
arranger. Actual voice/event activity now drives the source display.
