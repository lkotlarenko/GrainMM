# Troubleshooting

## GrainMM does not appear or closes immediately

Use the installation ZIP, not a source archive. Check for
`App/GrainMM/config.json` without an extra folder. Target: Miyoo Mini Plus with
OnionOS; other models are untested. Use the prerelease installation ZIP linked from the [user guide](../README.md).

If it closes, keep `state/session.log` and `state/session.previous.log` before
retrying. They record the startup error. Follow the [fresh installation/update
procedure](INSTALLATION.md#update-without-losing-your-sounds). Do not replace Onion
libraries or change audio configuration to work around an error.

## No sound, or an effect does nothing

Press A: launch, loading and reopening start paused. Check device listening volume.
Browser A loads; press A again afterward to play. In a patch dialog, A loads a
patch instead of playing. Return to the main controls.

With no WAV and all built-in sources disabled, the quiet fallback tone bypasses
FX. Raise OSC/LEVEL to 0.05 or enable another musical source to hear effects.
Most effects start dry: raise MIX gradually. COMP can reduce volume; begin with
mix zero and modest gains. RHYTHM needs TREMOLO or AUTO PAN above zero.

TIMBRE/TABLE MIX changes OSC A's color but needs OSC/LEVEL above zero. OSC B is
independent; TIMBRE does not color it. SUB LEVEL can play on its own. SCAN DEPTH
needs a nonzero SCAN HZ for movement. GROOVE needs an enabled WAV slice source;
it does not swing drones, balls, ordinary grains or rhythm FX.

The built-in speaker cannot reveal stereo panning. For distortion, lower source
levels, EQ/makeup boosts or OUTPUT gain. Enable the optional OUTPUT limiter and
listen at low volume. It is a sample-peak limiter, not a true-peak guarantee.

## A WAV is missing or will not load

Use your own short **16-bit PCM WAV**, mono/stereo, 8–96 kHz, beneath `state`.
The decoded limit is 16 MiB: about 43 seconds stereo at 48 kHz. MP3, 24-bit/float
WAV, hidden files and links are unsupported. Large folders can hit listing limits;
split them into smaller subfolders. Loading failure keeps the prior sound.

A missing saved sample remains selected. Restore its original relative location,
choose another with Y, or choose built-in with Y then X. Patches store paths, not
copies. A 60-second stereo export cannot be loaded back; choose 5/15/30 seconds.

## Settings or patches cannot save

MENU saves ordinary settings on normal exit. Power interruption can lose unsaved
edits. Back up the **whole state folder** with the app closed before recovery.

Current settings use `parameters.v18`. If it is malformed, move that exact file
into your backup to allow the newest remaining v1–v17 file to load. To start from
defaults, move all versioned parameter files aside. Preserve your WAVs/patches.
A corrupt newer file is never silently ignored in favor of an older one.

An interrupted save can leave `parameters.v18.pending` (or an older pending
transaction). The previous committed settings remain. Move the pending file to
your backup before retrying; it is never automatically promoted or deleted.
Check SD free space and filesystem health if failures continue.

START opens patch slots. X saves; replacing a used slot requires a second separate
X press. Y edits a draft name; it does not rename until saved. A loads, B/START
closes. Failed patch saves preserve old committed files and may leave
`state/patches/patch-NN.gmm.pending`; back up and move that exact pending file
aside. A schema18 patch needs the new build. Restore a full old state backup to
use older builds. Color recovery follows the same rule for `theme.v1.pending`.

## Patterns or harmony sound unexpected

PATTERN's Y switches MANUAL/GENERATED; edits select MANUAL automatically. Manual
steps keep their region/reverse/chance settings. GROOVE then changes traversal:
check STEPS, ROTATE, STEP ORDER and ACCENT if some stored steps are not heard.
ACCENT 100% silences the non-accented steps. GATE/trim/pitch can make very short
slices silent. Stop/play restarts phrase position and accents.

HARMONY needs grains from a WAV, with SLICE off. It does not detect key or retune
oscillators. Tune tonal samples by ear. CHANCE zero keeps the transposed root.
Extreme pitch changes can alias with current linear sample interpolation.

The waveform is cached input peaks, not processed output. It will not follow FX.
Use trim controls to change the bright region; the WAV remains intact.

## Controls, snapshots and colors

X opens modules; it does not cycle pages. SOURCE has two banks: keep moving down
for OSC B and TIMBRE. L/R changes groups in modules, pages in the main view, and
steps in PATTERN. Short START/SELECT actions happen on release.

For reset, hold START+SELECT about a second, release both, choose a scope and
press X; B cancels. Close file dialogs first. All sound controls resets and pauses.
Effects-only keeps sources/motion/groove; effects + modulation resets those motion
controls while preserving base synthesis. MENU always exits.

For a snapshot, hold SELECT first, then tap A or B, and release. Source loading
invalidates the snapshot; exit loses it. Colors are saved separately: TOOLS →
COLORS, A saves, B/X cancels. App icon appearance may need leaving/re-entering Apps.

## Export and connection questions

Export is a fresh render, not live recording. It pauses playback and includes the
complete source/effects/master settings. Existing tails and button gestures are
not captured. B cancels unfinished work; completed files remain in `state/exports`.
For clipping, lower gain or enable limiting and export again. Copy important files
before moving aside any incomplete export after an SD/power error.

Stock Miyoo Mini Plus has no Bluetooth. GrainMM has no MIDI, USB audio/MIDI,
network music sync or live audio input. Wi-Fi file transfer is separate from
musical connectivity. External hardware modifications are outside tested scope.

## Report a problem

Use [Issues](https://github.com/lkotlarenko/GrainMM/issues). Include the GrainMM
version, handheld model, OnionOS version, steps to reproduce, whether you used a
WAV or built-in source, and the relevant error from the two session logs. Remove
personal filenames, addresses and credentials first. Mention whether it happened
on the speaker or headphones. Do not include copyrighted samples without rights.

For engineering diagnostics: [build](BUILDING.md), [testing](TESTING.md),
[hardware](development/MIYOO_HARDWARE.md). Callback lateness is a scheduling hint;
the current SDL adapter exposes no hardware XRUN counter. Keep Onion's normal
audio preload enabled. Sanitizer leak checks need execution outside ptrace.
