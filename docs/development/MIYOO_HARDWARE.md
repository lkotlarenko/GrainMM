# Physical Miyoo Mini Plus — verified 2026-11-17

Sanitized findings from read-only SSH discovery and bounded approved app tests.
No credentials, endpoints or raw captures belong in this repository.

| Area | Observed evidence |
| --- | --- |
| OS/kernel | OnionOS v4.3.1-1; Linux4.9.84 armv7l; BusyBox1.20.2 |
| CPU | Two ARMv7 cores, implementer0x41/part0xc07 (Cortex-A7); device tree INFINITY2M SSC011A-S01A-S, sstar,infinity2m. Exact silicon marking not inspected. |
| Instructions | NEON, VFPv3/VFPv4, VFPD32, integer division, Thumb/Thumb2 reported; ARM hard-float loader present |
| Clock | ondemand; available400/600/800/1000/1100/1200MHz; initial reading1000MHz; no policy changed |
| Memory | Kernel-visible103360KiB; initial MemAvailable73004KiB; later71896KiB; swap131068KiB, all free. Do not assume all128MiB nominal RAM is available to apps. |
| Display | 640x480,32bpp; virtual640x1440; stride2560; reported current mode100.060Hz; RGB8/16,8/8,8/0,8/24 |
| Libraries | glibc2.28; libstdc++6.0.25; ARM hard-float SDL1.2 SONAME in customer library directory, with SigmaStar dependencies. SDK executables actually run. |
| Audio | SDL reports dsp/OSS,48000Hz,S16,stereo,1024 frames. ALSA also exists; pcm0 remains owned by the existing audioserver in SETUP at8kHz and is not a valid measurement of GrainMM's MI/OSS stream. XRUNs unavailable through current adapter. |
| Input | gpio_keys on event0. Existing VNC supports mapped arrow/A/MENU events; physical A playback and upright display confirmed by user. Full physical-button matrix incomplete. |
| Storage/tools | About4.7GiB SD free at discovery; tar,sha256sum,cmp,readlink,timeout available. No readelf/strace/gdb found in inspected PATH. BusyBox timeout uses `timeout -t SECONDS command`. |

The authenticated virtual SSH account executes with UID0; no services, firmware,
network configuration, governor, mixer settings or unrelated apps were modified.

## Native evidence and limits

With existing VNC set to2FPS by the user, a bounded eight-case offline DSP probe
ran successfully. Dense stereo80grains/s,250ms: p95=2473.333us,max=3829.333us per
512 frames; arithmetic deadline10666.667us. Only93 blocks per case; no real-time
soak or audio-quality claim. Available RAM and swap were unchanged across this probe.

The first silent app used4212KiB RSS and about13.53% of one CPU over3.03s.
The corrected upright tone screen used4216KiB RSS,3 threads and about13.25%
over3.02s (live listening activity may overlap). USER_HZ=100 verified from auxv.
One tone run exited normally after16667 callbacks, late-hint count0,max822us;
these hints are not measured hardware underruns. User heard a normal but very
quiet test tone. Headphone tests, detailed listening, VNC-off measurements,
startup/sample timings and long soaks remain incomplete.

## Confirmed display fix

Unrotated output was physically upside-down. SDL's UD shadow rotation failed
with its missing32bpp blitter. Keep native32bpp and rotate drawing rectangles
180 degrees inside Display; the user confirmed the resulting screen stays open
and upright. No framebuffer/system config file is changed.

##2026-11-18 update

Physical granular launch/play/edit/MENU and parameter save/reopen now work;
see DEVICE_TEST.md for versions, counters and remaining tests. Stock libpadsp
ignores openat's directory descriptor; keep Onion routing and use descriptor-
anchored procfs paths in the app. Four native loader cases and two native preset
cases pass. These dummy-driver cases are separate from owner-confirmed listening.

SD footprint before pruning was3328KiB for all GrainMM data (five test releases).
Three obsolete releases were host-archive/hash verified and removed with exact
file paths; host recovery archives retained. Retain current/verified rollback
rather than accumulating builds on SD. No user files outside GrainMM were touched.
