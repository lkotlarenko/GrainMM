# Licensing, permission and provenance

**GrainBrick porting and distribution permission is confirmed.** On 2026-11-18,
GrainMM maintainer elly (@lkotlarenko) confirmed full permission to work on and
redistribute this port, with credit to the original author. The grant requires credit to the original author.

Credit: **GrainBrick by ETRNLIAM**; **GrainMM design, native implementation and Miyoo adaptation by elly (@lkotlarenko)**.
[Original project](https://github.com/ETRNLIAM/GrainBrick) ·
[Port project](https://github.com/lkotlarenko/GrainMM).
The public discussion linked in UPSTREAM.md is supplementary provenance; it is
not represented as the complete wording of the maintainer-confirmed grant.

## What the current package contains

The native C++ implementation, UI glyphs, generated icon, scripts and documentation
are original GrainMM work under the scoped GPLv3 license in
[LICENSE_SCOPE.md](../LICENSE_SCOPE.md). Their exact reviewed source paths are listed in
packaging/source-files.txt. The release package uses an exact file allowlist.

It dynamically uses the SDL1.2, libc, C++ and audio/runtime libraries already
installed on OnionOS. No device library, SDK, original AArch64 DSP library,
Python/NumPy distribution, GrainBrick sample, preset or brand asset is bundled.
This keeps the first native package small and avoids incompatible ARM64 binaries.
It does not imply that the maintainer lacks permission to redistribute upstream
materials; they are simply outside the current native implementation/package.

## Component register

| Component | Applicable rights / provenance | Current use |
| --- | --- | --- |
| Original GrainBrick application/materials | ETRNLIAM; maintainer-confirmed port/modification/distribution permission, with credit | Preserved reference; no blanket GPL relicensing |
| Newly authored GrainMM code/assets/docs | GPL-3.0-only from alpha10; earlier MIT grants preserved | Reviewed source/native app; alpha publication authorized2026-11-20 |
| Device SDL1.2 | LGPL-2.1-or-later; existing Onion-provided shared library | Dynamic system dependency; not copied into package |
| Compiler/libc/C++ runtime | GCC runtime exception and glibc/library terms as applicable | SDK is a private build input; device libraries not shipped |
| SoundTouch2.3.2 | LGPL-2.1-or-later, Olli Parviainen/contributors; compliance source archive preserved in reference | Not used or shipped |
| GrainBrick SoundTouch adapter | Separate MIT adapter source/license in reference archive | Not used or shipped |
| Mutable Instruments Rings/stmlib | MIT, Emilie Gillet; original notices/revisions retained in reference audit | Not used or shipped |
| NumPy/OpenBLAS/libgfortran bundle | Multiple BSD-style and runtime-exception notices; see original component notices | Not used or shipped |

The immutable initial file/hash inventory is docs/REFERENCE_INVENTORY.json.
Original inputs and guiding files remain private/ignored, with a verified
relocation map. Permission does not change third-party licenses or provide the
missing native DSP source. Do not claim feature parity or a successful ARMv7
rebuild of libraries for which no source is available.

## Future component reuse

Retain original author credits and each component's license notices. If a future
release bundles LGPL libraries, include matching corresponding source/build
materials using a compliant distribution method, preserve replacement/relinking
rights and do not restrict reverse engineering for debugging library modifications.
Static linking needs the additional materials required by the applicable LGPL.
The current app ships no LGPL library; the matching source/build distribution
for GPL binaries is a separate requirement.
References: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html and
https://www.gnu.org/licenses/gpl-3.0.html .

## Release readiness

Legal permission is no longer a release blocker. Technical acceptance remains
separate: target sample launch/playback, save/reopen behavior, controlled resource
use, clean exit, tested packaging and CI must be recorded honestly before calling
an artifact a stable release. Publishing source never includes private device
configuration, credentials, raw captures, guiding documents or the SDK.


## Alpha10 license transition

The owner explicitly authorized GPLv3 on2026-11-18. Current reviewed original work
uses GPL-3.0-only, retaining the MIT notice and all earlier grants. This changes
neither ETRNLIAM's materials nor third-party licenses. Native source/CI and the1.0.0-alpha.1 prerelease were authorized for publication
on2026-11-20. Public binaries require matching source/build files and notices. See LICENSE_SCOPE.md and development/RELEASING.md.
