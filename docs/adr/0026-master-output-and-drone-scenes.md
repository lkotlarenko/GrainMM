# ADR0026 — Master output and original drone starting scenes

2026-11-19. Owner confirms alpha22 improves sound/UI but requests more usable FX
volume. A host PCM audit of one fixed pad showed varied level changes: chorus
about−3.1dB, delay−3.7dB, resonator−6dB, tape+0.5dB, reverb+0.2dB, all-band EQ+8.2dB.
These are signal/settings-dependent, not universal corrections. Avoid cumulative
fixed boosts in every effect; add one explicit post-rack output stage instead.

OUTPUT offers smoothed−24to+18dB gain, stereo-linked sample-peak limiting, smoothed
ceiling−12to0dBFS,20–1000ms release, attenuating stereo balance and smooth mono.
No lookahead/true-peak guarantee. Instant attack bounds actual sample peaks against
the current slewed ceiling; heavy limiting can sound different. Recovery snaps
numerical residue to unity. The default stage is bit-exact neutral/off. Existing
source/FX/tone conversion order is preserved at neutral. Diagnostic tone is also
subject to master output; device mixer is never changed. Gain design/exponentials
stay on control thread; fixed state and bounded sample math in the callback.

Output telemetry extends the value-only relaxed atomic snapshot with packed gain
reduction. The UI shows actual limiting, not a fake threshold indicator. Output
lives after rack and before existing transport fade/PCM. Rack/modulation reset keeps
output for comparable dry/wet tests; all-controls reset restores0dB and pauses.
Schema16 appends6fields after schema15's unchanged126prefix. Missing-only migration
preserves earlier files and defaults output to0dB/off, without loudness jumps.

B on DRONE/GENERATE opens an eight-scene chooser; other pages retain waveform B.
Original static scene data contains no assets/samples/copied presets. Browsing only
changes highlight/seed. A explicitly applies only drone generation, preserving gain,
FX/output, other sources, immutable WAV ownership, transport and snapshot eligibility.
B/X cancels; MENU exits without applying a pending choice. Edits use the existing
bounded prepared queue, without replacing engines or resetting phases. No separate
scene file/schema: saved scalar parameters fully describe the chosen sound.

Chooser/reset/snapshot arbitration excludes the modal; cached scene cards use the
existing software renderer and bounds checks. No screenshots needed. Tests cover
neutral output/gain/limiting/recovery/stereo/mono/block determinism/allocation,
scene audio/bounds/unchanged neighbors, old15migration, cached geometry and actual
SDL browse/cancel/apply/output-edit/play/save/reopen. Native listening is separate.
