# 1.0.0-alpha.1 validation

Session: 2026-11-20. Starting commit dc2916a, native-device-proof branch.
Local work in progress; this document is updated with final command results below.
Alpha25 remains the accepted physical build. This record covers the feature-development session before publication approval.
The later [publication review](PUBLIC_RELEASE_REVIEW.md) records release authorization
and final checks.

## Implemented scope

Oscillator B, generated wavetable morph/scan on A, mono sub oscillator, slice swing,
variable phrase length, traversal/rotation and accents. Three append-only pages,
schema18/150scalar fields plus Pattern1. Patches/snapshots/export share the graph.
SOURCE card banks maintain drawing bounds. See [ADR0030](../adr/0030-synthesis-groove-and-major-milestone.md).

## Checks and corrections

- Initial440reference inputs hash-verified; no upstream payload/assets reused.
- Initial focused run found a test still expecting OUTPUT to be the final page.
  Updated schema traversal expectations; no runtime bypass added.
- Initial full run found old SOURCE navigation scripts landing on newly added
  cards and a test using the old sample-token offset. Corrected actual input-only
  navigation with two extra UP events and updated schema18 expectations.
- A new fade assertion tested only208ms at96kHz, shorter than the smoothed gain's
  convergence threshold. Extended the silence observation to one second.
- Sandbox sanitizer run hit LeakSanitizer's ptrace limitation. Repeat outside the
  sandbox with leak checks enabled; do not count the sandbox failures as passes.
- Corrected four WAV integration cases pass, including drone/scenes/evolution and
  reset. New feature/parameter tests pass after correction.

## Final verification

| Check | Result / scope |
| --- | --- |
| Host CTest |36/36pass,210.04seconds; includes28WAV integration cases in normal and openat-compat runs |
| ASan/UBSan/LSan |35/35pass,158.81seconds outside sandbox/ptrace, leak checks enabled |
| Runtime integration |5/5pass: formats/buffers, startup errors, relaunch and signal shutdown |
| Tooling / hardware tools / source archive tooling |8/8,8/8,2/2pass respectively |
| ARM |All targets cross-compiled with pinned GCC8.3 SDK; six runtime/benchmark ELF checks pass. Static evidence only |
| Static analysis |GCC -fanalyzer/-fsyntax-only on changed DSP, schema and codec; no diagnostics |
| New feature allocation guard |Passes host debug, optimized -O2 and sanitizer; ARM compilation passes |
| Source rebuild |310allowlisted files archived; all151build steps compile; nine focused integration cases pass in3.39seconds |
| Installation packaging |Two independently generated253,046-byteZIPs compare byte-identical; member/type/hash allowlist validates |
| Docs/source/preservation |131Markdown local-link/heading checks;310approved paths;440reference hashes unchanged; diff whitespace passes |
| Public preview |17documentation/image/license files, standalone links valid, SHA256 manifest; local only |
| Physical device |Read-only probe cannot reach device; no deployment or owner-data mutation |

The final allocation-counter test uses observable reads and opaque noinline
allocation/free helpers. GCC16 optimization otherwise cached the counter or issued
a mismatched-allocation false positive; placing the attribute directly on overloads
hit a GCC8 SDK warning. Helpers resolve both without suppressing warnings or leak
checks. Focused checks pass after this test-only portability correction; production
DSP is unchanged. Final source manifests identify exact compiled inputs.

Host optimized synthesis sweep: 11cases, worst512-frame block 792.272us,
software clips0. The standalone dual/table/sub + rack case has p95=257.829us,
p99=327.551us, max343.381us. These are host observations with synthetic samples,
not MM+ CPU/headroom or audible quality evidence. Dense workloads may drop grains
at the bounded16-voice limit; this is not a hardware XRUN count.

ARM app SHA256: `84c59f13436f82904a1e32d8160533bdbefe141991aa3d0fc06cfcd27165e881`.
Private command logs: `build/logs/v1`. Exact source/cleanup/public staging manifests
and artifact hashes: `.local/v1`. Final install/source rehearsal outputs live in
`dist`; they are not uploaded. No Git commit, push, tag or release was made.
Only explicit documentation relocation paths were staged for the source gate.

## Remaining physical acceptance

New synthesis sound/tuning, groove feel, paged SOURCE controls, save/reopen and
new settings export/resample need actual-device evidence. Longer stress/headphones,
power/sleep and VNC-off resources remain open. Cross-compilation/dummy audio cannot
mark these passed. Deployment, when available, uses ongoing approved app-only
scope, never a running executable or Onion internals.
