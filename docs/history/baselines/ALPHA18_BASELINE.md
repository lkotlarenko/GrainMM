# Alpha18 feedback fixes

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-19. Owner found17working but laggy, balls lingering near walls and tab order
returning to SOURCE before TEMPO. Owner requested selectable palettes/default
readable burgundy. These are actual issues;17was not promoted known-good.

Changed page traversal to contiguous groups, preserving storage IDs. Added a
regression traversing all groups once and inverse shoulder routes. Older regression
scripts now reach their destinations via actual grouped key events; no controller
bypass. Preserve help context when passing STYLE.

Dynamic frames use50ms deadline pacing with at most20ms main input wait. Cached
background remains; animation <=20Hz. Rebounds have an energized .6+bounce minimum,
keeping the musical source moving. Long virtual motion regression checks interior
visits rather than only clamped telemetry bounds. This is intentional excitation
energy, not a passive mechanical simulation.

STYLE previews Burgundy/Amber/Mint/Ice/Violet/Mono; A commits, B/X cancels. Separate
strict theme.v1 uses retained procfs anchors/type/size/symlink guards, exclusive
pending file, fsync/rename. Bad preferences report/fall back without deletion;
leftover pending files are preserved. Sound codec remains12. Host-only theme I/O
tests never run against device system paths. Exact final evidence in RESUME.md.

## Final automated/native evidence

27host/26ASan/UBSan/LSan cases passed; final transport-badge change additionally
passed focused host/sanitizer UI/theme/navigation tests. ARM ELF/dependency and
collision numerical checks passed; unitSHAe301ff04ee12259842024cc0e28bda8b1c687db0bbdbf1ad04ba6ffa995581b9.
Runtime5/tooling8/hardware8/source2, format/analyzer/doc/source/reference gates.

Current/LAST_GOODafe50220da16/appSHA198a31dc24db81d7d9f60dbad13bc3458aecc439c4416581fe362928a4e2b9b2.
Install181495bytes, repeat-identical. First18candidate is archived rather than
overwritten; final menus also preserve truthful playback state while open.
Thirteen native cases pass including true grouped navigation, help context, color
preview/cancel/save/reopen with actual guarded filesystem storage, collision/slice/
effects/reset/patch/migration/rollback. Owner WAV/v9/v10/v11/v12 hashes preserved;
fixture files including theme.v1 were removed individually. Only final18+verified16
remain,3648KiB including state; superseded candidates archived/hash-checked/pruned.

All11default native PCM checksums match16. Offline highest p95/worst us:16default
6838.334/10932.667;18default7465.833/10412.667; warmed collision/texture5927.166/7907.834.
These are bounded unmanaged-DVFS/VNC2FPS runs, not speedup/zeroXRUN evidence. Prior
all-on20.49ms/512 and17dummy27.17ms/1024 outliers remain open. Physical pacing,
wall rebound and theme acceptance are recorded in RESUME.md. No new image inspection.

## Owner acceptance and final maintenance

Owner reports “working so much better”, accepting the improved movement, pacing,
group order and palettes. Closed prior18UI log:18157callbacks/1024,worst24518us,
late0,software clips0,1815wall hits,1045slices,650updates. No steady CPU/XRUN claim.

Final guarded matching-format cache copies remove the vendor SDL per-blit log path.
The final13native dummy cases have zero SDL_CalculateBlitN lines, preserve all owner
WAV/settings hashes and remove exact fixtures. Scene23641us/1024/late1 remains a
profiling concern. The matching burgundy launcher icon is original generated art;
root asset replaced atomically only after old/new hash checks. Sound is unchanged
from the accepted18engine. Owner has not separately listened to this maintenance
executable; final cache/icon checks are automated evidence. Independent matching
source draft builds and passes27host tests; final focused copy sanitizers3pass.

Two direct SSH fbcon probes initialized and exited with zero UI requests, even
with stdin held open. They establish startup/cleanup only, not rendered-frame
acceptance. Private logs retain this limitation; exact fixture was hash-checked
and removed. Do not repeat screenshots. Normal Onion launch is owner-confirmed.

Next musical workflow: NEXT_INSTRUMENT_REPORT.md prioritizes temporary recall,
ambient evolution, dual/wavetable synthesis, manual slicing and export.
