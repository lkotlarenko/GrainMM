# Make sounds with GrainMM

Start with the [quick start](../README.md#your-first-sound). This guide covers the
1.0.0-alpha.1 development controls. The new synthesis/groove controls have separate
validation from the accepted alpha25 device build; see [status](PORTING_PROGRESS.md).

## A few useful words

A **grain** is a small piece of your sample. **Density** is how often grains start.
A **slice** is a larger region played by a sequencer. **Dry** means the unchanged
signal; **wet** means processed. **MIX 0%** bypasses a wet/dry effect. A **semitone**
is one piano key; **12 semitones** make an octave. A **cent** is 1/100 of a semitone.
A **MIDI NOTE** number is a pitch label (60 = C4), not a MIDI connection.

## Pick a source

Load a WAV with Y, or choose Y then X for built-in synthesis. A always controls
transport on the main pages. Starting or reopening is paused. Enabling a source
lets it layer with the others; choose zero levels/off to isolate one sound.
The gentle 220 Hz fallback plays only with no WAV and no enabled oscillator,
sub, collision or drone source. It bypasses FX but passes through master OUTPUT.

X opens SOURCE, FX, TEMPO and TOOLS. L/R selects a group, up/down a module,
A opens, B/X returns. SOURCE shows at most 12 cards; continue down for the second
bank containing OSC B and TIMBRE. Main-view L/R visits all sound pages in group
order, skipping actions such as export. SELECT opens page help, with X to reset
just the selected control. Hold left/right for progressively faster edits;
choices and up/down selection do not accelerate.

## Samples, grains and motion

Use your own **16-bit PCM WAV**, 8–96 kHz, mono/stereo, decoded size at most 16 MiB.
Keep WAVs under `App/GrainMM/state/WAV/`; nested folders work. Browser A opens or
loads, B returns, X chooses built-in. A failed load keeps the previous sound.
Files remain unchanged. Linear interpolation can alias at high pitch shifts.

| SOURCE page | Controls / purpose |
| --- | --- |
| GRAIN | SIZE MS, DENSITY HZ, PITCH, POSITION, SPREAD, GAIN: grain duration, frequency, playback speed, location, random location spread and level |
| MOTION | Reverse chance, pitch/size/timing randomness, pan spread and continuous scan; adds variety to new grains |
| SAMPLE | START/END trim, ATTACK, RELEASE, ENV BLEND, STEREO WIDTH; B normally opens a larger trim view |
| HARMONY | TRANSPOSE ST, CHORD, ORDER, OCTAVES, CHANCE, DETUNE CT; tunes individual grains into chord-note patterns |
| EVOLVE | MODE OFF/WAVE/DRIFT/BLEND, RATE HZ, POSITION DEPTH, PITCH CENTS, SPREAD DEPTH, SCAN HOLD; adds slow motion to grains |

There are at most 16 simultaneous grains. Trim bounds and grain tuning/envelopes
latch when a grain starts; existing grains finish naturally. ENV BLEND mixes the
regular grain window with your attack/release shape. HARMONY needs a tonal WAV
tuned by ear; it does not detect the sample's key. Chord choices are OFF, OCTAVES,
FIFTH, MAJOR, MINOR and SUS4; ordering is UP, DOWN, BOUNCE or repeatable RANDOM.
CHANCE controls how often the chord pattern advances. Transpose and detune still
work with CHORD OFF. Playback restart resets the phrase.

## Sample evolution

EVOLVE works on grains when SLICE/MODE is OFF; it does not modulate sliced audio,
oscillators or balls. Its saved base grain controls stay editable. RATE 0.01–0.50 Hz
means a 100–2 second cycle. HOLD freezes evolution and scan, while random spread
can continue. MODE OFF eases offsets back to zero. This is not live freeze.

## Dual oscillator synthesis

**OSC** and **OSC B** are independent stereo oscillators. Both provide LEVEL,
MIDI NOTE 24–96, WAVE (SINE/TRIANGLE/SAW/PULSE), DETUNE CT 0–30, PULSE WIDTH
0.10–0.90 and VIBRATO CT 0–100. Detune separates stereo pitches; vibrato moves
pitch slowly. Maximum level is 0.25 (25%). OSC uses its original decimal level
display; OSC B displays a percentage. Lower levels before combining sources.

**Try a fifth:** built-in source, OSC level 0.08 and note 57, OSC B level 8% and
note 64. Try SINE plus TRIANGLE, then add a little REVERB. For bass and octave
layers, tune OSC B 12 semitones above or below OSC.

## Wavetable color and sub bass

TIMBRE shapes **OSC A (the original OSC page)**, leaving OSC B independent.
These tables are generated mathematically; no waveform assets are loaded.

| TIMBRE control | What it does |
| --- | --- |
| TABLE MIX % | 0 keeps OSC's selected wave; 100 replaces it with the wavetable |
| TABLE POSITION | 0 PURE → 1 HOLLOW → 2 BRIGHT → 3 GLASS; values between blend smoothly |
| SCAN HZ | Automatic table-position movement, 0–2 cycles per second; 0 holds the current scan phase |
| SCAN DEPTH % | Movement around the saved position, up to three positions either way, limited to 0–3 |
| SUB LEVEL % | Adds a mono sine at 0–25% level, independently of OSC's level |
| SUB OCTAVES | Tunes the sub one or two octaves below OSC's note |

**Try a moving pad:** OSC SINE, level 0.10; TIMBRE mix 100%, position 1.5,
scan 0.10 Hz, depth 30%; REVERB mix 25%. Add OSC B quietly for a second pitch.
Try SUB LEVEL 3%, one octave down, for weight. The sub alone works with OSC level
zero. Raise OSC's level to hear tables; table mix alone does not enable sound.

Tables use up to eight harmonics and remove upper partials as pitch rises.
Very high notes therefore sound simpler. This is a compact color source, not a
user-wavetable importer or a copy of GrainBrick's wavetable engine. Sub pitch
follows OSC A's stereo-average tuning/vibrato. Pitch/octave changes slew.

## Slices and the pattern editor

Load a WAV, open SLICE and choose MODE FORWARD to start. OFF returns to grains.
REGIONS chooses 2–16 cuts; DIVISION chooses quarter/eighth/16th/32nd notes. GATE
sets slice duration. Generated modes include FORWARD, REVERSE, PING-PONG and
SHUFFLE; VARIATION adds sparse steps, alternating reverse or chance. Global
REVERSE CHANCE combines with per-step reverse. Gain, pitch and trim apply;
grain size, density, harmony and grain-only motion do not.

Open **X → SOURCE → PATTERN → A**, or B on SLICE:

| Button | In PATTERN |
| --- | --- |
| L/R | Select one of 16 stored steps |
| ↑/↓ | Select REGION, PLAY STEP, REVERSE, CHANCE % or CUT END % |
| ←/→ | Edit the field |
| X | Toggle a rest |
| Y | Switch MANUAL/GENERATED, keeping your edits |
| A | Play/pause |
| B | Return |

Editing selects MANUAL and enables slicing if it was off. The playback underline
is separate from the edit selection. In MANUAL, generated mode/order and VARIATION
do not replace stored steps; GROOVE's traversal still applies. CUT END trades
space with the neighboring region, affecting all steps using either region. The
final cut stays fixed; trim changes the overall end. REGIONS redistributes saved
relative widths. Very short regions can become silent after edge ramps.

## Swing, phrases and accents

Open **X → TEMPO → GROOVE → A**. These six controls affect WAV slices, including
manual and generated patterns. They do not swing drones, balls or rhythm FX.
TEMPO's RHYTHM page sets the shared BPM (30–240).

| GROOVE control | Range / behavior |
| --- | --- |
| SWING % | 0–65%; delays every other slice, keeping each pair's total duration unchanged |
| STEPS | Play the first 1–16 stored pattern steps; shorter phrases do not erase the others |
| ROTATE | Move the starting position within the active phrase, 0–15 modulo its length |
| STEP ORDER | FORWARD, REVERSE or BOUNCE; bounce does not repeat end steps |
| ACCENT % | Soften non-accented steps, from no change to silence; accented steps keep their normal level |
| ACCENT EVERY | Accent every 1–16 transport steps, starting with the first |

**Try a lopsided loop:** STEPS 7, SWING 30%, ROTATE 2, BOUNCE, ACCENT 35%,
ACCENT EVERY 3. Stored pattern boxes remain editable, including steps outside the
active phrase. Stop/play restarts traversal and accent count; settings edits take
effect at the next audio update, so change timing/order between phrases for an
intentional transition. Slice gates follow long/short swing intervals.

## Drones and collisions

DRONE generates its own sound without a WAV. Raise LEVEL to about 60%; choose
ROOT NOTE 36–84, one of eight MOODs, EVOLVE SEC 4–60, TEXTURE and MOVEMENT.
GENERATE adds EVENT MIX, EVENT RATE 0.05–2 Hz, GLIDE SEC 0.05–3, STEREO WIDTH,
SEED 1–9999 and PATTERN (DRIFT/FORWARD/REVERSE/BOUNCE/HOLD). HOLD keeps the
current chord while movement/events continue. Four sustained voices and at most
four event voices share fixed storage. Pause freezes harmonic time.

B on DRONE/GENERATE opens eight scenes: DAYBREAK, TIDELINES, GLASSHOUSE,
NIGHTGLOW, OPEN SKY, EMBER, SLOW ORBIT, STILL WATER. Up/down chooses, left/right
changes seed, A applies, B/X cancels. Applying keeps level, effects, other sources,
selected WAV and transport. A zero-level drone remains off. Scenes are editable
starting points, not extra saved slots. Activity graphics are not an FFT.

BALLS/MODE ON enables tuned wall impacts. BALLS controls count 1–6, gravity,
rotation, bounce and level. BOWL controls note, scale, octave, decay, brightness
and excitation. Pause freezes physics and fades the sound. There is one original
sound family; ball-to-ball collisions and upstream resonator models are absent.

## Effects and output

All musical sources share the same serial chain. FX has nine module cards;
RESONATE is one processor, and TEMPO/RHYTHM is another stage outside the FX menu.

| Module | Main use |
| --- | --- |
| FILTER | Low/high/bandpass, cutoff, resonance, drive, wet output and mix |
| RESONATE | Tuned ringing: note, decay, brightness, inharmonicity, width and mix |
| EQ | Six bands; large boosts can clip |
| TAPE | Drive/tone, wow/flutter, wet output and mix; color, not recording |
| CRUSH | Bit depth, held frames, drive/tone, wet output and mix |
| CHORUS | Modulated short delays, feedback, width and mix |
| COMP | Threshold, ratio, attack/release, makeup and mix; not a safety limiter |
| RHYTHM (TEMPO) | Shared BPM, division, tremolo, auto pan, shape and stereo phase |
| DELAY | Time, feedback, mix, stereo spread/ping-pong and tone |
| REVERB | Space, damping, decay and mix |

The processing order is Filter → Resonator → EQ → Tape → Crusher → Chorus →
Compressor → Rhythm → Delay → Reverb → OUTPUT. Dry defaults keep effects quiet
until enabled. MIX uses percentages; 30% is the old decimal 0.30.

**TOOLS → MASTER OUTPUT** has GAIN −24 to +18 dB, LIMITER, CEILING −12 to 0 dBFS,
RELEASE 20–1000 ms, BALANCE and MONO. Start with limiter ON, then try gain +6 dB
at low device volume. Frequent gain reduction means lower source/effect boosts.
This linked stereo sample-peak limiter has no lookahead or true-peak guarantee.
The speaker cannot demonstrate stereo; use headphones to hear width/pan.

## Patches, snapshots and reset

START pauses audio and opens 16 named slots. Up/down chooses, X saves, A loads,
Y edits the next save's name. Replacing a slot needs another X press. B/START
closes. Patches save controls, sample paths and full patterns, not WAV copies or
live voice/effect history. Back up `state`, including samples and `patches`.

For a temporary snapshot on the main controls, hold SELECT before tapping A to
capture or B to recall, then release. Transport stays as it was. Source loading
invalidates the slot. It is RAM only and disappears on exit.

## Reset all effects for comparison

Hold START+SELECT for about a second and release both. Up/down chooses a scope;
X confirms, B/SELECT cancels. **Effects only** clears the rack/tails, keeping
sources, patterns and groove. **Effects + modulation** also resets motion,
harmony, oscillator detune/vibrato, table scan and groove; keeps base oscillator
levels/notes, table color/sub level, sample and trim. **All sound controls**
restores defaults and pauses. Samples and saved patch files always remain.

## Export and resample

TOOLS → EXPORT WAV pauses playback. Choose 5/15/30/60 seconds, A renders,
B cancels/returns, MENU exits. New numbered 48 kHz stereo PCM16 files go into
`state/exports`; existing files are never replaced. Sixty seconds needs about
11 MiB plus 1 MiB reserve. Export runs a fresh complete instrument graph from
your current settings, including new synthesis/groove. It does not capture live
button presses, current phases or existing tails. A short fade ends the file;
tails do not extend the selected duration. Playback remains paused.

For clips, lower gains or enable the limiter, then export again. Power/SD errors
can leave incomplete files. Load 5/15/30-second exports via Y → exports to resample;
60-second stereo exports exceed the 16 MiB decoded loading cap.

## Saving, colors and recovery

Normal MENU exit saves changed settings/source to `state/parameters.v18`.
The 150 scalar controls and full pattern are included in settings/patches/snapshots.
Schemas 1–17 load when no newer file exists; old files remain for rollback.
A corrupt newer file reports an error rather than silently loading an older one.
New patches require a schema18-capable build. Old versions cannot read new patches;
keep a full backup before updating. Power interruption does not save unsaved edits.

TOOLS → COLORS previews six palettes. A saves, B/X cancels. Appearance lives in
`state/theme.v1`, separately from sound patches. Pending save transactions are
preserved on failure; back up first, then follow [recovery](TROUBLESHOOTING.md).
