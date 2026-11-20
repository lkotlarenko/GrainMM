# GrainMM development progress

Updated 2026-11-19. Native C++17 sound instrument for Miyoo Mini Plus / OnionOS.
GrainMM design, native implementation and Miyoo adaptation by elly (@lkotlarenko);
original GrainBrick by ETRNLIAM. Good musical behavior and useful feature coverage
are the goal; identical DSP and original project compatibility are not promised.

## Current development: alpha23 / output and ambient scenes

The owner confirms substantially improved ball movement, responsiveness, group
navigation and colors. The final maintenance package adds a matching burgundy
launcher icon and removes vendor SDL per-frame cache-blit logging. Exact artifacts,
selectors and evidence are in [RESUME.md](../development/RESUME.md) and the
[alpha18 baseline](baselines/ALPHA18_BASELINE.md). Exact current/rollback selectors are maintained in RESUME.md; owner samples/settings/patches stay intact.

| Area | Working coverage | Next |
| --- | --- | --- |
| Grain source | Fixed16voices, reverse/randomness/scan/trim/window/stereo/harmony | Dual/wavetable synthesis and manual slice editing |
| Synthesis | Basic oscillator; real six-ball/eight-voice wall-impact source | Dual oscillator/wavetable and additional excitation models |
| Rhythm | Audio-frame BPM, generated16step slice patterns | Manual steps/cuts, phase-locked effects and tempo workflow |
| FX | EQ, compressor, delay/reverb, tape, resonator, rhythm, crusher/filter/chorus | Measured routing/space expansion |
| Media/state | Rooted WAV browsing,132controls/settings16,16named patches, visual trim | Full-instrument offline export and exchange |
| UI | Cached studio scenes, grouped SOURCE/FX/TEMPO/STYLE,20Hz cap,6palettes, hold repeat/reset scopes | Ambient performance view and compact editing workflows |
| Reliability | Host/sanitizer/ARM/native regression and short owner acceptance | Timing outliers, longer stress, headphones, power/sleep, VNC-off resources |
| Distribution | Scoped GPLv3, exact install/source allowlists and prepared CI | Hosted validation and explicit native publication authorization |

See [the next-instrument report](research/NEXT_INSTRUMENT_REPORT.md) for the
prioritized ambient/snapshot/synthesis/pattern/export plan, and
[feature comparison](research/FEATURE_COMPARISON.md) for remaining upstream gaps.
No parity percentage is inferred from page counts. Backend XRUN counts remain
unavailable; callback lateness is a scheduling hint. Native code/workflows/binaries
are still private under the owner's documentation-only publication policy.

The versioned notes below retain historical observations and pending checks as
recorded at the time. Use RESUME.md, not an earlier candidate's “next” instruction.

## Alpha14 UI iteration (historical)

Six parameter cards, source/transport state, page/range indicators and a cleaner
Apps icon/description. Hold START+SELECT800ms for confirmation; X applies,
B/SELECT cancels. Individual taps open help/patches on release. Native held/cancel/
repeat/save/rollback probes pass. One essential host main-frame inspection, no
new device screenshots. See ADR0016 and ALPHA14_BASELINE.

## Earlier alpha12/13 iteration

Three original effects, confirmed dry reset and one typed parameter registry. No
added grain voices/sample copies/dependencies. Final host21/sanitizer20 and native
functional/numerical tests passed. Dense all-on p956268.167us/512; repeated6153.5us,
but larger outliers remain. All11default native checksums match alpha11. State9/
owner WAV hashes preserved; old app rollback reads its older saved state. See
Alpha12's held shortcut was replaced after physical feedback; alpha13 confirmation
passed cancel/reopen/repeat tests. See ADR0015 for architecture and FEATURE_COMPARISON for missing sequencer/dual-source/
shimmer/tempo capabilities that effect names alone do not cover.

## Earlier alpha11 iteration

Original prepared grain chord/order/octave/probability/detune with unchanged voice
budget. All11default native graph checksums match alpha10. Dense full graph p95
5004.666us off/5024.833us on per512frames; changing clock/background conditions
prevent controlled overhead claims. Two dummy runs had1late hint each; no backend
XRUN counter exists. User state preserved; older app rollback reads untouched v8.
See ADR0014 and baseline for conditions and physical acceptance.

## Earlier alpha10 iteration

