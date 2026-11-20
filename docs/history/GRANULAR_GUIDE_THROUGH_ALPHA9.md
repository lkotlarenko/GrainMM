# Historical snapshot

Preserved for engineering evidence; use the current [documentation index](../README.md).

# Experimental granular playback

This describes the early native granular instrument. See DEVICE_TEST.md for
physical checks and release notes for supported versions. It is a small feature
subset, without full GrainBrick parity. GrainBrick is by ETRNLIAM; this native
engine is independently written. Use your own legally usable PCM16 WAV; no sample or preset is shipped.

## Host and offline use

```sh
sh scripts/build.sh host-debug
build/host-debug/grainmm --sample-dir /path/to/your/samples --sample example.wav
build/host-debug/grainmm-render --sample-dir /path/to/your/samples --sample example.wav --output new-render.wav --seconds 5
```

The player starts silent. A/Space toggles playback, MENU/Escape fades/exits.
Up/down selects a parameter, left/right edits it; X cycles GRAIN/DELAY/EQ/REVERB/MOTION/OSC/COMP.
When --state-dir is supplied, grain and delay values restore at startup and
edits save on normal MENU exit; signal/power interruption does not auto-save.
Omit --sample to use the immediately available gentle 220Hz test-tone proof.
The offline renderer runs without SDL and exclusively creates a stereo 48kHz
WAV, with a fade tail and peak/RMS/DC/count report. Existing output is preserved.

| Parameter | Range | UI step |
| --- | --- | --- |
| Size | 5–250ms | 5ms |
| Density | 1–80 grains/s | 1 |
| Pitch / source rate multiplier | 0.25–2 | 0.05 |
| Position | 0–1 of sample | 0.02 |
| Spread / random position distance | 0–1 | 0.02 |
| Gain | 0–0.5 | 0.01 |

Sample rate conversion accounts for source/output rates. Position jitter is
clamped at sample endpoints. Voices wrap; active voices retain their launch
settings. Density can exceed the 16-voice cap for long grains: DROP counts skipped
new grains. The mix divides by the fixed cap to bound gain rather than pumping
with changing voice count. Defaults are intentionally conservative.

Accepted input: complete little-endian RIFF PCM16 mono/stereo 8–96kHz. Decoded
cap 16MiB (about 43.7s of stereo at 48kHz), input cap 32MiB. No float, 24-bit,
MP3, RF64 or large-file streaming. Directory is explicitly selected; browser selection is a rooted relative path. Directory and file symlinks,
traversal and special files are rejected. The offline renderer accepts a basename.

## Onion installation

Follow [INSTALLATION.md](../INSTALLATION.md). Copy your own WAVs into
`App/GrainMM/state/`; subfolders are supported. Y opens the browser and pauses
playback; up/down select, A opens/loads, B goes back, Y closes, X selects the test
tone. Loading and closing start silent. Press A to play again. The chosen source
and controls save on MENU exit. The launcher never writes a sample file.

A missing or unsupported saved sample shows an error and starts silent with tone
access. Its saved selection stays intact until you explicitly choose another
source. Legacy settings use `state/sample.wav` if present. No sample is bundled.

Browser limits:128visible entries per folder,4096scanned entries,7nested folders,
8path components,128bytes/component and512bytes/relative path. Hidden entries and
links are excluded. Use smaller subfolders if a limit is reached. Filenames stay
unchanged; unsupported display characters appear as `?`, long labels are shortened.

Linear interpolation is not band-limited; pitch-up or wrapping nonperiodic
material needs further listening/spectral work. Full GrainBrick projects,
shimmer and waveform display are not implemented.

## Benchmarks and device preparation

`granular_benchmark` is built for host and ARM but is not included in the App ZIP.
It produces no audible output and touches no SDL display/audio device. Use
`build/miyoo-armv7-release/granular_benchmark --seconds 5` on the actual device
(after ABI inspection) for a short mono/stereo, density/size CPU sweep. Each case
renders the specified simulated audio duration offline, not a real-time soak.
Default is 60s per case; deadline at 512/48kHz is 10,666.67us. The console identifies
HOST or ARM build; always record whether execution is native or emulated.

## Stereo delay

Press X/LeftShift to switch GRAIN and DELAY pages. D-pad selects/edits six
controls on the current page. TIME is10–2000ms; FEEDBACK0–0.95; MIX0–1;
PING PONG crosses left/right feedback; SPREAD narrows/widens the wet stereo
image; LOWPASS500–20000Hz filters feedback. Delay starts dry (MIX0).

Time, feedback, mix and routing changes are smoothed. At high feedback, headroom
is limited and final PCM output can saturate. Start with low listening volume
and moderate feedback. Pausing or exiting fades and clears the delay tail.

