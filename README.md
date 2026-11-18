# GrainMM

A granular synthesizer for **Miyoo Mini Plus running OnionOS**. Load a WAV, reshape it with granular playback, layer a built-in synthesizer, and add EQ, compression, tape-style coloration, delay and reverb.
Control everything with the D-pad and buttons.

**Created and maintained by [elly (@lkotlarenko)](https://github.com/lkotlarenko).**
An independently written native adaptation of
[GrainBrick](https://github.com/ETRNLIAM/GrainBrick), by **ETRNLIAM**.

![GrainMM grain controls](docs/images/grain-controls.png)

*Grain controls from an earlier build, captured with the actual host renderer.
The example filename is illustrative; samples are not included.*

## Status

**Early alpha. No public download yet.**
This repository currently publishes documentation. The native source is being
kept private while the first version is developed and tested. Granular
playback, delay, controls and saved settings work on a real Miyoo Mini Plus.
Recent builds add EQ, reverb, grain motion, a playable oscillator and compression,
including synthesis without a sample. These have been checked on the Miyoo.
The newest tape-style effect has native automated checks; listening acceptance
is tracked separately.
Sample browsing and the six-control interface also passed testing on the Miyoo.
This version does not yet include all GrainBrick features. Recording, MIDI,
shimmer, resonators and other features are still to come.

Tested system: **Miyoo Mini Plus · OnionOS 4.3.1-1**. Other models are untested.

## Why a native version?

GrainBrick's supplied DSP libraries target a different,64-bit ARM device. The
Miyoo runs32-bit ARM with128MiB RAM, so copying the original app could not make
it work. We built a new C++ audio engine, button-driven interface and OnionOS
adapters for this hardware. GrainMM aims for good sound and useful feature
coverage; it does not promise identical DSP or compatible GrainBrick projects.

Read [the porting story and differences](docs/ABOUT_GRAINMM.md) for what we rebuilt,
the hardware challenges and where the project is going.

## Install

1. Open [Releases](https://github.com/lkotlarenko/GrainMM/releases) and download
   **GrainMM-…-miyoo-mini-plus.zip** under **Assets**. Choose the app ZIP,
   not **Source code**. If there are no releases, a download is not available yet.
2. Turn off your Miyoo and connect its SD card to your computer.
3. Extract the ZIP onto the SD card. Check that `config.json` is at
   **`App/GrainMM/config.json`**, without an extra folder around `App`.
4. Safely eject the card, put it back, and open **Apps → GrainMM**.

Already installed? Follow the [update guide](docs/INSTALLATION.md#update-without-losing-your-sounds)
to keep your sounds and settings.

## Play

GrainMM starts silent. Keep the listening volume low and press **A** to hear
the built-in test tone. Press **A** again to pause.

To use your own sound:

1. Copy a short **16-bit PCM WAV** into **`App/GrainMM/state/`** on the SD card.
   Create `state` if it does not exist. You can organize sounds into subfolders.
2. Open GrainMM and press **Y**. Choose a file with **↑/↓**, then press **A**
   to load it. Use **A** to open a folder and **B** to go back.
3. Press **A** again to play. Change the controls with the D-pad.
4. Press **MENU** to save your settings and exit. The selected sound returns
   next time, paused.

![GrainMM sample browser](docs/images/sample-browser.png)

*Sample browser captured with the same host renderer; example filenames only.*

## Controls

| Button | Action |
| --- | --- |
| **A** | Play or pause |
| **↑ / ↓** | Select a parameter |
| **← / →** | Change the selected value |
| **X** | Next control page |
| **L / R** | Previous / next control page |
| **Y** | Open the sample browser; playback pauses |
| **MENU** | Save and exit to Onion |

In the browser, **A** opens a folder or loads a sound, **B** goes back,
**Y** closes the browser, and **X** selects the test tone.

Grain controls change the size, density, pitch and position of the sound fragments.
Press **X** for delay or reverb controls and raise **MIX** above zero to hear them.
The EQ page changes six frequency bands. MOTION adds reverse grains, pitch/size/
timing variation, stereo spread and sample scanning. Lower GRAIN/GAIN before large EQ boosts.

## Play without a WAV

Press **Y**, then **X** for the built-in source. On the **OSC** page, raise
**LEVEL** to about **0.05**, then press **A**. Select **WAVE** to try sine,
triangle, saw or pulse. **MIDI NOTE 57** is A3; **69** is A4.

![Built-in synth controls](docs/images/builtin-synth.png)

*Actual host-rendered interface, shown paused.*

The pages are **GRAIN · DELAY · EQ · REVERB · MOTION · OSC · COMP · TAPE**.
Use **L/R** to move between them. The oscillator can also play alongside a WAV.
**COMP** controls compression; start with MIX0 and raise it gradually. Lower
GRAIN/GAIN before large EQ boosts or makeup gain. The quiet test tone at OSC/LEVEL0
bypasses effects; raise LEVEL for synthesis through the rack.

## Tape-style sound

Open **TAPE** with L/R. Start with **MIX 0.50**, **DRIVE DB 6** and
**OUTPUT DB −6**. DRIVE adds grit, TONE darkens the sound, WOW adds slow pitch
movement, and FLUTTER adds faster movement. Set MIX to zero for the original sound.
This works with a WAV or the built-in synth; the quiet fallback test tone bypasses effects.

## Help

- [Installation, updates and backups](docs/INSTALLATION.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)
- [Report a problem](https://github.com/lkotlarenko/GrainMM/issues)

## Credits and development

**GrainMM design, native implementation and Miyoo adaptation by
[elly (@lkotlarenko)](https://github.com/lkotlarenko).**
**Original GrainBrick by [ETRNLIAM](https://github.com/ETRNLIAM).**
Porting and distribution permission granted with attribution.

Newly authored documentation uses the [scoped MIT license](LICENSE_SCOPE.md).
Original GrainBrick and third-party materials retain their own terms.
See [credits](docs/CREDITS.md) for authorship, references and acknowledgements.
Source, build instructions and engineering notes will be published when ready.
For now, follow development here and report issues or ideas through GitHub.
