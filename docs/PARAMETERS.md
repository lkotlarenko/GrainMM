# Sound and patch guide

GrainMM is an independently written native instrument inspired by GrainBrick.
It starts silent; A plays/pauses and MENU saves settings/exits. Raise listening
volume cautiously. Original WAVs are never edited. See the [user guide](../README.md)
for installation and [troubleshooting](TROUBLESHOOTING.md) for errors.

## Sources and files

Y opens the sample browser and pauses audio. A opens a folder or loads a WAV;
B goes back, Y closes, and X selects the built-in source. Loading starts paused.
Use short16-bit PCM RIFF WAVs, mono/stereo8–96kHz. Decoded audio is limited to16MiB
(about43.7seconds of stereo48kHz), input files to32MiB. Float/24-bit WAV, MP3 and
RF64 are unsupported. Symlinks, traversal and special files are rejected.

The browser accepts128visible entries per folder, at most4096scanned entries,
7nested folders,128bytes per component and512bytes per relative path. Hidden
entries are excluded. User filenames are preserved; unsupported display characters
appear as question marks. Keep large libraries in smaller folders.

Without a WAV, OSC/LEVEL0 is the quiet test tone. Raise LEVEL for musical synthesis
through effects. GRAIN/MOTION/SAMPLE require a WAV. The oscillator can layer with it.

## Control pages

L/R move between pages; X also moves forward. Up/down selects a control, left/right
changes it. SELECT shows help; B or SELECT closes help. A can still pause/play
while help is open outside a browser/dialog. MENU always exits.

| Page | Controls and behavior |
| --- | --- |
| GRAIN | Size5–250ms, density1–80Hz, pitch0.25–2×, position/spread0–1, gain0–0.5 |
| DELAY | Time, feedback, wet/dry mix, ping-pong, spread and lowpass |
| EQ | Six gain bands at80/250/700/2000/6000/12000Hz; flat by default |
| REVERB | Mix, size, decay, damping, predelay and width |
| MOTION | Reverse probability, pitch/size/timing variation, pan spread and scan |
| OSC | Level0–0.25, MIDI note24–96, sine/triangle/saw/pulse, detune, duty and vibrato |
| COMP | Threshold, ratio, attack/release, makeup and wet/dry mix; not a brick-wall limiter |
| TAPE | Drive, tone, wow/flutter, mix and wet output; coloration, not tape recording |
| RESONATE | Note36–84, decay0.05–3s, brightness, inharmonicity, width and mix |
| RHYTHM | Tempo30–240BPM, half/quarter/eighth/sixteenth division, tremolo, pan, sine/triangle and stereo phase |
| SAMPLE | Non-destructive start/end region, attack/release window blend and stereo width |

Grains use a fixed16-voice pool. Excess grains are skipped without stealing sounding
voices; skips are not audio underruns. Pitch conversion uses linear interpolation,
which is not band-limited. High drive/pitch or abrupt source loops can alias.
EQ/makeup/feedback can clip; lower source gain before strong boosts.

SAMPLE start/end are fractions of the file; end stays above start. Region/window
edits apply to newly started grains. ENV BLEND0 keeps Hann;1 uses ATTACK/hold/
RELEASE. Attack/release are fractions0.01–0.49 of grain duration and affect that
custom component. WIDTH0 folds grains to mono;1 preserves stereo. OSC is unaffected.

## Waveform trimming

With a WAV loaded, press **B** outside the browser to open the waveform. The bright
region is selected; gray audio lies outside it. Up/down selects a SAMPLE control,
left/right changes it, A plays/pauses, and B returns. L/R/X leave for another page.

The waveform shows cached source peaks, not the processed effects output. The
cache is built once on load, outside audio processing; trim changes do not rescan
the sample. A one-frame or silent file may look like a flat line.

## Named patches

Press **START** to open16slots. This pauses audio. Up/down chooses a slot.

- **X** saves the current sound/settings/source. For an occupied slot, release X
  and press it again to confirm replacement; B cancels that confirmation.
- **A** loads a saved patch and returns paused. Missing/invalid WAVs leave the
  previous instrument intact with an error; nothing is silently overwritten.
- **Y** edits the name for the next save. Left/right chooses a character,
  up/down changes it, A accepts the draft name and B cancels. Then X saves.
- **B** or **START** closes the dialog. **MENU** saves ordinary settings/exits;
  it does not commit an unsaved patch-name draft.

Names use up to20uppercase letters/numbers/spaces/hyphens/underscores. Patch files
are `state/patches/patch-01.gmm` through `patch-16.gmm`. Each contains its name,
all66controls and relative source selection. WAV audio is referenced, not copied:
keep the WAV at the same relative path when sharing or moving a patch.

Patch format1 wraps the strict settings8 format. Atomic saves refuse a leftover
`.pending` file and preserve old committed data on failure. Back up the whole
`state` folder, including patches. There is no delete action.

## Settings and recovery

Ordinary settings remain `state/parameters.v8`; older v1–v7 files are read only
when newer versions are absent, and are preserved. A malformed newer file reports
an error rather than hiding it with an older one. Normal MENU exit saves edits;
power/signal interruption does not auto-save unsaved changes.

Back up state before moving a failed `.pending` transaction aside. It is never
silently promoted/deleted. Missing sample selection remains saved until an explicit
source change. See the [update guide](INSTALLATION.md#update-without-losing-your-sounds).

Recording, MIDI, named GrainBrick project compatibility, shimmer and multiple grain
layers are not provided yet. This is still an alpha, not full feature parity.
