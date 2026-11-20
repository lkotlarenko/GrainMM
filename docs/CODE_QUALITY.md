# Code quality policy

Keep each change small enough to review and test as a working behavior. New
GrainMM code uses the scoped GPLv3 license; never copy reference implementation
code or mix its assets into new modules.

- `src/dsp`: deterministic, bounded audio algorithms, no platform/files/UI calls.
- `src/audio`: sample ownership and file codecs; decoding/writing occurs before
  audio starts or in an offline tool, never in the callback.
- `src/model`: bounded versioned preset state; filesystem work stays on the control
  thread, outside audio, and preserves failed transactions for recovery.
- `src/platform`: SDL resources, format negotiation, callback scheduling and
  input/display adapters. Close audio before destroying referenced sample data.
- `src/app`: CLI/lifecycle and composition; parameter navigation is a separate
  SDL-independent editor. Do not accumulate DSP or parsers in main.cpp.
- `include/grainmm`: documented ownership, units, limits and public contracts;
  use concrete types, RAII and const data. Avoid layers without a current use.
- `tests` / `benchmarks`: synthetic inputs only; check numerical behavior,
  corruption/limits/concurrency and real regressions. Keep host and device
  measurements separate. A timing hint is not an XRUN counter.

Compile with the project warning set and `GRAINMM_WARNINGS_AS_ERRORS=ON` (default).
Format original C++ with .clang-format; do not reformat reference code. Run the
relevant CTest profiles, runtime/tooling tests, static analyzer and source/package
allowlists. Record test limitations honestly. Do not disable checks to conceal
failures. A local exception must have a measured/documented reason.

Real-time boundaries: no heap allocation, file access, logging, parsing, drawing
or potentially blocking app locks. UI-to-audio messages are complete value
snapshots sent through a fixed single-producer/single-consumer queue. Bound the
number consumed per callback and retain pending UI changes if the queue fills.
Keep safety mute independent of parameter delivery. No sample buffer mutation or
replacement while an engine references it.

Optimize only a measured path; preserve sound and rerun numeric/block-boundary
regressions. Publish the workload, compiler/options and before/after data. No
NEON-only implementation or claims of MM+ performance from host measurements.

Before committing: inspect the actual diff, explicitly review each new source
allowlist path, run scripts/maintenance/verify_reference.py, scripts/maintenance/check_source.py and git diff --check.
Keep progress, architecture decisions and current test evidence up to date.
No push, release, system modification or overwrite of user data is implied.
