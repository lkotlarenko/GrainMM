# Testing GrainMM

Run checks against the current implementation, then record the source/artifact
hash, test conditions and evidence type. The exact active hardware checkpoint is
in [RESUME.md](../development/RESUME.md).

## Host checks

```sh
sh scripts/build.sh host-debug
sh scripts/build.sh host-asan-ubsan
python3 tests/runtime_test.py
python3 tests/tooling_test.py
python3 tests/hardware_tools_test.py
python3 tests/source_archive_test.py
python3 scripts/maintenance/check_source.py
python3 scripts/maintenance/verify_reference.py
```

LeakSanitizer requires execution outside ptrace; keep leak checks enabled. Host
SDL dummy tests do not prove audible output or physical buttons. Tests use
synthetic samples and temporary state, never original GrainBrick assets.

Coverage includes sample-clock boundaries/phase/drift, slice pattern/region/reverse/
transition/chunking/allocation checks, schema10→11 preservation, and DSP references/continuity/bounds, grain scheduling/wrapping,
WAV corruption/limits, state codecs/migration, directory anchors, patch atomicity,
replacement confirmation/name entry, waveform min/max/cache views, source ownership,
held-button guards, harmony sequence/reference/chunking/bypass/no-allocation checks,
help editing/reset, typed schema compatibility, crush/filter/chorus references,
held reset gesture/tick wrap, reset scopes/paused defaults, accelerated held arrows
and full patch/save/load/trim application navigation.

## ARM and device

```sh
GRAINMM_SDK="$PWD/.local/sdk/miyoomini-toolchain" sh scripts/build.sh miyoo-armv7-release
```

ELF/dependency validation is static evidence. Numerical suites may be run as
bounded, temporary device tests under the approved app directory. The dummy-only
`--self-test-library` needs a WAV, state directory and at least4seconds; it writes
one patch in that isolated state. Never point automated tests at the user's real
patches. Use copied fixtures, preserve original hashes and remove only known files.

Physical validation covers real launch, sound, controls, waveform/patch readability,
interrupted/corrupt save recovery, repeated load/exit, memory/CPU and longer soaks.
Use logs and read-only metrics; screenshots are exceptional. VNC changes workload.
Callback lateness is not an XRUN measurement; do not report zero hardware underruns
when the adapter cannot count them. See [device acceptance](../DEVICE_TEST.md).

[Earlier verification reports](TESTING_2026-11-17.md) are retained as history.

Alpha17 adds navigator coverage and actual SDL module open/cancel/transport guards,
collision physics/audio chunk invariance/allocation checks, schema11 preservation,
text bounds and static-layout reuse across animated frames. Native dummy rendering
measures software paths; physical feedback separately establishes upright/readable
UI and audible collision notes. No routine screenshot loop.


Alpha19 adds snapshot ownership/bounds/source-generation and modifier consumption
checks. The dummy-only --self-test-snapshot needs a WAV, isolated state and4seconds;
it tests empty recall, capture/edit/recall while playing, changed-source rejection
and recapture/recall while paused. It must not toggle playback/open help/trim as a
side effect; saved state is checked independently. No snapshot disk file is made.


Alpha20 adds periodic-reference/drift determinism/bounds, hold/pause/off settling,
PCM whole/split/reset/bypass and render-allocation guards. Old settings12 are
preserved through13migration. Dummy-only --self-test-drone requires a WAV, isolated
state and6seconds; it visits DRONE via actual grouped keys, edits all six controls,
plays, holds/resumes movement, opens help, stops and saves. Base grain position
remains unchanged. Offline benchmark --drone uses the same fixed voice budget.


Alpha21 standalone --self-test-drone uses no explicit WAV, isolated state and
6seconds. It navigates from OSC to DRONE, edits all six controls, plays/help/stops,
saves14 and reopens. The former grain diagnostic is --self-test-evolve and still
requires a WAV. DroneSynth numerical tests cover split/reset determinism, autonomous
changes, finite/power/DC/stereo bounds, pause, allocation, shared effects and reset.
