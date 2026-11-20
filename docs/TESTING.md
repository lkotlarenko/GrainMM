# Testing GrainMM

Use [RESUME.md](development/RESUME.md) for the exact candidate and hardware state.
The 1.0 candidate has36host and35sanitizer CTest cases; exact results are in
[1.0 validation](development/V1_VALIDATION.md).
A host dummy-audio run cannot establish physical audio quality or button behavior.

## Commands

```sh
sh scripts/build.sh host-debug
sh scripts/build.sh host-asan-ubsan
python3 tests/runtime_test.py
python3 tests/tooling_test.py
python3 tests/hardware_tools_test.py
python3 tests/source_archive_test.py
python3 tests/release_test.py
python3 scripts/maintenance/check_source.py
python3 scripts/maintenance/check_docs.py
python3 scripts/maintenance/verify_reference.py
```

LeakSanitizer requires execution outside ptrace; retain leak checks. SDK restoration
and ARM compilation are in [BUILDING.md](BUILDING.md). Never execute host filesystem
regression tests on the Miyoo or substitute cross-compilation for physical evidence.

## Coverage

DSP checks cover signals, continuity, sample/chunk boundaries, deterministic state,
allocation guards, grain/slice scheduling, harmony, effects, collision, drone/events
and output gain/limiting. Model tests cover strict codecs, legacy settings1–17,
patch replacement/recovery, snapshot ownership and rooted sample loading. SDL tests
exercise controls, repeated input, modal apply/cancel, cached rendering, play/save/
reopen and clean shutdown. Fixtures are synthetic and isolated from owner state.

Current dummy-only source diagnostics:

| Flag | Input | Minimum duration |
| --- | --- | --- |
| `--self-test-performance` | WAV and isolated state | 9 seconds |
| `--self-test-evolve` | WAV and isolated state | 6 seconds |
| `--self-test-drone` | Built-in source and isolated state | 8 seconds |
| `--self-test-ambient` | Built-in source and isolated state | 12 seconds |

Native scripts are reviewed, bounded and streamed over SSH stdin. Keep current and
verified rollback only on SD; never overwrite a running executable. Preserve owner
files and distinguish callback lateness, software clipping and actual listening.

Historical test detail remains in [the versioned record](history/TESTING_THROUGH_ALPHA23.md).
Old flag names/durations there describe their versions, not the current CLI.


Alpha25 composition coverage includes `pattern` (cuts, rests/reverse/chance,
clock/block equivalence and reset scopes), `instrument_export` (full graph PCM
reference, repeatability, migration, names/path guards and cancellation),
`composition_screen` (dummy audio/UI transactions) and the allocation guard in
`slice`. WAV integration runs `--self-test-pattern`, checks saved step fields and
reopens the unchanged save. Host dummy success does not establish physical audio.


1.0 `performance_features` covers dual-source fallback/tuning, table spectra/scan,
sub octaves, all supported rates, no-allocation/chunk-equivalent synthesis,
swing timestamp pairs, variable phrase/rotation/order, accents and reset scopes.
Export checks include schema17 migration and new-control PCM. Display tests visit
every module selection/bank in all six palettes; numeric bounds avoid screenshots.
`instrument_benchmark --seconds 3 --synthesis` adds the new dual/table/sub workload.
Record workload/flags; host timing does not predict target deadlines.


Release verification checks the installation manifest/version, both exact checksum
files and every source-archive member/hash against the release checkout allowlist.
Adversarial tests reject source substitution even with a regenerated checksum.
The release workflow runs this verification before publication; see
[publication review](development/PUBLIC_RELEASE_REVIEW.md).
