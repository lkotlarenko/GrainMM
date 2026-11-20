# GrainBernik comparison

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


Reviewed 2026-11-18 at commit
`100d8db7bdc47f49fc02a4bba47945ade7f3537b` in
[AlekseyMalykh/GrainBernik](https://github.com/AlekseyMalykh/GrainBernik).
Read-only source/document inspection; no foreign installer or code was executed.
Research copies remain ignored on the host, not on the Miyoo or in a release.

## What it actually ports

GrainBernik adds Python adapters around the official GrainBrick beta1.0/1.0.1
application for Anbernic RG40XX H on KNULLI/ROCKNIX. Its
[launcher](https://github.com/AlekseyMalykh/GrainBernik/blob/100d8db7bdc47f49fc02a4bba47945ade7f3537b/rg40xx-h-patch/grainbrick-beta/gb_launch.py)
checks Python3.12 and ARM64 before importing the original application. It uses
bundled NumPy and Pygame. The inspected tree does not provide a portable C/C++
implementation of GrainBrick's granular/effects engine. Its kernel-module build
sources concern USB gadget audio, not a replacement DSP engine.

Consequently this is not a drop-in ARMv7/128MiB OnionOS port. Its success on a
640×480 Anbernic does not establish Miyoo CPU/RAM/ABI or audio compatibility.
GrainMM retains its native architecture and measured target adapter.

## Useful lessons and concrete follow-up

| Observed mechanism | GrainMM decision |
| --- | --- |
| Legacy display adapter reuses surfaces and scales the full canvas; falls back from smooth scaling after repeated slow frames | Keep direct640×480 drawing/cache. Measure presentation cost; avoid adding full-canvas scaling. |
| Physical shutdown diagnosis found problems absent in headless tests | Require ordinary Onion foreground launch/exit as well as dummy-driver tests. Keep signal/relaunch tests and bounded cleanup. |
| ALSA USB card discovery uses capabilities/identity instead of a fixed card number | If recording/USB is later supported, discover real capabilities; do not hard-code another console's cards. No capture support claimed now. |
| Hold/toggle controls, deadzones and smoothing have regression tests | Add explicit physical-button event/repeat tests as mappings grow; Miyoo has no analog sticks, so do not mirror its stick UI. |
| Preserve settings/recordings when updating | Release ZIP contains no state folder. Document backup and copy-only-state upgrades. |

The [shutdown report](https://github.com/AlekseyMalykh/GrainBernik/blob/100d8db7bdc47f49fc02a4bba47945ade7f3537b/docs/SHUTDOWN_FIX.md)
diagnoses an unused Pygame mixer and blocked worker pipe on that platform.
GrainMM uses SDL for its actual audio and has no Pygame/aplay worker: applying its
dummy-audio workaround would mute GrainMM. Keep this as a test lesson, not a fix.
Its PipeWire/WirePlumber, governor and gadget/kernel changes are platform-specific
and outside GrainMM's device safety boundary. None were applied to the Miyoo.

## Licensing and credit

The public API reports no repository license and the inspected tree has no
repository-level LICENSE/COPYING file. No license grant was found in the inspected
adapter headers. ETRNLIAM's permission for GrainMM does not automatically license
another contributor's independently authored patches. No GrainBernik code or assets
were copied. Public behavior, test scenarios and architectural observations inform
original implementation work; any future source reuse needs applicable terms and
credit to Aleksey Malykh alongside the original author and GrainMM port maintainer.

This comparison does not change the confirmed GrainBrick permission or create a
new blocker for publishing the independently implemented GrainMM package.

## DSP sourcing follow-up

The original GrainBrick public main tree was checked again on2026-11-18 and
contains only README.md. Neither that tree nor the supplied local native binaries
provides their implementation source. Source access would offer the most faithful
porting route; it is distinct from the already-confirmed distribution permission.

Without it, prefer measured portable DSP reuse plus original application/control
logic over binary decompilation. Candidates inspected read-only:

- [DaisySP](https://github.com/daisyaudio/DaisySP): effects/filter library with mixed per-file licensing (MIT and LGPL; not uniformly MIT)
  component notices. Audit the exact selected files and dependencies before import.
- [Clouds granular processor](https://github.com/pichenettes/eurorack/blob/master/clouds/dsp/granular_processor.h):
  inspected header has Emilie Gillet's MIT notice; includes granular, reverb and
  pitch processing. The shown engine uses a32kHz rate and hardware-oriented
  dependencies, so this is not a drop-in48kHz Linux adapter.
- [Signalsmith Stretch](https://github.com/Signalsmith-Audio/signalsmith-stretch):
  pitch/time processing, MIT license verified. MM+ CPU, latency, dependencies and
  real-time allocation behavior are unmeasured; not selected for the audio path.

No candidate library was imported, built or deployed. Their sound differs from
GrainBrick until proven otherwise. For the next DSP increment, compare a small
portable delay/filter/reverb candidate against an original bounded implementation,
using host numerical/impulse tests followed by real ARM deadlines/RSS and listening.
Decompilation is a fallback for narrowly specified missing behavior, not the primary
schedule: it does not recover source/build ownership or guarantee sound parity.
