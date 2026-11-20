# ADR0019 — Audio-frame clock and bounded slice source

Date:2026-11-19. Alpha16 development.

## Clock and model

FrameClock uses a Q48 fractional beat and unsigned beat count. Increment is prepared
outside audio from BPM/rate. Tempo updates retain phase; transport start resets to
beat0. Consumer copies are block anchors; the master advances once per output frame.
No wall/UI clock schedules notes. Quarter/eighth/16th/32nd boundaries survive render
chunk changes; wrap is unsigned. Standalone supported clock rates8–96kHz/BPM20–300;
current shared RHYTHM/BPM UI retains its existing30–240 range.

Six append-only scalar fields configure a separate16step slice source: order,
region count, division, gate, variation and reverse chance. Prepare a fixed pattern
on the control thread, clamp all values, and send it in the existing bounded queue.
Modes are off/forward/reverse/ping-pong/shuffle. Variations are straight, sparse,
alternating slice reversal and deterministic75%chance. This is a useful generated
sequence, not an arbitrary step editor, manual cut editor or independent time stretch.
Pattern state/data can later grow independently; do not label presets as manual editing.

## Ownership, transitions and budget

SlicePlayer references the same immutable Sample as Granular. Two fixed slice
voices crossfade boundaries with4ms ramps; region reads clamp at region ends,
reverse reads stay inside bounds, and gate is limited by step/source duration.
It uses GRAIN pitch/gain and SAMPLE start/end. Grain density/motion/harmony/window
and stereo width remain grain-source controls. The UI identifies bypassed controls.
OSC and the existing single effects rack remain shared.

A20ms source crossfade switches grains/slices. Steady state runs either16grain
voices or two slice voices. The transition is explicitly bounded at18voices and
one extra512float scratch array, with no sample duplication or allocation. Measure
that brief transition on target; do not represent the steady16limit as a total cap
throughout the crossfade. Existing default-mode PCM must remain unchanged.

The clock shares RHYTHM/BPM with the LFO. The old LFO retains its own phase and
smoothing: this does not yet provide phase-locked effects, metronome, external MIDI
clock or a general synchronization system. Rack/modulation resets preserve shared
BPM; modulation also clears slice variation/reverse chance. Factory reset pauses
and restores defaults. Source selection and stored patches remain separate.

## Persistence and visual feedback

Schema11 appends six fields after the original90; old1–10 load only on absence
and remain intact. New11patches need alpha16+. Strict validation and atomic saves
remain unchanged. No old scalar order is reused or reordered for presentation.

The callback publishes step/region/valid bits through one lock-free unsigned atomic,
plus a slice-trigger counter. UI reads no mutable voice state. SLICE displays actual
step/region and a16step strip; cached drawing capped at15FPS. Other parameter pages
stay cached at their previous limit. This is not a bouncing-ball visualizer or a
waveform playhead yet; no screenshot loop or SD capture dataset is added.

## Verification

Independent clock boundary/phase/drift references, generated pattern orders,
constant-region numerical amplitudes, reverse/trim extremes, pause silence,
whole/split render equality (including integrated mode transitions), render-allocation
checks, schema10 preservation and actual SDL edit/play/save/reopen probes.
Host/cross-build evidence is separate from native dummy and owner listening.