Version8 settings save in `state/parameters.v8`, including the sample selection.
When absent, v7 tape, v6 synth/compressor, v5 motion, v4 EQ/reverb, v3 source/grain/delay, v2 grain/delay or v1 grain settings load without changing the older
file. A corrupt newer file reports an error instead of substituting older settings.
Preserve `parameters.v8.pending` if a write is interrupted; move it into a backup
before saving again. Pending files are never silently promoted or removed.

## EQ and reverb

X cycles through all eleven pages. EQ gains cover80/250/700/2000/6000/12000Hz,
-20..20dB,1dB steps. Frequencies and Q are fixed at the initial musical defaults.
Positive boosts can clip: lower GRAIN/GAIN first. The final PCM boundary clamps
output; it is not a musical limiter. `clipped_samples` in the session log reports
software saturation since the latest source switch, not hardware clipping/XRUNs.

REVERB offers MIX0..1,ROOM SIZE0..1,DECAY0.3..20seconds,DAMP500..20000Hz,
PREDELAY0..250ms,WIDTH0..1. Default MIX0 is dry. This is an independently authored
parallel-comb/allpass sound; upstream diffuse/mod/motion/shimmer are not exposed.
Grain→EQ→delay→reverb→global fade is the current fixed routing. All tails mute on A.

## Motion

MOTION changes new grains while existing ones finish with their original settings.
REVERSE CHANCE0..1 selects their playback direction; PITCH RAND ST0..12 adds random
semitone variation above/below base pitch. SIZE RANDOM/TIME RANDOM0..1 vary duration
and scheduling. PAN SPREAD0..1 creates stereo movement with bounded linear balance.
SCAN HZ-0.5..0.5 sweeps the sample independently of pitch (0.1 is one lap/10seconds).
Defaults0 leave motion off. Try small amounts first; high timing randomness can
hit the16voice cap. High pitches can alias with the current linear interpolation.

## Built-in oscillator and compressor

Y then X selects the built-in source and focuses OSC. LEVEL0 retains the safe tone;
raise LEVEL to hear the synth through EQ/COMP/DELAY/REVERB. A WAV can layer with it.
OSC controls: LEVEL0..0.25,NOTE24..96,WAVE(sine/triangle/saw/pulse),DETUNE0..30cents,
PULSE WIDTH0.1..0.9 and5Hz VIBRATO0..100nominal cents. Detune spreads two channels;
vibrato depth is approximate. Default note57 is220Hz. Wave names appear on screen.

COMP controls: THRESHOLD-48..0dB,RATIO1..20,ATTACK1..100ms,RELEASE10..2000ms,
MAKEUP0..12dB,MIX0..1. It links stereo peaks to preserve channel balance, before
delay/reverb. MIX0 bypasses it. Large EQ/makeup/feedback can clip the final PCM;
compression does not guarantee peak limiting. Mute still silences the whole rack.


## Tape-style coloration

TAPE processes WAV + oscillator after EQ and before compression/delay/reverb.
DRIVE0..24dB adds soft saturation; TONE500..16000Hz darkens the wet signal.
WOW0..2ms and FLUTTER0..0.25ms vary delay at fixed0.6/6Hz. MIX0..1 blends wet/dry;
OUTPUT−18..6dB controls wet output. Start with MIX0.5, DRIVE6, OUTPUT−6 at low
listening volume. MIX0 is exact bypass. The short wet delay gives partial mixes a
chorus-like phase character; high drive can alias. This is not a tape recorder.
Settings v7 appends these controls; v1–v6 remain preserved on migration.


## Tuned resonance and rhythm

RESONATE filters the WAV/oscillator through four tuned modes. MIDI NOTE36..84 sets
the fundamental, DECAY0.05..3seconds controls ringing, BRIGHTNESS favors upper
modes, and INHARMONIC stretches their tuning. WIDTH narrows stereo and MIX blends
dry/wet. Start around MIX0.4; a saw or pulsed WAV excites it more clearly than a
sine far from its tuning. It is not the original GrainBrick resonator engine.

RHYTHM uses TEMPO30..240BPM and half/quarter/eighth/sixteenth DIVISION. TREMOLO
attenuates rhythmically; AUTO PAN moves stereo balance. SHAPE selects sine or
triangle, STEREO DEG offsets right-channel tremolo. Depth0 bypasses. The clock
is internal and starts/reset with playback, without external sync or MIDI.

## Sample region and grain window

SAMPLE changes the loaded WAV non-destructively. START and END are fractions of
its frames (0.25 = one quarter through); END stays at least0.01above START. Every
region contains a frame, even on a one-frame file. Forward/reverse interpolation
wraps inside the region; original audio is never edited. Sounding grains keep the
bounds with which they started; new grains use edits.

ENV BLEND0 uses Hann;1 uses a linear ATTACK/hold/RELEASE envelope. ATTACK/RELEASE
are fractions0.01..0.49 of a grain and affect only that custom component. WIDTH0
folds grains to mono;1 preserves the original stereo/pan spread. OSC is unaffected.
These18controls are appended in schema8; older versions remain preserved.
