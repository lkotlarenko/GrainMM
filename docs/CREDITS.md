# Credits

## GrainMM

**Created and maintained by elly (@lkotlarenko).**

GrainMM's instrument design, native C++ implementation, Miyoo Mini Plus adaptation,
interface, build/release tooling and testing are developed in this project, with
AI-assisted engineering. It is an independently written instrument rather than a
rebuild of GrainBrick's supplied native libraries. The new work uses the scoped
[GPLv3 license](../LICENSE_SCOPE.md).

## Original project

**GrainBrick by [ETRNLIAM](https://github.com/ETRNLIAM/GrainBrick).**

GrainBrick supplies the original project and feature inspiration. GrainMM is
adapted with the author's permission and attribution. Its original materials and
third-party components retain their separate terms.

## Runtime and community

| Project | Role in GrainMM |
| --- | --- |
| [OnionOS](https://github.com/OnionUI/Onion) | Target environment and device-provided runtime |
| [SDL](https://www.libsdl.org/) | Device SDL 1.2 / host SDL 2 audio, graphics and controls |
| [Miyoo toolchain](https://github.com/shauninman/miyoomini-toolchain-buildroot) | Pinned ARM compiler/SDK used to build the app |
| [MIASMORA](https://github.com/oldtokage-collab/MIASMORA), by Tokagetchi | Owner-supplied behavior reference for a self-running ambient instrument; no code/assets/presets included |
| [PocketTracker](https://github.com/conanizer/pockettracker), by conanizer and contributors | Credits organization and research into handheld effect bypass/lifecycle patterns; no code or assets included |

System libraries are dynamically used, not bundled. Research acknowledgements do
not mean those projects authored or endorse GrainMM. Exact component usage and
terms are recorded in [legal and provenance](LEGAL_AND_PROVENANCE.md).
Algorithm references and implementation tradeoffs belong in the [architecture
records](adr). Contributions should be credited by their actual role; borrowed
code must retain its authors' notices and applicable license.

## Algorithm references

The original resonator implementation uses the constant-peak-gain filter form
explained by Julius O. Smith in [Introduction to Digital Filters](https://www.dsprelated.com/freebooks/filters/Constant_Peak_Gain_Resonator.html).
This is a mathematical reference, not imported library code. Implementation and
quality/cost decisions are recorded in ADR0011.
