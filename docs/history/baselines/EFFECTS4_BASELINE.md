> Historical evidence. Current checkpoint: [RESUME.md](../../development/RESUME.md).

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


# Four-page feature build — 2026-11-18

Candidate0.1.0-alpha.4 / `deebcb9073ad` is installed. CURRENT selects it;
LAST_GOOD now selects `deebcb9073ad`; verified `f8ebc22ce5fb` is the only rollback.
Owner confirmed both effects, clean audio and saved settings.
ARM appSHA256 `aa5cd77cc3e5de134a2efc8f7b9f154b91ee2c93c6a8d86c84ae23ce35bbccee`.
Installation ZIP73926bytes. Target/runtime/toolchain unchanged from BROWSER_BASELINE.

## New working code

X cycles GRAIN,DELAY,EQ,REVERB. Six real EQ gain controls, six real reverb controls,
prepared coefficient snapshots, v4 source/control persistence and clipping telemetry.
Portable code performs no callback I/O/allocations/locks. New reverb is an original
musical equivalent, not an upstream binary/source transplant. ADR0007 records design.

Host15CTest and ASan/UBSan/LSan14 suites pass. Tooling8/hardware-tools8 pass. Native
ARM ELF/dependency checks pass; no new shared dependency. Numerical tests cover
reverb causality, stereo width, decay, arbitrary blocks, reset, finite boundaries,
prepared graph/mute and allocations. Preset tests preserve original v3 bytes while
writing v4; source/browser tests use v4. Extended tests are deferred to follow usable
builds per owner preference, without hiding these remaining acceptance items.

## Native silent tests

Under stock audio preload/search paths and SDL dummy audio:

- Copied v3→v4 migration + browser load passed; max callback7068us/1024,late0,
  clipped0; new source resumed silently and original fixture/source hashes held.
- All effects enabled in v4, resumed and edited through real SDL event loop:
  max12625us/1024,late-hint1,clipped0,0grain drops; saved EQ/reverb values retained.
- Missing saved WAV: visible/logged error + silent tone, exact v4 bytes preserved.
- Temporary fixtures removed by exact name after hash verification. No owner state
  was overwritten to manufacture the test. Existing v1/v2/v3 remain for rollback.

These are not audible quality, backend XRUN or hardware latency results.

## Short combined benchmark

Native SDKv0.0.3/GCC8.3 -O2, VNC2FPS + SSH, app closed, CPU observed1200000kHz
before/after; clock was not locked/changed. --seconds1 per case,93blocks of512frames
at48kHz, offline synthetic stereo. Full means six EQ bands±6dB + delay MIX0.65 +
reverb MIX0.5/DECAY12seconds. Delay768016bytes; reverb186912bytes.

| Case | p95 us/512 | Maximum us/512 | Software clipped samples |
| --- | ---: | ---: | ---: |
| Default grains, all effects | 2707.667 | 2908.333 | 0 |
| Dense grains, all effects | 3923.000 | 3981.333 | 0 |
| Dense grains, dry EQ/reverb | 2253.833 | 2590.666 | 0 |

Dense250ms/80grains per second exceeds16voice capacity:64spawned/16skipped grains
in these short cases, an intentional voice-cap policy. Default cases have no drops.
Other dry cases had scheduling outliers up to12962.166us despite lower p95 values;
short offline runs do not prove WCET or uninterrupted live output. Actual adapter
buffers are1024frames; per512 timing is the benchmark workload, not negotiated size.

## Device boundary and next action

Only current candidate + verified alpha3 rollback are installed. Olderede matched
its host tar before nonrecursive exact-file pruning. App footprint2432KiB, free SD
about4.7GiB. No new device images, services, mixer/clock changes or other-app writes.

Owner check: low-volume EQ/reverb sound, page controls, MENU/save/reopen, normal exit.
Then collect bounded logs, promote only if confirmed, and continue useful grain
modulation/oscillator/routing work. Do not spend the next iteration on exact original
DSP matching. Longer effects/headphone/suspend/VNC-off tests remain stable gates.

## Owner foreground result

Owner confirmed EQ/reverb sound changes, clean playback and saved settings. A
completed real OSS/MI foreground session logged4883callbacks,5000192frames,
1024frames/callback,span104150ms,worst12372us,late-hint0,1216grains,0grain drops,
61parameter updates,0clipped samples and normal exit. v4 contains the selected WAV
and reverb edits; older v1/v2/v3 hashes remain preserved. LAST_GOOD promoted only
after this result. The first collection overlapped an active session; retry after
exit found the completed log and v4 file. No system recovery/change was needed.
