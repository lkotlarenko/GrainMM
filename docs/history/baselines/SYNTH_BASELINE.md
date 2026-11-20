> Historical evidence. Current checkpoint: [RESUME.md](../../development/RESUME.md).

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


# Built-in synth and compressor acceptance — 2026-11-18

Alpha6 / `adaa4e48e88f` is installed and owner-verified. CURRENT/LAST_GOOD select it;
verifiedalpha5 `e5f93ff6d27b` is the only rollback. AppSHA256:
`3ba16290554d37e5d5323ea49f696813d2aec814a5c79f99c07a0b0e890777c5`.
Installation ZIP83486bytes, byte-identical on repeat packaging. No new shared dependency.

## Working increment

Musical sine/triangle/saw/pulse oscillator without WAV or layered with grains;
stereo detune, duty/vibrato; linked compressor; seven real pages;42saved controls.
One nullable-sample audio graph, quiet fallback, transaction-safe replacement,
prepared snapshots, named waves and L/R page navigation. See ADR0009.
Probe injection is a bounded platform module rather than growing main.cpp.

Host16CTest and ASan/UBSan/LSan15 suites pass. Tooling8/hardware8 pass. ARM/ELF,
format/source/reference checks and local actionlint pass. New numerical suite
also ran successfully on the physical ARM device, including pitch/level/DC,
Fourier-reference saw comparison, compressor curve/link, block invariance,
allocation checks, no-WAV graph, mute and safe fallback. Native testSHA256:
`c8389bf58a61e63c8aa0d9981dcef61cbe56d54c45350365d24a1f28ff2960f6`.
The temporary test executable was hash-checked and removed by exact name.

## Native silent and performance checks

Under stock audio preload/search paths, SDL dummy tests passed:

| Case | Max callback us/1024 | Late hints | Software clips |
| --- | ---: | ---: | ---: |
| v5→v6 migration/browser | 9965 | 0 | 0 |
| WAV + oscillator + full effects/COMP | 11947 | 0 | 0 |
| No-WAV OSC/COMP UI save | 8746 | 0 | 0 |
| No-WAV reopen/edit/save | 20676 | 1 | 0 |
| Missing WAV, preserve stored v6 | 1132 | 0 | 0 |

Standalone probe drove actual UI LEVEL/WAVE/NOTE and COMP/MIX, confirmed source1
and level0.01→0.02 on reopen. Layered probe retained both sources and compressor
mix0.7. Missing-file recovery stayed silent and did not overwrite the selected path.
Owner v5 and WAV hashes stayed unchanged; all isolated fixtures were removed.
These are not audible quality, hardware latency or backend XRUN results.

Offline --seconds1 workload,93blocks/512frames/48kHz, native SDKv0.0.3 GCC8.3 -O2,
VNC2FPS+SSH, foreground app closed. CPU800000→1200000kHz observed, never changed.

| Workload | p95 us/512 | Maximum us/512 |
| --- | ---: | ---: |
| Standalone pulse/detune/vibrato + compressor/FX | 2073.666 | 2104.000 |
| Default grains + synth + all FX/motion | 2542.500 | 2601.000 |
| Dense grains + synth + all FX/motion | 4029.167 | 4514.834 |

All cases had0software-clipped samples. Dense random grains hit the fixed16voice
cap (70spawned/9skipped), not a hardware underrun counter. Background/DVFS and
workload differences prevent an optimization percentage or WCET claim. App audio
negotiates1024frames; benchmark blocks are512. The app was closed when foreground sampling ran. A separate controlled4second
native SDL-dummy/silent probe then recorded5784KiB RSS/HWM,3threads. Its two samples
span startup (1→3threads), so they are not a steady live-audio CPU baseline.
Process delta3ticks/system192ticks across2cores is about3.1%of one core during that
mixed startup/silent interval. Copied v6 and the owner's original stayed unchanged;
its isolated preset/log were removed. Physical fbcon/active-audio RSS remains open.

## Owner foreground acceptance

Owner confirmed built-in synthesis, clean audio, controls including L/R, compression
and saved values. Completed real OSS/MI session:3165callbacks,3240960frames,
1024/callback,span78470ms,max10404us,late-hint0,0grain drops,93parameter updates,
0software clips,1source switch and normal exit. Final source was built-in, oscillator
level0.05, with note/wave/detune/duty/vibrato edits. Compressor ended bypassed;
its listening check is owner-reported, while native numeric/graph tests above
verify processing separately. WAV-layer audible acceptance is not separately claimed.
Span includes browser pause; produced frames/span is not a DAC clock measurement.

v6 persisted; older v1–v5 and original sample remain intact. LAST_GOOD promoted
after checksum/log/state verification. App2560KiB; only current + alpha5 rollback.
Alpha4 matched its host tar before exact nonrecursive pruning. No new device images,
services, mixer/governor/network changes, unrelated files or public publication.

Remaining: longer combined soaks/headphones/power/VNC-off resources; additional
resonator/tape/envelope/routing features. One essential host synth UI image was
inspected and reused in README; no repeated navigation photography.

## Follow-up state guard

Final source review found an alpha6 edge case: with a missing saved WAV, editing
new no-WAV controls and exiting could replace the unavailable file selection with
built-in mode. The existing missing-file test covered no-edit recovery only.
The follow-up alpha7 retains source intent until an explicit browser choice and
adds edited-recovery coverage. Audio DSP remains unchanged; alpha6 is preserved
as its physically verified audio/UI reference, not the recommended release artifact.


Alpha7 completed: release f9b6dabe5516, appSHA256
81aabc7b7b9e26f57273d3f39d719401d74339c9668d715b394cb0d7edb2bb8a.
Host16 and ASan/UBSan/LSan15 passed, tooling8/hardware8 passed; sanitizer suite
required execution outside the ptrace sandbox with leak checks enabled. ARM ELF
and dependencies pass; public ZIP83619bytes was independently reproduced byte
for byte. No DSP changes or new benchmark claims.

Native stock-preload + dummy regression edited missing-file controls and saved
source2/input.wav; restoring that WAV reopened granular mode with unchanged saved
state. Both runs late0/clips0, max2413/467us respectively; these are silent tests.
Owner then confirmed alpha7 opens/exits normally. Final physical log checked;
LAST_GOOD promoted. Only alpha7 and alpha6 remain,2592KiB; alpha5 was verified
against its host archive before exact-file pruning. Owner WAV/v6 hashes preserved,
fixture removed, no screenshots. Exact scripts/output are private alpha7-recovery,
alpha7-prune and alpha7-acceptance records under.local/hardware.
