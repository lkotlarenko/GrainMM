# ADR 0029 — Separate sound processing from file and project actions

2026-11-19. Alpha25 usability follow-up requested after the owner's export test.

Export was discoverable only through B on OUTPUT in FX. That mixed a file operation
with sound processors. Keep the four top-level groups, but replace STYLE with TOOLS:

- SOURCE contains instruments plus a direct PATTERN editor entry.
- FX contains the nine shared sound-processing pages in a 3×3 navigator.
- TEMPO retains the shared musical clock page.
- TOOLS presents EXPORT WAV, PATCHES, MASTER OUTPUT and COLORS in a 2×2 layout.

Export, patches and pattern editing are explicit navigator actions with separate
virtual IDs. They are not scalar parameter pages and do not change the schema.
Opening export still needs an explicit A, followed by another A to start writing.
Shoulder browsing skips file actions so passing a tab never starts an export or
pauses audio. The master-output page moves after TEMPO; each group is visited once
per full shoulder cycle. Existing START, B-on-OUTPUT and B-on-SLICE shortcuts remain.

Reuse the existing patch-opening transaction rather than duplicating save/load
logic in the navigator. Keep pattern/export controllers separate from the main
event adapter. Control-thread diagnostic counters record pattern views, completed
exports and cancellations, without callback logging or file work.

Tests cover every destination, the complete shoulder cycle, all group layouts in
every palette, and real application navigation to PATTERN and EXPORT WAV followed
by export, browser reload and playback. Update user-facing paths together with the
UI; historical version evidence retains its old paths.
