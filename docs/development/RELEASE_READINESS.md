# Release readiness

Updated 2026-11-20. **1.0.0-alpha.1 is a candidate, not a stable release.**
Permission to adapt/distribute GrainBrick with ETRNLIAM attribution is confirmed.
The owner authorized reviewed native source, CI and the experimental prerelease
on2026-11-20. This supersedes the documentation-only scope. New work is scoped GPL-3.0-only with earlier MIT grants preserved.

| Gate | Required evidence |
| --- | --- |
| Source quality | Host/sanitizer checks with leaks enabled; runtime/tooling; source/docs gates; reviewed diff/allowlist; preserved440reference hashes |
| Exact artifacts | ARM ELF/runtime validation, install ZIP, matching source/checksums, fresh notes and repeatable packaging |
| Candidate hardware | Launch, each new source/groove feature, controls/banks, save/reopen/patches, export/resample, clean MENU exit |
| Resource use | Measured new workload CPU/RSS/timing, documented scheduling spikes and limitations; VNC-off checks |
| User installation | Fresh-folder SD installation, updates/backups/rollback; no system changes |
| Publication | Explicit native source/branch/tag/binary authorization; matching-source metadata enabled only afterward |

[1.0 validation](V1_VALIDATION.md) records completed checks. Alpha25's accepted
listening/navigation is a previous-build checkpoint, not validation of new DSP.
SDK/Actions are pinned; hosted Actions and hermetic cross-host builds are unverified.

Before calling any build stable: longer combined-source listening/stress,
headphone/speaker behavior, repeated loads/exits, corruption/full-disk recovery,
power/sleep behavior and VNC-off measurements. Current SDL exposes no hardware
XRUN counter. Callback lateness does not establish an underrun or zero underruns.

The beginner README describes the experimental download and hardware/app
limits. No parity percentage, MIDI, recording or unsupported hardware claim should
appear in release copy. [Release process](RELEASING.md) explains rehearsal and
publication. [Resume](RESUME.md) records exact device state and remaining actions.


## Experimental release scope

This prerelease is for testing. The owner requested publication while new1.0DSP
and groove physical acceptance remains pending. The previous alpha25 hardware
checkpoint is preserved; new-feature tests are host/ARM evidence. The release must
state these limits plainly. Final source review, hosted CI, exact source/install
checksums and a tested release workflow gate publication. Stable_ready remains
false. [Publication review](PUBLIC_RELEASE_REVIEW.md) records final evidence.
