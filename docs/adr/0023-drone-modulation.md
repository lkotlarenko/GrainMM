# ADR0023 — Slow grain modulation and DRONE workspace

2026-11-19. The next-instrument report prioritizes a playable ambient workflow
using the tested granular engine, rather than another isolated effect or rewrite.

DRONE appends six scalar controls: mode(off/wave/drift/blend), rate0.01–0.50Hz,
position depth0–1, pitch depth0–1200cents, spread depth0–1 and scan hold. Settings13
append them after the unchanged108field prefix. Absent-only migration retains
versions1–12, preserves corrupt/pending files and never overwrites old saves.
Source selection/patch wrapper are unchanged; new patches require alpha20+.

`DroneMotion` is audio-owned, fixed storage and separately seeded from grain
randomness. Phase advances on audio frames; drift uses smooth interpolation between
seeded endpoints. Wave/drift/blend targets update every64frames and slew per sample
over a bounded20ms full-range interval. Control edits retain phase/seed. Pausing
freezes phase; hold also freezes the existing continuous scan while mode is on.
Hold does not stop sound, freeze live input or eliminate random per-grain spread.

Base grain values never change. At spawn, position/spread offsets are bounded in
the normalized sample region; pitch is converted once per spawned voice and
limited to existing base pitch bounds before harmony. Existing grains retain
latched pitch/window. No sample copies, new voices, file I/O, locks or allocation
in render. Disabled/settled modulation has a fast path and must retain old output.
No per-sample graph preparation or UI-driven timing. Slice/oscillator/collision
sources are unaffected; DRONE requires WAV granular mode and reports active slices.
Rack reset preserves it; modulation/full resets clear it. Snapshots/patches include it.

DRONE is stored page18; STYLE moves to UI-only page19. SOURCE has nine cards and
all groups still form one contiguous shoulder cycle. Existing lifecycle probes
retain historical18page semantics while traversing the actual20destination route.
Collision scene detection now names only BALLS/BOWL, rather than treating every
later page as a ball scene. DRONE uses the cached waveform/base-position marker and
existing active-grain telemetry; no extra cross-thread state or per-frame wave scan.

Validation includes a periodic reference, drift determinism/bounds, pause/hold,
zero-offset settling, whole/split PCM/reset equality, allocation guards, old-format
preservation, all-page navigation/render bounds and a real SDL edit/play/hold/help/
save/reopen flow. Native default/on workloads must be compared, then owner listening
recorded separately. A cap/host result is not proof of target audio reliability.
