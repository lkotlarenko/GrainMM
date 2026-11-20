# Development status

Updated 2026-11-20. **Candidate: 1.0.0-alpha.1. Accepted device: alpha25.**
GrainMM by elly (@lkotlarenko), inspired by GrainBrick by ETRNLIAM, with permission
and attribution. Musical usefulness takes priority over identical upstream DSP.

## Current milestone

Implemented seven musical additions: oscillator B; generated wavetable morph/scan;
sub oscillator; slice swing; variable phrase length; rotation/reverse/bounce;
repeating accents. Three new pages append18 controls to the old132-field prefix.
Schema18 keeps old1–17files. Patches/snapshots/export carry the added settings.
SOURCE now has two banks; GROOVE lives in TEMPO.

The [validation record](development/V1_VALIDATION.md) separates actual checks,
failures/corrections and outstanding target work. Compilation is not a physical
success claim. The major number is a feature milestone; this remains an alpha.

## Feature coverage

| Area | Implemented candidate behavior | Remaining |
| --- | --- | --- |
| Granular source |16 voices, trim, envelopes, stereo, reverse/jitter/scan, harmony, EVOLVE | Independent layers, improved resampling |
| Synthesis | Dual oscillators; table morph/scan; sub; collision source; generative pad/events/scenes | New-feature target listening/headroom |
| Sequencing | Q48 clock, generated/manual16steps/cuts; swing, length, rotation/order, accents | Parameter automation, richer arrangements |
| Effects | Filter, EQ, compressor, tape, crusher, chorus, resonance, rhythm, delay, reverb | Shimmer, routing, timing investigation |
| Output | Gain, linked sample-peak limiter, balance/mono | Longer headphone/speaker checks; no true-peak claim |
| Media/state | Rooted PCM16 WAV browser,16patches, snapshots, settings18, full-graph WAV export/resampling | Live recording, richer exchange |
| Interface | Six parameter cards, grouped pages, explicit source banks, six palettes, help/reset | New-bank device usability check |

MIDI, USB audio, Bluetooth connections, multitrack and GrainBrick project loading
are unsupported. Hardware limits are in [platform](TARGET_PLATFORM.md).

## Accepted physical checkpoint

Alpha25 owner checks confirm clean sound, saving, pattern/WAV export and clear
TOOLS navigation. CURRENT/LAST_GOOD **4f445a4fb862**; accepted prior build
**1f259d9b5b21** retained as rollback. Alpha25's31native app cases and numerical
export probes are historical acceptance for that build, not the 1.0 candidate.
See [alpha25 baseline](history/baselines/ALPHA25_BASELINE.md) and
[device acceptance](DEVICE_TEST.md). Never prune owner state/exports.

## Release preparation

Player guides were rewritten, dated baselines/research archived with links fixed,
packaged notices brought current and publication scope kept explicit. Local
installation/matching-source archives must agree with tested code.
Longer stress/listening, power/sleep and VNC-off CPU/RSS remain open; backend XRUN
counts are unavailable. On2026-11-20 the owner authorized final review, native
source/CI publication and the1.0.0-alpha.1 prerelease. Stable acceptance remains
separate; release notes must disclose the new-feature physical-test gap.

[Resume](development/RESUME.md) · [Roadmap](development/ROADMAP.md) ·
[Release readiness](development/RELEASE_READINESS.md) · [Docs](README.md)
