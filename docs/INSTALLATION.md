# Install, update and back up

GrainMM targets **Miyoo Mini Plus with OnionOS**. The accepted test device runs
OnionOS 4.3.1-1. Other models/versions are untested. The1.0milestone is an
**experimental prerelease**, not a stable release. Read the release notes and
back up your app/state before installing.

## First installation

1. Download **GrainMM-…-miyoo-mini-plus.zip** from [Releases](https://github.com/lkotlarenko/GrainMM/releases), under Assets. The source archives are for developers, not installation.
2. Turn off the Miyoo and put its SD card into your computer.
3. If `App/GrainMM` already exists, follow the update instructions below first.
4. Extract the ZIP onto the card. Check for **App/GrainMM/config.json**, with `launch.sh`, `icon.png` and `grainmm` beside it. Avoid an extra enclosing folder or a second App folder.
5. Safely eject the card, put it back and open **Apps → GrainMM**.

No terminal commands, firmware changes or system-library replacements are needed.

## First sound and samples

Start at low listening volume. GrainMM opens paused. A plays/pauses the gentle
test tone; MENU saves and exits. For sample playback, create
`App/GrainMM/state/WAV/` on your computer, copy in your own short **16-bit PCM WAV**,
then use Y → WAV → file → A. Press A again after loading to play.

Mono/stereo, 8–96 kHz are supported, with a 16 MiB decoded limit. At 48 kHz this
is about 43 seconds stereo or 87 seconds mono; use shorter files first. MP3,
24-bit and float WAV are unsupported. Samples are never edited. With no samples,
Y then X chooses built-in synthesis. Follow the [quick start](../README.md#your-first-sound)
or [sound guide](PARAMETERS.md) for drones and oscillators.

## Update without losing your sounds

1. Press MENU to exit. Turn off the Miyoo and open its SD card on your computer.
2. Back up the **whole App/GrainMM folder** to your computer.
3. Move the old app folder off its location on the card. Extract the new app ZIP to create a fresh `App/GrainMM`.
4. Copy **only the old state folder** into the fresh app folder. Do not copy old executables, launchers, CURRENT/PREVIOUS selectors or releases folders over the new installation.
5. Keep the backup until the new app loads your sounds and saves correctly.

New settings are written to **parameters.v18**. Older v1–v17 files are retained.
This protects the older settings file but does not version every owner file:
replacing a named patch in a new build makes that patch unreadable by an older
build. A full state backup is essential for a complete rollback.

To undo an update, exit and power off, then restore the **complete backed-up app
folder**, including its state. Do not mix new patch files into an older version.
The same fresh-folder procedure applies to SSH-managed development installations.

## Where everything lives

| Inside App/GrainMM | Contents |
| --- | --- |
| state/WAV/ | Suggested home for your samples; other folders beneath state also work |
| state/exports/ | New numbered exported WAVs |
| state/parameters.v18 | Current saved controls, sample selection and pattern |
| state/parameters.v1 … v17 | Older settings, preserved for compatibility/rollback |
| state/patches/ | 16 named patch slots; samples are referenced, not copied |
| state/theme.v1 | Saved color preference |
| state/session.log, state/session.previous.log | Last two launcher reports |

GrainMM creates its state folders as needed. Back up the whole state folder to
retain samples, exports, settings, patches and colors together. MENU saves edited
ordinary settings after audio stops. Sudden power loss does not save unsaved work.
A missing/invalid sample remains selected until you explicitly choose another.

## Remove GrainMM

Exit, power off and back up state first. Remove only `App/GrainMM` with your
computer's file manager. This also removes its samples/exports if you did not copy
them out. Other apps do not need changing. For launch/save errors, see
[troubleshooting](TROUBLESHOOTING.md).
