# ADR 0001 — Native audio-first device proof

Date: 2026-11-17. Status: accepted for prototype; backend provisional until MM+ test.

| Option | Runtime/memory and build implications | Decision |
| --- | --- | --- |
| A: Python/Pygame/NumPy ARMv7 | Closest control/UI behavior, but no verified device Python3 runtime, NumPy copies/process costs unknown and AArch64 DSP source missing. A 19.79 MiB PCM16 sample set becomes about 39.6 MiB float32 before duplicate buffers, interpreter and UI. No measured RSS/CPU yet. | Not default |
| B: C++17 portable DSP + SDL | SDK and host compilers available; few runtime dependencies, audio-thread ownership and fixed buffers feasible. Native 640x480 surface 1.17 MiB at 32bpp; proof needs no samples. Feature parity must be implemented/tested incrementally. | Default |
| C: Python controls + native audio | Still needs unavailable verified runtime; fewer Python DSP costs, but process/IPC and packaging remain. No demonstrated advantage at current scope. | Deferred |

Use host SDL2 and target SDK SDL1.2.15, with common portable Tone and small
surface/audio adapters. Only two backend-specific display paths, no GUI framework
or GPU requirement. Target SDK has fbcon/OSS symbols; actual device backend,
audio device and format must be observed. Desired PCM is 48k/S16 stereo with
1024 frames (21.33ms); accept returned frequency and mono/stereo native S16,
reject unsupported formats. 256/512 are experiments, never forced latency.

Verified upstream Onion keymap: A=SPACE, MENU=ESCAPE. UI repeat suppression and
RAII exit are implemented; long MENU, global shortcuts, sleep/power interactions
remain physical-test items. Do not intercept POWER or modify volume/governor.

Reject current steward-fu SDL2 as default pending review: at pinned
0631abc8e8916db6f9bc7e2afd0c22913d092a29, mini input indexes a two-element
array by ev.value (EV_KEY repeat can be 2) without a visible guard; mini audio
uses unsigned frame/rate sleep arithmetic that can underflow below 10ms and
sets hardware volume. These are static findings, not reproduced hardware bugs.
Generic SDL2 claims alone do not validate its device fork. No fork code is copied.

Proof scope: silent start, bounded 220Hz test tone, software display, observed
PCM format/callback-time hints, A toggle, MENU exit, local SD logs. No fake
GrainBrick engines, MIDI, project controls or unsupported live input. Add P2
portable bounded WAV/grains only with regression tests; preserve original
project semantics only where rights and an actual migration specification permit.

Revisit after device diagnostics, input/display/audio run, dependency inspection
and measured deadline/RSS data. Interfaces keep a backend change reversible.
