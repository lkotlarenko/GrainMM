# ADR0009 — Built-in synthesis and linked compression

Date:2026-11-18. Status: implemented; host/native tests and owner foreground checks passed.

Add practical musical features while keeping one portable audio graph. Instrument
accepts a nullable non-owning Sample pointer and holds optional in-place Granular,
Oscillator,Compressor,EQ,Delay,Reverb and a quiet Tone fallback. The adapter always
owns one Instrument, including no-WAV operation. Close/join before releasing the
sample remains mandatory. Replacement constructs the new graph before discarding
the old one; source/parameter UI state is adopted only after successful handoff.

## Oscillator and dynamics

Oscillator uses a1025-point sine table, a finite odd-harmonic triangle and quadratic
edge correction on saw/pulse. Pulse duty-cycle DC is removed and amplitude normalized.
The mathematical approach is informed by [Pekonen's oscillator research](https://research.aalto.fi/en/publications/filter-based-oscillator-algorithms-for-virtual-analog-synthesis/);
this is independently authored code, with no copied implementation/dependency.
Tests compare the saw with a finite Fourier reference, verify sine pitch/level,
pulse DC, stereo/rate limits, block invariance and mute. It reduces aliasing rather
than guaranteeing alias-free synthesis under every modulation.

UI prepares frequency/detune/vibrato coefficients. Audio uses table/polynomial
operations, smoothed level/frequency/duty and waveform crossfades. Note24–96,
level0–0.25, detune0–30cents total stereo separation, duty0.1–0.9 and5Hz vibrato
0–100nominal cents. Vibrato uses a small fractional-frequency modulation, so its
cents depth is approximate. No allocation, files, locks or per-sample transcendentals.

Compressor links both channels through a peak envelope. UI prepares attack/release,
threshold, ratio exponent and makeup. The gain curve uses a bounded pow operation
every8frames; envelope/gain/dry-wet smoothing runs per frame. Dry defaults to an
exact bypass. This is compression, not a final limiter; PCM clipping is still counted.
Tests verify the steady dB curve, stereo link, unity ratio and block invariance.

Current routing: grains + oscillator → EQ → compressor → delay → reverb → mute/PCM.
The220Hz fallback joins after effects and stays quiet, even with a saved wet rack.
It plays only with no WAV and OSC/LEVEL0. Browser X explicitly returns to this safe
source, clearing oscillator level transactionally. Increasing LEVEL activates the
musical oscillator. Both source paths start silent and all tails mute on A/MENU.

## Controls, state and code organization

OSC/COMP are sixth/seventh pages; no-WAV launch focuses OSC. GRAIN/MOTION edits are
disabled without a WAV and show a source hint. Other pages process the musical
oscillator. L/R add previous/next navigation with repeat guards; X remains next.
Wave names replace numeric choices. All42controls/source save in parameters.v6;
v1–v5 remain untouched, with strict absent-only migration and corruption refusal.
The historical source enum Tone means built-in/no-WAV; nonzero oscillator level
selects synthesis while retaining compatible source values.

Dummy-only event injection moved from main.cpp to platform/LifecycleProbe. Its
bounded synth scenario drives actual oscillator/compressor controls, saves and
reopens without a sample. It refuses actual audio, explicit/restored WAV sources
and durations shorter than2seconds. The real-time callback never runs probes.
