# Credits

## GrainMM

**Created and maintained by elly (@lkotlarenko).**

GrainMM's instrument design, native C++ implementation, Miyoo Mini Plus adaptation,
interface, build/release tooling and testing are developed in this project, with
AI-assisted engineering. It is an independently written instrument rather than a
rebuild of GrainBrick's supplied native libraries. The newly authored documentation uses the scoped
[MIT license](../LICENSE_SCOPE.md); native source publication is planned.

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
| [PocketTracker](https://github.com/conanizer/pockettracker), by conanizer and contributors | Credits organization and research into handheld effect bypass/lifecycle patterns; no code or assets included |

System libraries are dynamically used by the app, not bundled. Research
acknowledgements do not mean those projects authored or endorse GrainMM.
No PocketTracker code or assets are included. Original components keep their own
terms; future borrowed code must retain the authors' notices and applicable license.
