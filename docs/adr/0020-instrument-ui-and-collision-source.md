# ADR0020 — Instrument UI, grouped modules and collision source

Date:2026-11-19. Alpha17; owner explicitly requested the UI rework followed by a
real balls instrument and continued profiling of all-effects timing spikes.

## Interaction and rendering

Use an original charcoal/orange instrument panel with cyan signal feedback,
consistent chrome/footer, prominent source scene and six compact parameter cells.
X opens SOURCE/FX/TEMPO modules; L/R chooses groups in the navigator and pages in
the instrument. Up/down selects, A opens, B/X cancels. A still plays in the main
instrument; MENU saves/exits everywhere. Existing help/reset/browser/patch actions
retain their modal meanings. A navigator confirmation must never toggle playback.
Display order is independent of saved-field order; MIX remains bottom-right.

Keep the original glyphs/native640×480 rectangle rotation. A single extra same-format
surface caches static scene/labels/controls. Animated frames blit that backdrop and
paint meters, actual first-active-grain position, real slice step/region or six ball
poses; no repeated waveform scans, fonts, assets, FFT, GPU or framebuffer rotation
copy. Dynamic-only frames cap15FPS; immediate control/view updates retain25FPS.
Stereo meters show decaying latest-block peaks, not calibrated true-peak/loudness.
Parameter curves are illustrative control shapes, not measured FFT responses.

All dialogs share the new visual system. Text bounds are checked against640×480;
cache tests require animated poses to reuse a single layout. A single host contact
sheet was inspected; it revealed a footer overlap, fixed by separating credits.
Larger labels/step-aware numeric precision improve readability. No copied upstream
or commercial UI assets; influence is interaction hierarchy and instrument clarity.

## Collision instrument

CollisionSynth owns six fixed ball slots in a hexagon. At120Hz, audio-frame-driven
physics integrates gravity, rotating gravity direction and wall restitution, then
projects wall penetrations and reflects inward. Wall impacts excite original modal
oscillators at the exact physics frame. No UI/wall-clock scheduling, allocation,
I/O, locks or drawing. This version has wall impacts, not ball-to-ball collisions.

Eight fixed voices, each two damped modes, use prepared stable coefficients.
Impacts pick the quietest voice; tuning/brightness/decay latch at the next impact.
Ball count1–6, gravity, rotation, bounce and level are separate from note/scale/
octave/decay/brightness/excitation. The instrument mixes before the existing shared
rack. Disabled mode is an immediate idle fast path after mute decay. No-WAV quiet
fallback tone is suppressed while the collision source is enabled. Other sources
can still layer; no independent second FX rack/send or Rings/Orbix parity claim.

One packed peak/grain/hit snapshot plus six independent lock-free pose atomics
cross to UI; no mutable DSP pointers. Poses may span adjacent callbacks; this is
visual telemetry, never feedback into synthesis. DSP+physics stay independent of UI.

## Persistence and gates

Schema12 appends12fields after prior96. Preserve absent-only1–11 migration and all
old files; new12patches require alpha17+. Browser, named patches and sample lifetime
retain existing directory/type/size/atomic-save protections. Full reset pauses and
disables the new source; rack reset preserves it, modulation clears rotation.

Tests cover navigator coverage/cancel/confirm, actual SDL scene flow, legacy11 state,
collision signal/physics chunk invariance and allocation guards, paused silence,
render cache reuse and text bounds. Host/ARM/native/physical evidence is separate.
Measure the added surface/animation and all-on collision DSP on target; prior
all-effects spikes remain open until evidence resolves them. No CPU/service edits.
