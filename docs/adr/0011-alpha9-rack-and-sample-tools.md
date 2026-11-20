# ADR 0011 — Tuned resonance, rhythm and non-destructive grain tools

Date: 2026-11-18. Implemented; physical acceptance recorded separately.

Alpha 9 adds three six-control pages and SELECT help. These extend the original
portable instrument; they do not import GrainBrick/PocketTracker DSP or assets.

## Resonator

RESONATE: MIDI NOTE 36..84, DECAY SEC 0.05..3, BRIGHTNESS 0..1, INHARMONIC 0..1,
WIDTH 0..1, MIX 0..1. Four parallel two-pole bandpass modes begin at harmonics 1–4.
Inharmonicity stretches upper modes; brightness weights them and their decay.
Modes at/above 0.45 sample-rate are omitted and remaining weights normalized.

The constant-peak-gain filter form is documented by Julius O. Smith in
[Introduction to Digital Filters](https://www.dsprelated.com/freebooks/filters/Constant_Peak_Gain_Resonator.html).
This implementation is independently written. Per-mode denominator is
1−2 r cos(theta)z^-1+r²z^-2, numerator weight×(1−r²)/2×(1−z^-2).
Radius exp(−ln(1000)/(decay×rate)) gives the requested approximate 60 dB decay.
Modal weights sum to 1, retaining bounded frequency-response gain for a stationary
bank; it is not an output limiter or an absolute transient amplitude guarantee.

Coefficient preparation/validation runs outside rendering. Double precision
history avoids fragile low-frequency/long-decay recursion. Retuning crossfades
two fixed banks over 20 ms, with one latest pending target; no callback allocation.
Width/mix smooth separately. Settled bypass skips processing; reset clears tails.
This is a simple resonator effect, not an ORBIX/Rings replacement or full parity.

## Rhythm

RHYTHM: TEMPO 30..240 BPM, DIVISION half/quarter/eighth/sixteenth, TREMOLO 0..1,
AUTO PAN 0..1, SHAPE sine/triangle, STEREO DEG 0..180. Attenuating tremolo and linear
pan use a fixed sine table and an audio-rate phase counter. Rate and depths smooth
on changes. Both depths 0 give exact dry bypass. STEREO DEG offsets the right
amplitude LFO; mono averages channel gains. No tempo detector, sequencer, MIDI or
transport synchronization is implied. Clock starts/reset with instrument playback.

Reference testing found float phase accumulation drift at higher rates; retain
double phase while prepared rates/control values stay float. No trigonometric
calls, allocation or parameter parsing inside process.

## Sample region and window

SAMPLE: START 0..0.99, END>=START+0.01..1, ATTACK/RELEASE 0.01..0.49, ENV BLEND 0..1,
STEREO WIDTH 0..1. Region fractions map to a nonempty half-open frame interval;
reads, interpolation neighbors and forward/reverse wrapping stay inside it.
Every voice latches region/envelope at spawn so edits cannot redirect a sounding
voice. Sample storage is immutable. Width changes smoothly on mixed grain output.

ENV BLEND 0 retains the original Hann;1 selects a linear attack/hold/release window.
Intermediate blend interpolates both. ATTACK/RELEASE affect that custom component;
they do not alter the Hann at blend 0. Endpoints remain 0. Defaults keep whole-file
Hann/stereo behavior. Tiny files still have at least one readable frame.

## Graph, state and UI

Grains + OSC → RESONATE → EQ → TAPE → COMP → RHYTHM → DELAY → REVERB → mute fade.
The fallback tone bypasses effects. Schema 8 appends 18 fields to schema 7 (66 total)
and reads earlier versions only on absence. Never modify original WAVs or older
saved-state files. Missing-file source intent remains independent of decoding.

SELECT opens/closes cached help over the current view, preserving playback;
A still plays/pauses outside the sample browser, B closes help, MENU always exits.
Do not edit parameters while help is visible. Frame caching distinguishes help,
browser and parameter views. Test actual input/display on the Miyoo separately.
