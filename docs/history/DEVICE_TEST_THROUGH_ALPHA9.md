# Historical snapshot

Preserved for engineering evidence; use the current [documentation index](../README.md).

# Latest physical increment — alpha6 built-in synth/compressor

Owner confirmed clean built-in synth/compression, L/R and other controls, saved
values and normal exit. CURRENT/LAST_GOODadaa4e48e88f; rollbacke5f93ff6d27b.
Real max10404us/1024,late-hint0,grain drops0,software clips0. Host/native numerical
and five silent smoke cases pass; waveform layering is not separately listening-
confirmed. See [SYNTH_BASELINE.md](baselines/SYNTH_BASELINE.md) for conditions,
CPU timing and remaining gates. No new device screenshots or system changes.

---

# Latest physical increment — alpha4 EQ/reverb

Owner confirmed both effects audible, clean sound, controls and saved settings.
CURRENT/LAST_GOODdeebcb9073ad, rollbackf8ebc22ce5fb. Real foregroundmax12372us/1024,
late-hint0,grain drops0,software clips0,normal exit. These are not backend XRUNs.
See [EFFECTS4_BASELINE.md](baselines/EFFECTS4_BASELINE.md). No new device images.

---

# Latest physical increment — browser/interface, 2026-11-18

Alpha3/f8ebc22ce5fb passed four native silent regression cases and owner browsing,
WAV loading, editing, sound, upright layout, save/reopen and normal exit.
AppSHA256:f6cdc3f767cd15a0f3051fbf8097849ba154cbe5f32080379e423d86b62b8a29.
Foreground worst callback6828us/1024frames, late-hint0, grain drops0; these are
not backend XRUN measurements. CURRENT/LAST_GOOD f8ebc22ce5fb; rollbackede retained.
App footprint2336KiB; no device-wide changes or new device images. Full evidence
and remaining stable gates: [BROWSER_BASELINE.md](baselines/BROWSER_BASELINE.md).

---

# Latest physical increment — 2026-11-18

Candidate0.1.0-alpha.2 / ede6c953a072 adds stereo delay, X page switching and v2
settings. Owner confirmed all controls, audible change, save/reopen and no audio
problem in a short check. Native synthetic benchmark and migration tests passed;
v1 preset SHA256 remained unchanged. No screenshot was needed.

AppSHA256:50a441c50e12cee211cce2d4dbdbe878ab18c42385275126f28a2036e5ed1736.
Native dense wet p95=5270.334us/512frames; these are offline workload timings,
not hardware XRUNs. Actual foreground lengths were1024frames, produced3641344
frames over75840ms, worst callback8432us, late-hint0, grain drops0; ordinary exit
and settings restore are logged. Full evidence/limits: development/EFFECTS_BASELINE.md.
Long-duration effects, headphones, sleep/power and VNC-off measurements remain.

---

## Previous granular-only acceptance

# Physical Miyoo Mini Plus acceptance

Updated2026-11-18. Facts below distinguish owner observations from automated
native tests. This is not a claim of full GrainBrick parity or stable acceptance.
No private device endpoints, credentials or raw captures are published.

Target: Miyoo Mini Plus, OnionOS4.3.1-1, kernel4.9.84, glibc2.28; verified hard-float
ARMv7, Cortex-A7, NEON/VFP. Build uses checksum-pinned SDKv0.0.3/GCC8.3, -O2,
cortex-a7/vfpv3-d16/hard. See [hardware report](../development/MIYOO_HARDWARE.md).

## Current candidate

Version0.1.0-alpha.1, ARM application SHA-256
`debb65406ba3e387d5097e0fa67d8cd12ba4c1840e16f53a8ee2d21f94fc1d6a`.
Native candidate029c82a5a13c; generated two-second mono44.1kHz440Hz input,
48000Hz signed16 stereo output,1024frames (arithmetic period21333us).
Existing VNC remains enabled at owner-selected2FPS; SSH monitoring is present.
No mixer, CPU policy, framebuffer configuration or Onion runtime files modified.

