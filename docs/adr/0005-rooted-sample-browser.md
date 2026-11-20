# ADR0005 — Rooted sample browser and safe renderer replacement

Date:2026-11-18. Status: accepted; host/sanitizer/native regression and owner foreground checks passed.

SampleLibrary owns an explicitly selected root directory descriptor and a bounded
navigation stack. It lists regular WAV files and real directories, excluding hidden
entries, symlinks, FIFOs and other special files. Max128visible entries,4096scanned
entries,8path components,128bytes/component and512bytes/relative path. Direct
sample restoration can open a known file without listing the directory. Never
navigate above the root. Relative directory opens use live fd anchors and
O_DIRECTORY/O_NOFOLLOW; loads retain the same anchors until decoding finishes.
Root ancestors belong to the explicitly chosen trusted root; its final symlink
is rejected, including trailing-slash spelling. UI labels are sanitized/truncated
for the original glyph renderer; actual filenames remain byte-preserving.

Y opens/closes the browser after pausing audio. Up/down select, A opens a folder
or decodes a WAV, B goes up/closes at root, X selects the existing safe tone.
MENU always exits. File loading and listing stay on the control thread with an
explicit loading frame; there is no pretend cancel control during synchronous
WAV decoding. A failed load preserves the old sample/controls and shows the error.
Successful replacement and browser cancellation resume the device silently.

SdlAudio prepares the new Instrument/Tone before replacing the old renderer.
Allocation failure leaves the old renderer paused and intact. SDL1.2 PauseAudio
only sets a flag; hold SDL_LockAudio on the control thread for the ownership
handoff and queue drain. The callback never calls a lock or allocates. Free the
old Sample only after replacement returns. The audio device itself stays open,
avoiding repeated device initialization and its logs/configuration side effects.
Source-level evidence: [SDL1.2 audio loop and pause/lock functions](https://github.com/libsdl-org/SDL-1.2/blob/main/src/audio/SDL_audio.c).

Preset v3 combines numeric controls with an explicit source mode (legacy fallback,
tone, or rooted WAV path). Read v3→v2→v1 only when newer files are absent; never mask
corruption. Save only v3 with an exclusive parameters.v3.pending transaction,
preserving old files for rollback. Strings never cross the real-time control queue.
Resume missing/invalid WAVs with a visible/logged error and silent tone access;
keep the saved filename intact until the user explicitly chooses another source.
Explicit CLI sample errors still fail. No full GrainBrick JSON compatibility implied.

The launcher enables sample-root/resume behavior via app-scoped environment
variables, preserving old argv for verified rollback builds. Older executables
ignore the new variables. Bootstrap migration is app-only, backed up and reviewed.
No changes to Onion, services, volume, CPU or libraries.

Host tests cover catalog ordering/wrap/folders, repeated scans, path/type/symlink
rejection, depth/entry limits, descriptor cleanup on success/failure, source-mode
roundtrip, v1/v2 preservation, actual dummy-backend browser loading/restoration,
missing-file behavior,8consecutive renderer replacements, immediate old-sample
release, allocation-failure recovery and return to tone. Sanitizers remain enabled.

The instrument page shows six labels/values with selection highlight, while
callback telemetry remains in the bounded session log. Cached frame keys include
all values, current page/selection, source, play state and errors. Host-only
`display_test` verifies transitions and can export original synthetic UI fixtures
for documentation; it is not a target framebuffer or listening test.
