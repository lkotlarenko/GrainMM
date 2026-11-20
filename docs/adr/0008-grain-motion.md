# ADR0008 — Expressive grain motion

Date:2026-11-18. Status: implemented; host/native checks and owner listening/save/reopen passed.

Add six independent musical controls to the existing16voice native engine instead
of duplicating a GrainBrick layer/runtime. Reverse probability0..1, pitch jitter
0..12semitones, size/timing randomness0..1, per-grain pan spread0..1 and continuous
sample scan-0.5..0.5cycles/second. Defaults0 preserve the existing stationary mode.
MOTION is a fifth real page. Gain and sample limits stay unchanged.

At spawn, latch direction/pitch/duration/pan into the voice. Size stays5..250ms;
pitch jitter multiplies base pitch by an exponential semitone ratio once per new
grain. No allocation/file/locks; at most one spawn attempt per output frame. Timing
jitter varies the sample-clock interval with a5%minimum, preserving bounded work.
Xorshift random state remains seeded and resettable; results do not depend on block
size. Saturated voices still skip new grains rather than steal sounding ones.

Negative increments wrap using floor-based modular indexing before the next sample
read, including singleton buffers and96k→8k rate conversion. Pan uses a simple
bounded linear balance law, retaining stereo input rather than discarding it.
Sample scan advances on the audio clock while playing, independent of playback
pitch; pause freezes it, explicit engine reset restarts it. Existing envelopes and
parameter ramps are retained. Linear playback interpolation remains non-bandlimited;
high pitch/jitter can alias, and very short samples retain loop discontinuities.

Write30controls/source to parameters.v5, reading v4→v3→v2→v1 only on absence. v4
EQ/reverb/source values are retained; old files are unchanged. Control preparation,
fixed queue, mute and safe sample replacement contracts are unchanged.

Host tests cover reverse wrapping, source/output-rate extremes, deterministic
randomized arbitrary blocks/reset, singleton buffers, stereo variation, bounds,
allocation-free rendering and v4 preservation. Native performance/listening is
checked on a staged candidate; retain the verified alpha4 rollback until confirmed.