| Scenario | Evidence / status |
| --- | --- |
| Ordinary Apps launch, upright640×480 display | Owner-confirmed on corrected build; original panel was upside-down and SDL32-bit UD rotation failed. Native rectangle transform used. |
| WAV playback and D-pad parameter editing | Owner confirmed launch, sound, editing and MENU exit all work on loader-fixed09528bc5db33. |
| Silent start, A play/pause and ordinary MENU exit | Owner tested; exit logs close audio and leave graphics mode normally. Long MENU/global shortcuts remain untested. |
| Save edited values, MENU exit, reopen from Apps | Owner confirmed restored value on current029c82a5a13c. |
| Native save/reopen under stock audio preload | Two bounded SDL-dummy runs pass: density12→13, then restored13→14; no pending file; temporary fixture removed. Not an audible test. |
| Loader regression | Four controlled native dummy cases pass: baseline, preload, search-order, both. Before fix only preload cases failed. |
| Five-minute listening and exit | Owner reported clean sound and normal exit; final log confirms cleanup and no remaining app process.30/60-minute tests not run. |
| Speaker/headphones/hotplug/manual volume | Gentle tone heard as normal but very quiet in earlier test; complete routing/headphone matrix pending. |
| Reboot/sleep/power handling | Not run; no reboot or system changes authorized. |
| Full/slow SD, abrupt power during save | Host error/recovery tests only; no physical fault injection. |
| VNC-disabled CPU/audio baseline | Pending. No service setting changed automatically. |
| Actual backend XRUNs | Unavailable through current SDL/MI path. Lateness is only a callback timing hint. |

## Measured bounds, without overclaiming

The normal owner-operated loader-fixed sample session exited with4284callbacks,
2630grains,0dropped grains,126parameter updates, late-hint0 and max19551us.
UI requests1825/presentations163. Audio/dropout quality cannot be inferred from
these counters. SDK audio diagnostics include a vendor SetPubAttr warning before
successful setup; no undocumented mixer or system fix was applied.

Earlier eight-case native offline probe: dense stereo80grains/s,250ms,
p95=2473.333us and max3829.333us per512frames against10666.667us deadline.
This was only93blocks/case with VNC2FPS, not an audible soak.

A10.02second current-candidate process interval used4664KiB RSS/HWM and3threads;
CPU ticks increased by10 at USER_HZ100, about1.0% of one core. Playback state was
not independently sampled for that interval: treat it as a process observation,
not an accepted active-audio benchmark or a comparison with the older tone run.

During the owner-started listening interval, a second10.02second native process
sample used5432KiB RSS/HWM and3threads, with65CPU ticks: approximately6.49% of one
core at USER_HZ100. VNC remained2FPS; SSH monitoring was enabled. This short
interval is not peak CPU or VNC-off performance. Input log shows one A toggle
since launch. The owner subsequently confirmed clean sound during the listening
check and normal exit. This is an observation for that workload, not all samples.

Final current-candidate session:21036callbacks,6647grains,0dropped grain voices,
1parameter update, late-hint0, max8278us. UI requests8974/presentations15. Normal
cleanup closed audio and left graphics mode; no app process remained. Reported
buffer/rate imply an arithmetic deadline only; callback/frame counters are not
an end-to-end latency, actual sample-clock or hardware-underrun measurement.

## Remaining stable-release acceptance

Complete repeated launch/play/mute/exit cycles,
headphones/speaker, safe power handling,5/30/60minute audio tests and matching
VNC-off CPU/RSS conditions. Preserve user state and the known-good rollback.
No cross-build, successful process exit or headless test substitutes for these.


## Alpha9 larger feature checkpoint

Owner confirmed resonator/rhythm, SELECT help/B close/MENU, sample trim/window/
width, clean sound and save/reopen. Host18/sanitizer17 plus physical ARM numerical
and seven dummy lifecycle cases passed. Final real WAV session worst19195us/1024,
late0,software clips0; RSS6280KiB during a short interaction sample. Backend XRUNs
remain unavailable, and longer stress/headphone/power/VNC-off gates are pending.
See development/ALPHA9_BASELINE.md for precise conditions, hashes and limitations.
Only current alpha9 and verified alpha8 rollback remain. No public native release.
