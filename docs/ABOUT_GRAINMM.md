# Why GrainMM is a native instrument

**GrainMM is created and maintained by elly (@lkotlarenko).** It brings the idea
of GrainBrick to the Miyoo Mini Plus through an independently written native
implementation. **GrainBrick is the original project by ETRNLIAM**, adapted with
permission and attribution.

The goal is a compact, enjoyable granular instrument with strong feature coverage
and good sound on the Miyoo. Musical usefulness matters more than reproducing
GrainBrick's code, screen layouts or audio output exactly.

## Why this needed more than a simple port

The supplied GrainBrick release was built for a different machine. Its core DSP
libraries are mostly64-bit AArch64 binaries. The Miyoo Mini Plus runs32-bit ARMv7;
those binaries cannot run there. Native source for the main supplied DSP modules
was not present in the reference release, so changing a compiler setting could
not rebuild them for the Miyoo.

The application also depends on a Python/Pygame/NumPy environment and supporting
programs. Its bundled NumPy and runtime binaries target the other architecture.
Putting that whole environment on a128MiB handheld would still leave the missing
DSP implementations and require its own memory/performance investigation.

The screen, controls and operating system differ too. GrainBrick's1024×768 logical
interface does not fit the Miyoo's640×480 screen directly. Onion's audio routing,
launch process, display rotation and buttons needed real-device investigation.
Moving the original files onto the SD card could not resolve those differences.

The initial audit and detailed hardware findings will accompany source publication.

## What we built for GrainMM

- A native C++17 audio engine with granular playback, a playable oscillator,
  EQ, compression, stereo delay, reverb and tape-style coloration. DSP modules
  are independently authored; the app does not load GrainBrick's native libraries.
- A direct640×480 interface for the Miyoo's D-pad and buttons, with six controls
  per page, sample folders, silent startup, play/pause and normal MENU exit.
- A bounded audio path with preallocated storage and prepared parameter updates.
  File loading, saving, drawing and coefficient preparation stay outside audio.
- Safe WAV loading, source replacement and versioned settings. Older settings
  remain intact, failed saves preserve recovery data, and missing files retain
  their selected path until the player explicitly chooses another sound.
- Reproducible ARM builds, numerical/host/sanitizer tests, dependency/package
  checks, small installation ZIPs and release workflows.
- A real-Miyoo development cycle with isolated deployments, a verified rollback,
  bounded measurements and owner listening tests.

Some issues only appeared on the hardware: the physical display was rotated;
Onion's audio preload changed file-opening behavior; and stopping audio safely
required joining its callback before destroying the sample. Those are solved in
GrainMM's adapters without replacing Onion libraries or changing system settings.
New performance findings continue to shape implementation choices.

## How it differs from GrainBrick

| Area | GrainMM's direction |
| --- | --- |
| Platform | Miyoo Mini Plus / OnionOS first; other devices are untested |
| Implementation | Original native C++ engine and UI; no Python runtime or bundled upstream DSP |
| Sound | Practical musical equivalents; related features need not sound identical |
| Interface | Compact button-operated parameter pages rather than the original desktop-sized layout |
| Files | Own WAV browser and versioned settings; no claim of GrainBrick project compatibility |
| Features | Smaller working set today; recording, MIDI, shimmer, resonators and other features remain in development |
| Distribution | Small app package using the device's runtime; no included GrainBrick samples or libraries |

Working native features do not mean full parity or a stable release. Current
acceptance and remaining gaps will accompany source publication.
A successful ARM build or silent test alone does not prove clean physical audio.

## Where development goes next

Use short working iterations: pick a useful feature or measured bug, implement it,
run focused checks, test on the device, collect listening feedback and keep a
known-working rollback. Improve the sound and controls before adding complexity.
Near-term work includes performance headroom, additional musical effects and
modulation, followed by better sample/project tools and longer reliability tests.

## Credit and ownership

GrainMM's design, native implementation and Miyoo adaptation are elly's project.
ETRNLIAM retains credit for the original GrainBrick project and inspiration.
These roles deserve clear, separate attribution. See [credits](CREDITS.md).
New GrainMM source is planned for publication under scoped MIT terms; upstream and third-party materials keep
their own licenses. Permission to adapt GrainBrick does not relabel those materials
as newly authored GrainMM code.
