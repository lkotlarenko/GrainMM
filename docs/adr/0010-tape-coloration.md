# ADR 0010 — Bounded tape-style coloration

Date: 2026-11-18. Status: implemented; physical acceptance tracked separately.

Add a six-control TAPE page: DRIVE DB0..24, TONE HZ500..16000, WOW MS0..2,
FLUTTER MS0..0.25, MIX0..1, OUTPUT DB−18..6. Default MIX0 preserves old sounds.
This is a musical coloration effect, not a physical magnetic tape simulation.

Original wet path: odd soft clip x/(1+abs(x)), one-pole lowpass and a 3ms baseline
fractional delay modulated by linked sine LFOs at0.6Hz/6Hz. At48k, fixed delay
history + LFO table use9220bytes; same fixed storage supports8–96k. Interpolation
uses positive delay decomposition and integer wrapping, avoiding float rounding
of a tiny negative position to the buffer length. Feedback is absent.

Saturation harmonics are not oversampled; high drive can alias. MIX blends a
short delayed wet path with dry, so partial mix can color phase like a chorus.
OUTPUT affects wet only. These are explicit quality/cost compromises for an
initial handheld implementation. Do not advertise analog emulation or alias-free
processing. No hysteresis, hiss, recording, tape transport or wow-rate controls.

Prepare exponential gain/tone coefficients on the control thread. Smooth six
controls over approximately30ms; one fixed sine table is built at construction.
Callback has no allocation/files/app locks/trigonometric coefficient design.
Settled MIX0 returns exact input without per-sample work. Bypass clears history
once, and global stop/reset clears history to avoid stale sound on resume.

Graph: grains + oscillator → EQ → TAPE → COMP → DELAY → REVERB → safety fade.
The quiet220Hz fallback still joins after effects. Schema7 appends six tape
fields to schema6; older versions are read on absence only and never overwritten.
Missing-source recovery remains independent of decoded Sample ownership.

PocketTracker was reviewed for lifecycle/credit organization; no implementation
was copied. See development/POCKETTRACKER_RESEARCH.md. Tests cover transfer gain,
tone attenuation, block invariance, boundaries, finite-input containment, stereo
isolation, mono, bypass/reset and graph mute. Native audio acceptance is separate
from host DSP and dummy lifecycle results.
