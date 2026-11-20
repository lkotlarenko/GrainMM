> Historical evidence. Current checkpoint: [RESUME.md](../../development/RESUME.md).

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


# Alpha 9 — larger instrument iteration

2026-11-18. Original resonator, tempo modulation, sample tools and contextual help.
No new dependency, upstream code/asset import, public source push or system change.

## Delivered behavior

- Four parallel tuned modes with retuning crossfades, decay, brightness,
  inharmonicity, width and mix. A musical resonator, not exact ORBIX/Rings behavior.
- Tempo/division tremolo and autopan, sine/triangle LFO and stereo phase.
- Non-destructive sample region, attack/release window blend and stereo width.
  Voices latch region/window at spawn; forward/reverse reads wrap inside it.
- SELECT help preserves audio state; A still plays/pauses outside the browser,
  B/SELECT closes help and MENU always saves/exits. Cached view transitions.
- Eleven pages and 66 controls, schema 8 preserving v1–v7. Old Hann/full-region
  behavior remains the default; newly introduced effects start bypassed.

ADR 0011 records equations, reference, bounds and real-time contracts. Render/apply
allocation checks include the new active modules. No parsing, drawing or logging
is added to the callback. Fixed-mode state/LFO storage; no extra sample copies.

## Host and build gates

Full host 18 CTest groups passed; after final help/CLI/allocation-check changes,
all affected 8 groups passed again. ASan/UBSan/LSan 17 passed outside ptrace with
leak checks enabled. Tooling 8/hardware 8/runtime 4 passed; clang static analysis
covered resonator/rhythm/granular. ARM hard-float/loader/dependency checks pass,
without new shared libraries. Package 96921 bytes reproduced byte-for-byte.

Reference-sine testing caught accumulating float LFO phase error at higher rates;
phase is now double, preserving the prepared float control rate. Modal gain is
checked against an analytical complex transfer function. Trim sentinels detect
reads outside selected regions, including reverse/high increments and tiny files;
voice-latched envelope and endpoint tests pass. A UI test initially adjusted a
retained row instead of DIVISION; corrected the test to select its row explicitly.

One manual ARM CMake reconfigure omitted GRAINMM_SDK and failed safely. Re-ran
through the documented GRAINMM_SDK + scripts/build.sh entry; do not omit that env
when CMake regenerates the target build. No checks or flags were weakened.

## Native CPU and lifecycle

The same numerical suites passed on physical ARM:

- rack_test SHA-256: 277b05de659a72886ba69d352107ff2c0a21479b7b5985f5061a23618bba11a8.
- granular_test SHA-256: 498ba97e1367ff5cdcf6579319ae124977a9b96b21975a033811644180397b80.

Seven stock-preload + SDLdummy cases passed: v7 migration/browser, standalone
rack/help/save, reopen/rack edit, WAV region/window/help, all-effects WAV graph,
missing-WAV edit recovery, restored-WAV reopen. Maxima 15218/6650/6937/14415/
14585/13785/437 us per 1024 respectively; all late 0/clips 0. Three probes actually
opened/closed help. Owner WAV/v7 hashes preserved; fixtures/unit binaries removed.
These are CPU/lifecycle tests, not audible output or physical button acceptance.

## Physical acceptance

Owner confirmed both resonator/rhythm sound changes, clean audio, readable SELECT
help, B close, MENU and save/reopen. Separately confirmed sample region/window/
width changes, clean sound and saved settings. No screenshots were requested.

Final retained audible WAV log:6667 callbacks,6827008 frames,1024/callback,
span 145280 ms including UI/source activity, worst 19195 us,late 0,4100 grains,
dropped 0,171 updates,software clips 0. Last reopen/exit:132 callbacks,worst 14191 us,
late 0,41 grains,clips 0. The earlier synth/help listening session was superseded
in the two-log rotation; its physical evidence is the owner's explicit report.
Final saved source was a WAV; start≈0.25/end≈0.75,ENV BLEND 1,width 1,
RESONATE MIX 0/TREMOLO 0. Do not describe the final bypass snapshot as an always-wet
session. Old settings versions remain available for rollback.

A read-only 3.05 s foreground interaction snapshot recorded RSS/HWM 6280 KiB,
3 threads,17 CPU ticks at previously verified USER_HZ 100:5.57% of one core.
Audio/parameter state may have changed during that short window; this is neither
steady-state stress CPU nor peak memory with a maximum-size WAV. VNC 2 FPS + SSH
were enabled. No CPU/governor/mixer/service changes.

## Offline timing

48 kHz/512 frames,93 blocks per case, VNC 2 FPS + SSH. Clock observed 1000 MHz before,
1200 MHz after; no constant-clock claim. All software clips 0.

| Workload | p95 us | Max us |
| --- | --- | --- |
| Normal grains + old full FX, tape off | 2577.167 | 2608.333 |
| Normal grains + tape | 2988.167 | 10066.166 |
| Normal grains + tape + alpha9 rack/window | 3460.166 | 5334.333 |
| Dense grains + old full FX, tape off | 3911.500 | 4128.500 |
| Dense grains + tape | 6171.334 | 12663 |
| Dense grains + tape + alpha9 rack/window | 4805.500 | 4859.333 |
| Standalone synth + all effects/rhythm | 2862.167 | 3196.667 |

Scheduling outliers remain: ordering/clocks/workloads differ, so the lower expanded
result is not evidence that adding DSP made it faster. Dense cap 70 spawned/9 skipped
was equal across rack-off/on cases; skipped grains are intentional voice limiting,
not XRUNs. Actual backend uses 1024 frames; longest retained real callback 19195 us
leaves only about 2.1 ms to its 21333 us block budget. Longer VNC-off/audio/headphone/
power tests are still needed. Backend XRUN counters remain unavailable.

## Durable checkpoint and next iteration

CURRENT / LAST_GOOD: 173ca01ec1f9, appSHA 256:
cef568d2d3926ce9ee580391138531d9839563360e3a536f14bdb29c57781fcd.
Verified alpha 8 rollback d89355fb031f retained. Alpha 7 was checked against its host
archive before exact-file pruning; only two releases remain,2720 KiB total.
Private alpha9 scripts/logs retain the deployment, tests, measurements and approval.

Next: named patch/project workflow and cached waveform/trim overview; then more
synthesis/layers only with measured headroom. Maintain short feature gates,
physical listening and preserved rollback. Public main remains documentation-only;
no source/workflow/tag/release publication without new authorization.
