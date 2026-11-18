# Install GrainMM on a Miyoo Mini Plus

GrainMM is an early native sound instrument for **Miyoo Mini Plus with OnionOS**.
The verified test device runs OnionOS 4.3.1-1. Other models/versions are untested.
No firmware update, system-library replacement or terminal commands are needed.

## First installation

1. Open [Releases](https://github.com/lkotlarenko/GrainMM/releases). Download
   **GrainMM-…-miyoo-mini-plus.zip** from **Assets**. Do not download the Source
   code archives. If there is no release yet, the first one is still being prepared.
2. Turn off the Miyoo safely and put its SD card into your computer.
3. If `App/GrainMM` already exists, follow the update steps below first.
4. Extract the ZIP onto the SD card. Check that the path is exactly
   `App/GrainMM/config.json`, alongside `launch.sh`, `icon.png` and `grainmm`.
   Avoid an extra enclosing `GrainMM-version` or second `App` folder.
5. Safely eject the card and put it back. Turn on the Miyoo and open **Apps → GrainMM**.

## First sound

GrainMM starts silent. Set listening volume low, then press **A** to play the
quiet test tone. Press **A** again to pause; **MENU** exits back to Onion.

For granular audio, put a short **16-bit PCM WAV** you own at
`App/GrainMM/state/` on the SD card. Press **Y** to browse, **↑/↓** to choose,
**A** to open/load and **B** to go back. Subfolders are supported. Press **A**
again after loading to play. The selected sound restores next time. Older
settings still use `state/sample.wav` as a default. Create the
`state` folder if this is your first launch. Mono/stereo, 8–96kHz are accepted;
use short samples (decoded limit 16MiB, roughly 44 seconds stereo at 48kHz).
MP3, float WAV and 24-bit WAV are unsupported. The original sample is never edited.

For no-WAV synthesis, press Y then X to choose the built-in source. OSC opens;
raise LEVEL to0.05 and press A. NOTE chooses pitch, WAVE chooses the sound shape.
The quiet test tone stays available at LEVEL0. L/R move between pages.

**X** cycles GRAIN/DELAY/EQ/REVERB/MOTION/OSC/COMP/TAPE controls. **↑/↓** select a parameter; **←/→** change it. Values save when you exit normally
with MENU and restore next launch. Playback always starts silent. GrainMM does
not load original GrainBrick projects or save recordings yet.

## Update without losing your sounds

1. Exit GrainMM, turn off the Miyoo and open its SD card on your computer.
2. Back up the entire `App/GrainMM` folder somewhere safe on the computer.
3. Move the old GrainMM folder off its original location on the card. Extract
   the new ZIP onto the card to create a fresh `App/GrainMM`.
4. Copy **only the old `state` folder** into the new GrainMM folder. This keeps
   your sample and saved parameters. Do not copy old executables, launchers,
   CURRENT/PREVIOUS selectors or `releases` folders over the fresh installation.
5. Keep the backup until the new build works. To undo the update, restore the
   complete backed-up folder while the device is off.

## Where your files live

Everything stays in `App/GrainMM`. Your WAVs/folders in `state` are input sounds;
`state/parameters.v7` holds controls and the sound selection. Older
`state/parameters.v1`, `.v2`, `.v3`, `.v4`, `.v5` and `.v6` are read when newer versions are absent and stay
preserved for rollback. `state/session.log` and
`state/session.previous.log` hold the last two startup/exit reports. Parameter
writes happen only after edits and normal exit, after audio has stopped.

## If it does not open

Check the folder nesting, correct ZIP and target model first. See
[Troubleshooting](TROUBLESHOOTING.md) and keep the two session logs before retrying.
Do not change Onion system libraries or audio settings to work around an error.

To remove GrainMM, exit and turn off the device, back up `state`, then remove
only the `App/GrainMM` folder using your computer's file manager.
