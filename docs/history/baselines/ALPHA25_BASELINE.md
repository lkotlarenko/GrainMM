# Alpha25 — Patterns, cuts and full-instrument export

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-19. Original GrainMM work by elly (@lkotlarenko), inspired by GrainBrick
by ETRNLIAM. No reference implementation, samples or visual assets were copied.

## Delivered behavior

- A 16-step manual slice editor: regions, rests, reverse, probability and cuts.
- Independent cursor and playback position, waveform boundaries and mode indication.
- Generated/manual switching without discarding the manual pattern.
- Schema 17 pattern persistence in autosave, named patches and snapshots.
- Complete offline graph export, including layered sources, FX and OUTPUT.
- Duration/progress/cancel controls and safe numbered WAV files in `state/exports`.
- A separate `grainmm-export` developer tool that opens no audio/display device.

The generated sequence and existing scalar storage order remain unchanged.
Pattern preparation stays outside audio; fixed snapshots use the existing queue.
No callback allocation or filesystem work was added. Export pauses live audio and
borrows the immutable sample; it does not duplicate sample data or record live tails.
See [ADR 0028](../../adr/0028-composition-patterns-and-offline-export.md).

## QA and corrections

The first integration pass found an older probe translating shoulder presses into
page changes even inside the new editor, and a stale assertion that source names
ended the save file. Both were corrected. The full main host suite passes all
35 tests in 168.86 seconds; the full sanitizer suite passes 34 in 133.73 seconds,
including leak checks outside ptrace.

One host contact sheet was inspected for the two new screens. It exposed an
overlapping long title; the context line now sits below it. Host/sanitizer display
tests pass after that change. Existing captured grain pixels refreshed the README
image without more inspections. Captures are archived with hashes on the host.

Final review found that bounding an extreme-width cut pair could move unrelated
boundaries. The new regression fails against the earlier implementation and passes
after constraining both neighboring widths before storage. Five affected host
tests pass in 0.97 seconds and five sanitizer tests pass in 1.91 seconds afterward.
The final exact-code source archive rebuild passes all 35 tests in 168.97 seconds.

Static analysis covers all 47 source translation units, with a follow-up on the
cut correction. Runtime 5, tooling 8, hardware tooling 8 and source archive 2 tests
pass. ARM hard-float ELF/dependencies and packaging validation pass. All 440 original
reference hashes remain intact. Host results alone are not physical audio evidence.

## Initial native evidence (pattern/export build)

Initial accepted pattern/export build SHA-256:
`596a911e8f889062aca1b2c84f578d16e34fc1cc2c583caf3db9389e1bdfdf3e`.

Thirty bounded silent app cases pass on Miyoo Mini Plus / OnionOS 4.3.1-1, including
manual editing/save/reopen, migration from schema 16, legacy controls/reset/patches,
sources, scenes and earlier migrations. Three native probes pass for manual pattern
DSP, allocation-free slicing and full export/cancellation. The native export CLI
also writes a one-second stereo WAV; the app loads that exported WAV successfully.
Temporary fixtures/binaries/WAVs were checked and removed individually.

The suites ran sequentially with stock unmanaged clock/background conditions and
SSH. VNC was not toggled or reverified. Across different app cases, the worst callback
hint was 28,798 us with 25 late hints; software clipped samples were zero. These
are not hardware XRUN counts or proof of a sustained worst-case budget.

Offline comparison: 11 workloads per suite, 93 blocks of 512 frames per workload.

| Suite | Highest p95 (us) | Worst block (us) | Software clips |
| --- | ---: | ---: | ---: |
| Alpha24 default | 4,846.167 | 15,141.333 | 0 |
| Alpha25 default | 6,202.833 | 11,934.334 | 0 |
| Alpha25 texture | 8,809.833 | 15,010.167 | 0 |

All 11 default output checksums match alpha24. Scheduling/clock conditions are not
controlled sufficiently for a speedup claim. Outliers still exceed a 512-frame
period; longer combined-load and VNC-off profiling remain open.

## Initial owner check and storage

Owner reports: “Pattern/export work; audio clean; saved.” The app was closed,
schema 17 save and both installed hashes verified, and alpha25 promoted to known-good.
The retained rollback is alpha24. Original WAVs and schema 16 bytes remain unchanged.

The final session log records 1,353 callbacks of 1,024 frames, span 35,899 ms,
worst hint 29,122 us, late 0, software clips 0, 28 slices and 64 updates. This is
a short mixed-source listening check, not a long soak or hardware zero-XRUN result.

A read-only follow-up initially found no retained owner export. The owner then
explicitly completed EXPORT WAV and confirmed the SAVED message and successful
operation. She requested moving export out of FX, prompting the navigation follow-up
in ADR 0029. The automated native evidence remains distinct from owner listening.

Only alpha25 and verified alpha24 remain: 8,640 KiB including existing WAVs/state
at acceptance. Superseded builds were compared against host archives before exact
file removal. Future owner exports consume additional space; they are never pruned
automatically. Source/package archives remain private; nothing was published.

## Next work

Complete the requested navigation follow-up, then match VNC-off/longer
hardware conditions before calling a public build stable. Next synthesis priority
is dual/generated-wavetable sound design with measured target voice limits. Pattern
variations, musical swing/automation and further routing follow a stable one-pattern
and export workflow. Live recording, MIDI and shimmer remain separate work.


## Navigation/usability follow-up

After confirming export, the owner requested moving it out of FX. SOURCE now has
an explicit PATTERN entry; FX is a 3×3 set of nine processors; TOOLS is a 2×2 group
for EXPORT WAV, PATCHES, MASTER OUTPUT and COLORS. Shoulder browsing skips file
operations. Existing shortcuts remain. ADR 0029 records the rationale and stable IDs.
A second essential host inspection verified the new TOOLS layout; all groups and
six palettes pass numeric drawing bounds. No device screenshots were collected.

Full follow-up host 35 passes in 185.76 s; full ASan/UBSan/LSan 34 passes in 144.34 s. Independent
source rebuild 35 passes in 185.75 s. The initial native scene regression then revealed
its listen delay moved before Play after adding a menu key. Corrected the probe's
delay from stage 32 to 34; focused host/ASan scene tests pass in 6.83 / 6.84 s. Its failed
fixture was host-archived/verified before exact cleanup. All 31 final native app cases
now pass, including the new real-menu export/reload/play route; its log has one
completed export, zero cancellations, one pattern view and one source replacement.

Final app SHA is 1172cde5f2068873489159f3425427b4498416289ee811b748dc8efaa077c0ca.
The previous paragraphs' sound-build SHA identifies the earlier accepted revision.
Final native worst callback hint 29,831 us, late hints 28, software clips 0 across varying
cases; no hardware zero-XRUN or controlled performance claim. Owner's actual retained
15-second WAV was verified at 2,880,044 bytes. The app tree is 11,520 KiB including owner
files; only current and accepted rollback remain. The owner subsequently confirms the final navigation is clear and working.
Closed/save/binary checks passed; CURRENT/LAST_GOOD now identify the final UI build,
with the previous accepted pattern/export revision retained for rollback.


Final corresponding-source verification compiles every target from the 304-file
allowlist archive, then passes seven feature CTest cases (0.96 s) and the corrected
scene plus full menu/export/reload application tests (14.20 s). The final 31 native cases and owner acceptance refer to
`1172cde5…077c0ca`. CURRENT/LAST_GOOD is the
final UI revision; accepted prior sound revision is the only rollback. SDK contents
were re-audited against the pinned archive and compacted; host rebuild trees and
captures were archived/verified before exact cleanup. No code or binaries published.
