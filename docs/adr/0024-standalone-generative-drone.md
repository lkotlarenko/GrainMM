# ADR0024 — Standalone generative DRONE, separate from sample evolution

2026-11-19. Owner corrected the alpha20 interpretation: they want an automated
ambient synthesizer that runs without a WAV, can be steered and uses GrainMM's FX.
The M5Burner3NRPTY reference matches MIASMORA via its author's README. Reviewed
public behavior only; no source/assets/presets copied. GrainMM's implementation is
original, not a MIASMORA port or a claim of its25layer/event/export feature set.

Rename the useful sample-modulation page EVOLVE (stored18), preserving its fields.
New DRONE (stored19) is an independent four-voice source. Six appended controls:
LEVEL0–0.2(default0), ROOT NOTE36–60(default48), MOOD(minor/major/open/dark),
EVOLVE SEC4–60(default12), TEXTURE and MOVEMENT. No sample/input/hardware sensor
is required. Default start is silent; LEVEL enables the source, A controls play.
It can layer with other sources or run alone and enters the existing shared rack.
The fallback test tone is suppressed while this source is enabled/fading out.

Control-thread preparation designs four chord voicings per mood. A separate seeded
stream chooses another chord at each evolution interval, without adjacent repeats.
Four sustained oscillators glide smoothly to prepared frequencies. Voice weights
also drift smoothly; slow breathing, stereo pan and slight pitch drift are steered
by MOVEMENT. TEXTURE blends original generated sine/three-partial tables and quiet
band-passed noise. Noise filters are rate-scaled; the highest fundamental is bounded
for the three-partial tables. No transcendental pitch design, allocation, I/O,
locking or parsing in render. Fixed arrays/tables add about8KiB plus voice state;
no new sample buffers or duplicated delay/reverb. Lookup bounds/energy are explicit.

Pausing freezes the evolution clock; gain ramps stop sound. Root/mood changes and
automatic chord changes glide rather than reset phases. Source/effect state is
separate. Rack reset keeps the instrument; effects+modulation clears MOVEMENT;
all-controls reset disables it and pauses. Snapshot/patch storage includes controls,
not oscillator phase/noise history. Reopening is silent and begins a seeded scene.

Settings14 append6fields after the unchanged114prefix; old1–13files survive.
STYLE becomes UI-only20. Ten SOURCE cards use four columns within the existing
navigation panel; other groups retain three. No hidden pages or extra group loop.
One lock-free unsigned telemetry value carries real chord/cycle progress, never
mutable voice state. Cached source scene/header now identify the generative source.

Tests cover no-WAV stereo signal/power/DC/bounds, deterministic block/reset output,
autonomous chord changes, pause, all moods/rates, allocation guards, shared effect
processing/reset and actual SDL navigation/play/help/save/reopen. A reset-state
inconsistency caught by tests was fixed by initializing inaudible controls together.
Native timing and owner listening remain separate from host mathematical evidence.