New workflow and modularity; existing DSP preserved. Patch load prepares its next
source before releasing the old instrument; missing WAV failures keep the old
sound intact. Successful source loads return paused. Save confirmation and draft
names avoid accidental replacement. The waveform scans input once, not per redraw
or callback. No new screenshots were needed. License scope, matching-source rules
and current user/developer guidance were audited; older grants/evidence remain.

## Latest alpha9 iteration

Original resonator/rhythm, non-destructive grain region/window and contextual help.
Host18/sanitizer17, native numerical and seven dummy lifecycle cases passed.
Owner confirmed clean audio and saved controls. Real WAV session worst19195us/1024,
late0,clips0,4100grains/no skips,171updates. RSS6280KiB; short interaction CPU5.57%
ofonecore is not a stress bound. See development/ALPHA9_BASELINE.md and ADR0011.

## Earlier alpha8 iteration

Original tape-style DSP, six saved controls and native/host regressions. Owner
confirmed clean sound and saved values. Real worst17204us/1024,late0,clips0.
Timing outliers remain under investigation; see development/TAPE_BASELINE.md.
PocketTracker research informed lifecycle/credits organization; no code was copied.
GitHub main publishes documentation only at the owner's request. Native source
and workflows remain private while alpha development continues.

## Earlier alpha6 batch

Original musical oscillator (sine/triangle/saw/pulse,note,detune,duty,vibrato) and
stereo-linked compressor. One graph handles WAV layering and no-WAV synthesis,
with quiet fallback tone still available. Seven real pages and42saved controls.
Audio sample ownership and prepared snapshots are retained; probe code moved out
of main. Host16/sanitizer15 and native numerical/smoke tests pass. Owner confirmed standalone
synth, compression, controls and saved values; completed real max10404us/1024,late0,
drops0,clips0. See development/SYNTH_BASELINE.md. WAV layering is native-smoke tested,
not separately owner-listening confirmed. Next resonator/tape/envelope/routing batch.
See ADR0009 and development/RESUME.md for exact next commands/state.

## Evidence and limits

- [Motion acceptance](baselines/MOTION_BASELINE.md): owner clean/save/exit;
  real max18134us/1024,late0,grain drops0,software clips0. Stress outliers remain.
- [EQ/reverb acceptance](baselines/EFFECTS4_BASELINE.md): owner sound/save/exit;
  real max12372us/1024,late0,drops0,clips0.
- [Browser acceptance](baselines/BROWSER_BASELINE.md),
  [EQ core measurements](baselines/EQ_BASELINE.md),
  [hardware discovery](../development/MIYOO_HARDWARE.md).
- All440baseline hashes remain preserved. Host/ARM/dummy/physical evidence is
  distinguished. Backend XRUNs are unavailable; timing hints do not prove zero XRUNs.
- Stable gates still include long combined listening/stress,headphone/speaker,
  power/sleep and VNC-disabled resources. Full feature coverage remains incomplete.

Documentation-only main was published; native code/tags/releases remain private. Permission is confirmed; original and
third-party terms are separate from scoped GPLv3 new code. The public guide is
README; compiler/debugging/legal details stay in linked engineering documents.
Historical increments and exact commands remain in baseline reports, private
session evidence and Git history; do not repeat old source/runtime investigations.


## Alpha15 controls follow-up

Owner confirmed alpha14 UI works, requested held D-pad editing and chose reset
options. Added portable bounded repeat/acceleration, three explicit reset scopes,
paused all-controls reset and rack ACTIVE/BYPASS state. Settings remain10/90fields.
Expanded numerical/actual SDL regressions; exact final gates and native candidate
are in development/RESUME.md and ALPHA15_BASELINE.md. No native publication authorized.

Alpha15 owner acceptance: repeat/reset choices work, clean sound and saved settings.
Closed-app logs verified; marked known-good, retaining alpha11 explicit rollback.
Final23host/22sanitizer/nine native cases plus independent source23host tests passed.

## Alpha16 development — control order and transcript review

Separated UI order from codec field order; all eight wet/dry pages place MIX sixth
and optional output/makeup fifth. No schema migration or audio algorithm change.
Reviewed the owner's supplied GrainBrick video transcript and actual local tempo/
rhythm/Orbix/snapshot interfaces; VIDEO_FEATURE_ROADMAP.md defines coherent next
workflows. No video frames/audio inspected. Device remains accepted alpha15.

Preparation verification:23host/22sanitizer cases, ARM cross-build/ELF, runtime5/
tooling8/hardware8/source2, format/analyzer and source/doc/reference gates pass.
No physical test or deployment of the new order yet; no screenshots taken.


