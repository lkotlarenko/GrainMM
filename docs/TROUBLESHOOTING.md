# Troubleshooting

## GrainMM does not appear in Apps

Check that you downloaded the **GrainMM-…-miyoo-mini-plus.zip** installation,
not a Source code archive. On the SD card, `config.json` must be directly inside
`App/GrainMM`. Extra enclosing folders prevent Onion from finding the app.
GrainMM targets Miyoo Mini Plus with OnionOS; other models are untested.

## It closes immediately

Keep `App/GrainMM/state/session.log` and `session.previous.log`. They record the
startup error. Include the relevant error in a bug report after removing personal
filenames. Try a fresh installation using the [backup/update steps](INSTALLATION.md).
Do not replace Onion libraries or change its audio configuration.

## There is no sound

Playback starts paused. Press **A** and check the Miyoo's listening volume.
In the browser, **A** loads a file; press **A** again after loading to play.
Closing the browser also returns paused. Without a sample, A plays a quiet test
tone. Sparse grains are quiet; raise gain gradually at a low listening volume.
Delay is initially dry: raise its MIX control to hear echoes.

## The oscillator or compressor seems silent

The built-in source at OSC/LEVEL0 is the quiet test tone and bypasses effects.
Raise OSC/LEVEL to0.05 for musical synthesis, then press A. Loading a WAV or
closing the browser always returns paused. GRAIN/MOTION require a WAV; other pages
can process the musical oscillator. Compression can reduce volume substantially;
start with COMP/MIX0 and modest EQ/gain settings.

## My sample is missing or will not load

Place your own short **16-bit PCM WAV** in `App/GrainMM/state/` or a subfolder.
Use **Y** to browse, **A** to open/load and **B** to go back. MP3, 24-bit and float
WAV are unsupported. Use mono/stereo PCM16 at8–96kHz; decoded audio is limited to16MiB.
Hidden files, links and non-WAV files are not listed. Split a large sample folder
into smaller subfolders if GrainMM reports a listing limit.

A missing saved sample leaves the app paused with an error. Its stored selection
is preserved until you choose another source. Restore the WAV to its original
location, or use Y to choose a different sound. X inside the browser selects the
test tone without deleting any files.

## Settings cannot be saved or restored

Use **MENU** to save and exit; power interruption can lose unsaved edits.
If a saved file is malformed, GrainMM reports an error and preserves it. With the
app closed and the SD card on your computer, back up the entire `state` folder.
Move `parameters.v7` into that backup to use older v6/v5/v4/v3/v2/v1 settings. Move all seven
versioned files into backup to start with defaults. Your WAVs are independent.

An interrupted save may leave `parameters.v7.pending` (or an older version's
pending file). The previous committed settings remain usable. Move the pending
file into backup before saving again. GrainMM never promotes or deletes it
automatically. Check available SD space if saves keep failing.

