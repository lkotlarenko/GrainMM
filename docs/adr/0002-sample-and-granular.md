# ADR 0002 — Bounded sample ownership and original granular playback

Date: 2026-11-17. Status: accepted for experimental portable P2 work.

The host-tested P1 adapter does not establish physical MM+ readiness. Continue
with independently testable DSP and sample handling while hardware testing is
prepared, without translating GrainBrick DSP or using bundled samples.

## File and memory boundary

Support only little-endian RIFF/WAVE PCM16, one or two channels, 8–96kHz.
Validate RIFF length, complete chunk bounds/padding, unique fmt/data, block/byte
alignment, rates and decoded memory before allocation. Unknown chunks are
skipped; data may precede fmt. Limit files to 32MiB, chunk count to 4096, decoded
values to 4,194,304 float32 (16MiB). Decode with 4KiB scratch into one immutable
interleaved buffer. No eager float64/stereo copies, process sharing or ffmpeg.
The supplied sample remains unchanged. RF64/RIFX, compressed, float, extensible,
24/32-bit WAV and streaming long files are explicitly unsupported.

Use a trusted caller-selected directory and one basename (<=128 bytes). Reject
slashes, backslashes, dot paths and control bytes. Open directory/file with
O_NOFOLLOW, then fstat the opened file as regular; O_NONBLOCK avoids FIFO hangs.
Parent components of the explicitly selected directory are trusted, not a sandbox
boundary; neither this API nor the offline output path is a general filesystem
sandbox. Open descriptors prevent path replacement races from redirecting reads.
Truncation during decoding fails; a mutable source could still yield changing
content and should not be edited during load.

References checked for format semantics:
https://learn.microsoft.com/en-us/windows/win32/xaudio2/resource-interchange-file-format--riff-
and https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatex .
No implementation code copied from those references.

## DSP and controls

A fixed 16-voice engine uses a sample clock countdown, seeded xorshift positions,
precomputed/interpolated Hann envelope and linear sample interpolation. Each
voice captures size, position and pitch at launch, so UI changes do not jump an
already sounding voice. Density updates preserve remaining interval phase;
new grains are skipped when the pool is full, with a diagnostic counter.
16-voice normalization plus 0..0.5 gain bounds output; master gain has a ramp
bounded by 20ms. Silent rendering clears the pool once and fills the remainder.
One-frame sources, arbitrary block sizes and mono/stereo output are tested.

Whole parameter snapshots cross a seven-message SPSC queue (eight physical
slots). Acquire/release publishes complete values; callback consumes at most
seven per block. All 32-bit atomics are compile-time lock-free checked. Main
retries the latest pending parameters when full; mute uses a separate atomic
that parameter updates cannot undo. The sample outlives SdlAudio and its engine.
SdlAudio stops/joins callbacks before destroying engines. ParameterEditor handles
UI selection/ranges independently of SDL and audio ownership.

## Limits and follow-up

Linear interpolation is an initial CPU-conscious method, **not band-limited
resampling**. Pitch-up/downsampling and nonperiodic sample boundary wrapping can
alias or reveal discontinuities; listening/spectral tests and a measured filter
strategy are needed before audio-quality acceptance. No automatic destructive
sample conversion. No claim of GrainBrick sound parity or click-free playback
for every user sample.

Project persistence, delay/feedback, waveform view, sample browser, envelope
choice and ECO/BALANCED quality selectors remain unimplemented. Avoid fake controls.
Offline WaveWriter creates output exclusively, checks frame counts, writes
little-endian PCM and flushes/closes explicitly. Failed output may remain partial;
this is not the atomic project-save API planned for a later increment.
