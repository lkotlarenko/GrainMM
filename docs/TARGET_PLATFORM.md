# Target platform

GrainMM targets **Miyoo Mini Plus running OnionOS**, physically checked on
OnionOS4.3.1-1. Other Miyoo models and firmware versions are untested.

| Area | Verified contract |
| --- | --- |
| CPU/ABI | Two Cortex-A7 ARMv7 cores;32-bit little-endian hard-float; NEON/VFP reported |
| Build | Pinned SDKv0.0.3, GCC8.3.0, glibc2.28 sysroot; Cortex-A7/VFPv3-D16, hard-float, O2 |
| Runtime | Linux4.9.84; device glibc2.28 and SDL1.2 with SigmaStar dependencies |
| Display |640×480 software surface; app transforms drawing rectangles for physical180° orientation |
| Audio | Negotiated SDL dsp/OSS S16 stereo48kHz/1024frames in tested runs; backend XRUN count unavailable |
| Memory |103360KiB kernel-visible; initial discovery about71–73MiB available, not a permanent app budget |
| Storage | App/GrainMM only; state contains user WAVs/settings/patches/logs |
| Host | SDL2 adapter; host success does not establish physical audio/display behavior |

Full read-only findings, SDK archive digest, device constraints and measurements
are in [MIYOO_HARDWARE.md](development/MIYOO_HARDWARE.md) and the pinned toolchain
script. [Initial research](history/TARGET_PLATFORM_INITIAL.md) is preserved as history.

Keep the tested device SDL1.2 path. No CPU policy/mixer/rootfs/library/service
changes. No overclocking. Do not infer USB audio, BLE, microphone, power or sleep
support from generic hardware claims. Extended input/headphone/power and VNC-off
profiling remain separate acceptance gates.


## Musical connections

Stock Miyoo Mini Plus has no Bluetooth; GrainMM does not support Bluetooth MIDI
or Bluetooth headphones. USB audio/MIDI, network MIDI/sync and live audio input
are unsupported. A note number in the UI describes pitch, not a MIDI connection.
The built-in speaker cannot demonstrate stereo width; use headphones or an
appropriate stereo destination. Hardware modifications are outside tested scope.
See the [Bluetooth hardware-modification project](https://github.com/MagneFire/mmp-kcx)
for hardware context, not a supported GrainMM setup path.
