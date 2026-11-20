# ADR 0030 — synthesis, groove and a major alpha milestone

Date: 2026-11-20. Status: accepted implementation; target acceptance separate.

## Decision

Use 1.0.0-alpha.1 for the owner's major feature milestone while keeping
stable_ready and source_publication_authorized false. Seven musical additions
extend the existing native graph: oscillator B; generated wavetable morph/scan;
sub oscillator; slice swing; phrase length; traversal/rotation; repeating accents.

Reuse Oscillator for independently tuned B. A alone accepts TIMBRE settings.
Generate four normalized spectra with eight harmonic bands and 257 samples per
band in a shared immutable 32,896-byte table, initialized during construction.
Interpolate shape/sample/band; reduce supported harmonics with pitch. No assets,
callback allocation or trigonometric table generation occur in rendering.
Smooth level, pitch, table motion and sub-octave transitions. Sub remains audible
independently of A's level and suppresses the no-source fallback.

Derive slice ticks from Q48 audio phase and beat count. Delay odd subdivisions by
SWING times one step, keeping even boundaries fixed. At one subdivision per beat,
beat parity still swings pairs. Gates follow the long/short intervals. Traversal
maps elapsed events to an active 1–16-step prefix, then applies rotation; bounce
avoids duplicated ends. Accents attenuate other steps rather than boosting peaks.
Settings edits apply at an audio update; changing division/swing can rephase a step.

Append three six-control pages after the immutable 132-field prefix: OSC B,
TIMBRE, GROOVE. Schema18 stores 150 scalars and the unchanged Pattern1 payload.
Read legacy1–17 when newer files are absent; preserve their bytes. Named patches
and snapshots inherit these values; offline export uses the complete graph.
Modulation reset clears scan/groove/detune while preserving base synthesis.

Keep a maximum12 module cards per bank with explicit BANK indication, two banks
for14 SOURCE destinations. File actions require explicit selection. Drawing bounds
cover all selections/palettes; UI IDs are internal and not preset storage IDs.

## Consequences and evidence

Tables cost shared32KiB plus additional oscillator state/CPU. High pitches lose
upper partials. TIMBRE is not an importer, polyphonic keyboard or upstream DSP.
GROOVE affects slices, not drone/collision clocks or rhythm effects. No flexible
routing or copied sample buffers are introduced. Target headroom must be measured.
Tests cover no-allocation rendering, tuning, distinct spectra/scan, mono/stereo
chunk equivalence, swing timestamps, traversal/accent behavior, migration, export
PCM, reset and UI bounds. [Validation](../development/V1_VALIDATION.md) records
actual runs and limitations. Existing device acceptance does not validate these
new features. Historical docs move to archive; current user guidance is rewritten.
