# Alpha19 — temporary sound recall

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-19. First implemented priority from NEXT_INSTRUMENT_REPORT.md. One owned
bounded settings copy plus source generation; no sample/disk/audio-state snapshot.
SELECT-first A captures/B recalls on the main view without changing playback.
Successful source replacement invalidates recall; empty/stale slots retain the
current settings and explain why. Ordinary help/reset/transport stay intact.
See ADR0022. Sound settings12/108fields and all DSP algorithms are unchanged.

## Evidence

-28host tests passed118.64s;27ASan/UBSan/LSan tests passed96.30s outside ptrace,
  leak checks retained. Focused snapshot/shortcut/navigation/display4passed.
- Independent254-file source draft builds and passes28host tests118.30s. Final
  source adds this baseline/updated handoff only; native code is unchanged.
- Full actual SDL sequence tests empty recall, playing capture/edit/recall,
  changed-source refusal, paused recapture/edit/recall and MENU persistence.
  Source change is real browser→built-in activation, not controller state mutation.
- SDK ARM ELF/dependency validation, runtime5/tooling8/hardware8/source2 tests pass.
  Format/analyzer/source/document/reference gates recorded in RESUME.md.
- Candidate16e37769b28b/appSHA56596ee5252fcd3d038578f2ff689399ef40e9139a0d6df58e1c11628bd990e1.
  Install183961bytes, repeat identical. Sound-render/benchmark binaries are unchanged
  from18. Matching source is reviewed/packaged separately; no publication authorized.
-14bounded native stock-preload/SDLdummy cases pass, including the complete new
  capture/recall flow and prior sound/reset/slice/theme/patch/migration/rollback.
  Snapshot flow:2captures,2recalls,2rejections,2transport toggles,0help/trim opens,
  playing/paused state retained.74callbacks/1024,worst3253us,late0,software clips0.
- Across the native batch, worst24372us/1024; some cases retain late scheduling
  hints. This is not an XRUN count or stress/audio clearance. No new benchmark
  claims: DSP unchanged; earlier outliers/VNC-off/headphone/power gates remain.
- Owner WAV/v9/v10/v11/v12 hashes preserved; fixtures removed individually. Only
  candidate19+accepted18rollback remain,3680KiB including state before owner check.
  Archived16release removed only after exact node/type/hash checks. No system changes.

Physical gesture/icon/listening check is pending in RESUME.md. User acceptance of
alpha18 does not establish this new physical gesture. No screenshots were taken.
Next work is the slow modulation model and DRONE workspace, then dual/wavetable,
manual patterns/cuts and sound export. Snapshot is settings recall, not effect-tail
rewind, true live freeze or an ambient engine by itself.
