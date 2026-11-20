# ADR0025 — Steerable drone events, useful level range and live voices

2026-11-19. Owner reports alpha21 runs but is quiet at maximum, too dark and visually
sparse. Offline rendering of the saved patch measured −30.13dBFS RMS with its FX,
−29.99 dry, versus −16.99 for the basic sine at the same0.20 level. These are host
PCM measurements, not acoustic SPL. Source range was the useful fix; system mixer,
CPU policy and services remain untouched. Supersedes ADR0024's level/root/mood limits.

LEVEL now spans0–1, displayed0–100%, retaining all previous numeric levels. It is
not an automatic gain increase. Recommend60% for initial listening at low hardware
volume. ROOT36–84 and eight moods include brighter registers/voicings. Fresh defaults
are root60/major, still LEVEL0 and silent start. Earlier patches retain their root,
mood and level. Editing snaps only step-sized numerical residue near zero, so a
shown zero cannot leave a nominally bypassed source/effect processing tiny values.

GENERATE appends event mix/rate, pitch glide, stereo width, seed and chord pattern.
Four short melodic voices supplement four sustained voices. Prepared pitches and
fixed tables are reused; events have latched pitches/pans and attack/squared-release
envelopes. One bounded four-slot search per event; no stealing, heap, I/O, logs,
locks or transcendental pitch design in render. Max2Hz nominal events, jitter±50%,
short envelopes and explicit source headroom. Shared rack/layering can still clip
with excessive boost; lower source levels. Seed steers future RNG decisions without
resetting phases. Independent note RNG avoids consuming the harmonic/air streams.
Five patterns include a chord HOLD that leaves texture/events alive. Width0 is mono.

Four lock-free packed peak bytes each for sustained/event voices and a monotonic
trigger count extend the existing visual snapshot. Source display shows real activity
and target note labels, not pitch detection/final-output spectrum. Drawing stays on
the UI thread with cached layout,20Hz cap, raw surface copy and physical rotation.
No continuous captures; numerical bounds and owner inspection validate the scene.

Settings15 append six fields after the unchanged120prefix. Missing-only fallback
keeps1–14files intact; old patches set event mix0 to retain pad-only sound. New
patches require alpha22+. No sample copies, added dependencies or upstream DSP/assets.
Source controls remain separate from FX and temporary snapshots preserve transport.
Tests cover all patterns/new moods/rates, mono/headroom, deterministic chunk/reset,
allocation, relative gain, old schemas and actual SDL generation/play/save/reopen.
