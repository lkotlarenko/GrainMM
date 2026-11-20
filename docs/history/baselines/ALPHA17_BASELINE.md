# Alpha17 development record

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-19. UI rework followed by real collision source, as explicitly requested.

Implemented original SOURCE/FX/TEMPO navigator, charcoal/orange/cyan instrument
panel, shared dialog chrome, cached scene/labels/controls, actual source waveform,
audio meters/grain cursor/slice steps and collision poses. Dynamic-only frames cap
15FPS; controls retain25FPS. One extra640×480 surface. Numeric precision follows
control step; MIX/output placement retained. No imported assets or GPU dependency.

BALLS controls hexagon-wall collision physics; BOWL controls its original modal
sound. Six fixed balls, eight two-mode voices,120Hz audio-frame physics and stable
prepared coefficients. Paused/disabled idle fast path; source layers into the shared
rack. Not ball-to-ball collision, external clock or multi-model Orbix parity.
Settings12/108controls preserve older1–11 files. See ADR0020.

One essential host contact sheet inspected; footer collision fixed, labels enlarged.
Final actual renderer examples regenerated without a second image inspection.
No routine device captures. Test evidence/selector/acceptance in RESUME.md.

## Automated and target evidence

-26host/25ASan/UBSan/LSan cases passed, leak checks retained outside ptrace.
  Runtime5/tooling8/hardware8/source2, formatting, analyzer/source/doc and440reference
  preservation gates passed; SDK ARM ELF/dependency checks unchanged.
- ARM collision numerical suite passed, SHA256:
 de85848bc8bbbe6dc1a369878777c3a6c49ddab34fb333f45fb88a1fef163d8f.
- InstallationZIP176202bytes, repeat byte-identical. Native source stays private.
- CURRENTa8b26259cb96, appSHA256:
 17286f451183e299a98be2320ee6d1b088813e1b398773b9d8c30c347e9c762b.
  LAST_GOOD/PREVIOUS6275a2b5f2a4 is verified16 until physical acceptance.
- Thirteen native stock-preload/SDLdummy cases pass: texture/save/reopen; rack,
  modulation/factory reset; held edit/reopen; slice/save/reopen; grouped navigator/
  collision/BOWL/help/save/11migration/reopen; patch/waveform; alpha16 rollback.
  Owner WAV/v9/v10/v11 hashes preserved, all exact fixture files removed.
- Scene flow:2real wall impacts,2transport toggles,4updates, worst19063us/1024,
  late1;42presentations reused16static layouts. Reopen had2presentations/1layout.
  Slice case worst27172us/1024/late1; patch case19764us/late1. These are short dummy
  runs; lateness is not an XRUN counter and dummy rendering is not visual acceptance.
- After exact archive/type/hash checks,19alpha15files were removed individually.
  Only current17+verified16 remain,3456KiB app/state before owner retry. No owner
  files, system/configuration/services or runtime libraries changed.

## Timing and remaining profiling

Stock VNC2FPS/SSH, no CPU policy changes. Eleven offline workloads×93blocks/512
per suite. All11default checksums match16. Highest p95/worst us:16default4885.333/
7362.5;17default5156.667/10193.166. Collision+all-texture suite5813.5/9096, including
2virtual seconds of warmup per collision workload followed by1timed virtual second.
Do not infer relative speed from these unmanaged-DVFS short runs. Dense collision
suite deliberately skips grain spawns at the existing cap; this is not an XRUN.
Previous16all-on20493.334us/512 outlier and new dummy27172us/1024 remain documented
budget concerns. Longer stress/headphone/power/sleep/VNC-off tests are still open.
One host contact-sheet inspection caught footer overlap; corrected geometry and
larger labels passed bounds/cache tests. No new device images were taken.
Physical readability/navigation/audio/save acceptance is recorded in RESUME.md.

## Owner feedback

Owner confirms it works, but reports lag, wall-hugging balls and inconsistent
header traversal; asks color profiles with readable Burgundy default. Candidate17
was not promoted. Alpha18 addresses those issues;16remains verified rollback.
