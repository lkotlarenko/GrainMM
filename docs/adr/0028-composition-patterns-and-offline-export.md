# ADR 0028 — Editable patterns and bounded full-instrument export

2026-11-19. Implemented for alpha25; physical acceptance is tracked in the handoff.

The instrument needs deliberate musical sequences and a way to keep the resulting
sound. These are the first two workflow priorities in the instrument research report.

## Pattern ownership and compatibility

`SlicePattern` is a fixed value containing a manual/generated flag, 16 steps and
16 positive region widths. Steps contain a region, rest flag, reverse flag and
probability. Widths normalize into ordered cut boundaries for the active region
count. Moving a cut trades width between neighboring regions, preserving the
other boundaries. First/last cuts stay at the sample trim boundaries. Changing
REGIONS reinterprets the first N widths; it does not erase the saved pattern.

The instrument snapshot owns this value, including autosave, named patches and
temporary snapshots. Generated mode keeps manual edits available for returning
later; generated MODE/VARIATION do not overwrite them. Manual mode replaces the
generated steps and cuts; global tempo, division, gate and reverse still apply.

Preparation bounds data and computes cuts outside audio. The existing bounded
queue carries complete trivially copyable snapshots. Rendering adds no heap
allocation, parsing or file access. Existing voices finish safely after edits;
new triggers use the new snapshot. A pattern is 16 steps, not a song arranger.

Schema 17 retains all 132 scalar fields in their existing order, then source
selection and a separately versioned `Pattern 1` payload. Schemas 1–16 migrate
only when the current file is absent; corrupt current files remain errors.
Earlier settings are preserved for rollback. Old app versions cannot read new
named patches. The bounded preset/patch limits are 4 KiB / 8 KiB respectively.

## Export ownership and storage

`InstrumentExport` owns a new portable instrument graph and exclusive WAV writer.
It borrows the immutable sample while playback is paused. It starts sources, clock,
random seeds and effect histories anew from current settings; this is deterministic
offline rendering, not a recording of live playback or a continuation of its tails.

The UI advances at most eight 256-frame blocks per iteration, then handles events.
The export screen provides progress and cancellation. Playback stays paused on
return. No rendering or SD writing runs in the audio callback, and there is no
background writer thread or extra sample copy.

Outputs are bounded to 60 seconds, PCM16 stereo at 48 kHz in `state/exports`.
Names are exclusively created, never overwritten; the directory is descriptor
anchored and symlinks are rejected. Check space plus a 1 MiB reserve before
creation. Successful output is flushed; cancellation checks the actual open
writer's device/inode before deleting only its incomplete file. Process death or
SD errors can leave an incomplete file, which is preserved rather than guessed at
on the next launch. Complete outputs are never removed by cancellation.

The developer `grainmm-export` tool uses saved settings and opens no audio/display
device. The older `grainmm-render` granular diagnostic keeps its original contract.

## Controls

B on SLICE opens pattern editing; A retains play/pause, L/R selects the step,
D-pad selects/edits a field, X toggles a rest, Y switches manual/generated and B
returns. A manual edit selects manual mode and enables forward slicing if MODE
was OFF. B on OUTPUT opens export; arrows choose duration, A explicitly starts,
B cancels or returns and MENU always exits. Reset/snapshot gestures are suppressed
inside these screens; regular main-view behavior remains unchanged.
