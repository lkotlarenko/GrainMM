> Historical evidence. Current checkpoint: [RESUME.md](../../development/RESUME.md).

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


# Tape iteration — alpha8

2026-11-18. Goal: ship a useful original tape-style page with safe state migration,
then validate on the Miyoo before starting another effects batch.

## Implementation

Six real controls: drive, tone, wow, flutter, mix and wet output. Original DSP,
fixed9220bytes history/LFO storage, prepared coefficients and smoothed parameters.
Eight UI pages; schema7 saves48controls/source and preserves v1–v6. Extended the
bounded synth probe to actually navigate/edit TAPE and save/reopen. The fallback
tone still bypasses the effects. Credits now distinguish elly's native instrument
work, ETRNLIAM's original GrainBrick and research acknowledgements. See ADR0010.

Testing caught float wrap rounding a tiny negative read position to the delay
buffer length. Replaced float wrap with integer wrapping of a positive delay's
whole part; rate/boundary/block tests now pass. A separate attenuation-test
expectation incorrectly compared driven output with undriven output; corrected
it to the analytic −12dB power ratio for identical drive. No output gain was
changed to satisfy that incorrect test.

## Checks

- Host17 CTest and ASan/UBSan/LSan16 CTest passed. Sanitizers ran outside ptrace
  with leak checking intact. Tooling8, hardware tooling8, runtime4 passed.
- ARM build, dependencies and tape clang static analysis pass. No new shared libs.
- Same tape numerical/graph suite passed on physical ARM; unitSHA256:
  21e5052ac510ff4d5c9be6de02cba3e2c0c6b96fb2f1b705dca36abd6d9748a5.
- ZIP86293bytes was repeated byte-for-byte. No unit executable enters packages.
- Six bounded stock-preload/dummy cases passed: v6 migration/browser, standalone
  synth/TAPE edit, reopen/edit, WAV full graph, missing-WAV edit preservation,
  restored WAV reopen. All software clips0; migration had one lateness hint,
  worst20836us/1024. Full WAV graph worst15454us; these are not listening tests.
- Native fixtures preserved owner WAV/v6 hashes; all fixture/unit/profile files
  removed. Only current + verified rollback remain,2624KiB total.

## Physical acceptance

Owner: “Effect works; clean sound; saved” after instructed tape wet/dry comparison,
MENU/reopen and final exit. Completed foreground log2999callbacks/3070976frames,
1024frames per callback, span63950ms, worst17204us,late0,grains0,drops0,clips0.
67parameter updates; built-in synth source. Final saved state ended DRIVE0/MIX0,
WOW0.5/OUTPUT−6. Do not misrepresent final bypass as proof the whole session was
wet, or the owner's listening confirmation as a measured spectrum/latency result.

CURRENT/LAST_GOOD d89355fb031f; appSHA256:
a3a7e16ed77ec2ecf51b25cd73ed1b1a0fc0dad587497edba08b1566d9a4b0a6.
Verified rollback f9b6dabe5516 alpha7 remains. Alpha6 was matched against its host
archive and exact-file-pruned before deployment. No system changes/screenshots.

## Timing investigation

Offline48k/512-frame workloads; VNC2FPS and SSH enabled. First run93blocks,
repeat187blocks. Observed clock before/after first1200/1200MHz, repeat1000/1200MHz;
this does not establish clock throughout either run. No governor/clock changes.

| Full graph | Tape | First p95/max us | Repeat p95/max us |
| --- | --- | --- | --- |
| Normal grains + synth/FX | Off | 2922/3384.333 | 2972.667/3203 |
| Normal grains + synth/FX | On | 8824.667/14588 | 3049/11038.833 |
| Dense grains + synth/FX | Off | 4380/4655.333 | 4087/6298.833 |
| Dense grains + synth/FX | On | 11919.333/24842 | 4536.667/14581 |

All benchmark clips0. Dense fixed voice cap skips grains intentionally; repeat
138spawned/22skipped, equal with/without tape. A separate186block per-stage profile
measured tape p95/max281/1012.167us; grain2112.834us, EQ534.667us, reverb1685.833us
atp95. The first timing spike was not reproduced atp95; root cause is unresolved,
so no speculative optimization was made. Outliers exceed a512-frame deadline in
some offline cases. Current audio backend uses1024frames; neither comparison
proves zeroXRUNs or a worst-case bound. Collect longer VNC-off/live resources later.

## Reproduction and next action

Host: cmake host-debug build + ctest; sanitizer build + ctest outside ptrace;
ARM: scripts/build.sh miyoo-armv7-release; packaging/package.py release-version.
Private alpha8-native-test, repeat, stage-profile, acceptance and deployment
scripts/output record the exact commands; do not publish endpoints or raw logs.
No routine screenshots were taken. Next: practical resonator/modulation and
sample/project tools, keeping short working gates and later reliability soaks.