## Alpha16 slice implementation —2026-11-19

Added Q48 audio-frame clock, generated16step patterns, two-voice sliced WAV source,
20ms source transitions,96field settings11 and audio-driven step/region UI at15FPS.
Shared BPM remains on RHYTHM; old LFO phase is independent. No sample duplication.
24host/23sanitizer, ARM numerical and eleven bounded native app cases passed.
All11default output checksums match alpha15; all-on offline slice p955554.5us/512,
max20493.334us remains a budget concern under stock VNC/DVFS. Owner listening is
pending in RESUME.md. Only current16+verified15 remain,3360KiB including state.
Eight owner reference photos inspected once and described in VIDEO_FEATURE_ROADMAP;
no upstream images copied, no new device screenshots. Native publication withheld.

Owner confirmed alpha16 slices/UI clean sound/saved settings. Final closed-app logs
5365callbacks/1024,worst18746us,late0,858slice triggers,0clipped; interaction RSS/
HWM6852KiB,3threads. Marked known-good, retaining15rollback. This is short acceptance,
not long stress/headphone/power/VNC-off clearance. Next: collision/excitation scene
and grouped navigation, then manual step/cut editing and snapshot/oscillator expansion.

## Alpha17 studio UI and collision source —2026-11-19

Owner explicitly requested full UI rework followed by collision instrument/grouped
navigation. Implemented original studio scenes/dialogs, X module groups, cached
backdrop/live telemetry and real six-ball/eight-voice wall-impact synthesis.
Settings12/108controls preserve1–11.26host/25sanitizer/ARM numerical and thirteen
native app cases passed. All11default output checksums match16; timing outliers
remain open (ALPHA17_BASELINE). Only17+verified16 remain,3456KiB including state.
Owner physical check pending in RESUME. One host contact sheet/no device images.


## Alpha19 — temporary sound recall

Implemented the first priority from NEXT_INSTRUMENT_REPORT: owned bounded settings
capture/recall with no sample/SD copy, source-generation refusal and SELECT-first
A/B main-view gestures. Recall preserves transport and uses the existing prepared
audio queue; capture does not mark autosave dirty. Modal/reset/help arbitration
and real SDL capture/edit/recall/source-change flow are tested. Settings12/108fields
and DSP remain unchanged. Exact final tests/device state are in RESUME.md; next
is the slow modulation model and ambient performance workspace.


## Alpha20 — ambient grain workflow

Implemented slow periodic/seeded drift/blended position, pitch and spread with
scan hold, separate base values and waveform feedback. Settings13/114controls
preserve1–12; native evidence/selector/owner acceptance are in RESUME.md and
ALPHA20_BASELINE.md. Tests cover reference/determinism/chunking/hold/pause/off/
allocation, unchanged default PCM, all-page navigation and actual SDL save/reopen.
No additional grain voices/sample copies. Next: dual/wavetable tonal layers,
manual slice patterns and export, with timing-outlier profiling alongside features.


## Alpha21 — corrected standalone ambient source

Owner clarified DRONE should generate its own sound, not require a WAV. Added an
original four-voice self-running source with seeded harmonic changes, texture/air,
breathing/pan/pitch motion and shared FX. The former page is EVOLVE. Settings14/
120controls preserve1–13. Owner confirms functionality and snapshot/icon, but
requests much more usable volume, brighter/richer generation and a dynamic scene.
This is the alpha22 priority; exact tests/native selectors remain in RESUME.md.

## Alpha22 — brighter, steerable ambient generation

Owner feedback confirmed alpha21 works but is quiet, dark and visually sparse.
Saved-patch offline measurements traced the level issue to source range, not FX.
Expanded LEVEL0–100% without raising saved values; added eight moods/wider register,
four bounded melodic event voices, rate/glide/stereo/seed/progression controls and
a real-activity voice scene. App-only editing now snaps near-zero residue to zero.
Settings15/126fields preserve1–14. Exact final evidence and device state are in
RESUME.md and ALPHA22_BASELINE.md. No source/binary publication authorized.

## Alpha23 — master output and original scenes

Owner confirms alpha22 is better, asks for more FX volume. Measured signal-dependent
wet level changes; added global post-rack trim with linked sample-peak limiter and
activity, ceiling/release, balance and mono. Old patches keep0dB/off. Added original
drone starting scenes with transactional apply/cancel preserving level/FX/sources/
transport. Settings16/132fields preserve1–15. Exact evidence/device state in RESUME.
