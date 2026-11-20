# Historical alpha25 session checkpoints

These are dated work notes and superseded next actions. For current state, read
[the live handoff](../development/RESUME.md).

# Resume GrainMM development

## Active alpha25 usability follow-up

Owner confirms the pattern/export build works, sounds clean and saves, including
an explicit SAVED confirmation. She requested moving export out of FX and a broader
UI/usability pass. This request steers the ongoing alpha25 task; do not stop at the
older export clarification below.

Implemented SOURCE direct PATTERN action; FX has nine processors; TEMPO unchanged;
TOOLS replaces STYLE with EXPORT WAV, PATCHES, MASTER OUTPUT and COLORS in a 2×2
layout. File actions require explicit A and are skipped by shoulder browsing.
Existing B/START shortcuts remain. Added control-thread pattern/export counters
and --self-test-tools, which navigates, writes five seconds, reloads and plays it.
Dedicated host route and ambient/scene regressions pass; full host35 and ASan34
suites are currently running in .local/alpha25/usability-*.txt. ARM rebuild succeeds.

Device currently remains accepted alpha25 release1f259d9b5b21/appSHA
596a911e8f889062aca1b2c84f578d16e34fc1cc2c583caf3db9389e1bdfdf3e,
previous24=9c7e67201c45. LAST_GOOD25 set after owner report/closed/save/hash checks.
Do not assume the navigation follow-up is deployed yet. Root owner exports/state
are personal data; only fresh app-owned regression fixtures may be removed.

Next: finish host/sanitizer gates and essential navigator layout validation; stage
new immutable app-only package, sequential native30+new-tools case, preserve owner
state/export files, retain accepted1f259 as rollback and prune exact archived24.
Then update docs/baseline, matching source/repro checks and explicit local commit.
Nothing pushed/published. SDK restored again for the UI follow-up; audit/compact
when final ARM checks finish. One initial host contact sheet inspected so far.

---

# Resume GrainMM development

Updated 2026-11-19. Alpha25 development in progress; alpha24 is the last accepted
physical build. No native publication or delegation. Preserve private guides,
reference hashes and existing app/state. Read CODE_QUALITY and ADR 0028.

## Alpha25 checkpoint

Implemented 16-step manual patterns, custom cut widths, versioned pattern persistence,
dedicated pattern UI and complete offline instrument export with duration/progress/
cancel. New grainmm-export CLI does not open audio/display. Original granular diagnostic
contract remains. Callback receives prepared fixed values; no allocation/I/O added.

Schema 17 retains 132 scalar fields then source and independent Pattern 1 data.
Old v1–16 files remain; corrupt modern files do not silently fall back. Patterns
are included in patches and snapshots. Rack reset preserves them; modulation reset
clears per-step reverse/probability, full sound reset returns generated defaults.

Device before update: CURRENT/LAST_GOOD 9c7e67201c45 (accepted alpha24), previous
afe50220da16 (alpha18), closed; three original WAVs preserved. Ongoing app-root-only
approval remains private. Host/sanitizer/ARM/native results are being finalized in
`.local/alpha25`; do not infer deployed success yet.

Initial 34-test host run found two WAV integration failures: legacy shoulder-event
translation in the new probe and a stale sample-name-at-EOF assertion. Corrected
probe bypasses that legacy translation in pattern mode and expects two toggles;
assertion uses the existing source position. Dedicated new unit/UI tests passed.
Final full suites are running after these corrections and writer identity hardening.

Next: finish all host/sanitizer/tooling/static gates; validate ARM; stage isolated
alpha25, bounded sequential native tests preserving all owner state, then prune
only exact archived old alpha18 files so device keeps alpha25 + accepted alpha24.
Ask for one concise physical pattern/export check. No routine screenshots.

The SDK was restored offline for this run; audit/compact after final ARM build.
Keep current build outputs, compress verified generated evidence only. Native
code/publication remains private. Use the report for subsequent synthesis work.

---

# Earlier alpha24 checkpoint (historical)

Updated 2026-11-19. Read AGENTS/CODE_QUALITY and private master/research guides.
No delegation; very few image inspections. Two host contact sheets this session,
no device screenshots. No native publication.

## Current checkpoint: alpha24 UI QA

Installed CURRENT9c7e67201c45; appSHA
c25b765eac03368068c0f24eb03fc10c1a4cdf848baf732adac4aa63402d5524.
LAST_GOOD 9c7e67201c45 is now owner-accepted alpha24; PREVIOUS afe50220da16 retains
verified alpha18. Only alpha24 and alpha18 remain: 8,384 KiB
including the owner's three original test WAVs/state. Native app binary matches the
host ARM build. Frozen older23artifacts are historical, not current candidates.

Larger separated card values/focus, MIX % display, module/source status and collision
caption. Raw0.30 remains0.30 in DSP/storage, displayed30. No schema migration:
16/132values. Typed cache fixes hidden trim edits on HARMONY; selection repaints only
two cards and reuses the scene. Glyph row spans are byte-identical in eight views.
Typed group-local range callbacks match whole-instrument reference bounds, including
sample-region dependencies. Same surface count/20Hz cap/physical rotation.

32host cases pass162.16s; final31ASan/UBSan/LSan128.53s outside ptrace/leaks retained.
All42source TUs static analysis clean; runtime5/tooling8/hardware8/source2, format,
ARM ELF/dependencies and440reference hashes pass. Two inspected host sheets only,
no device screenshots. Automated main-view bounds cover22pages×6palettes×2extremes.
An independent allowlisted source archive rebuild also passes all 32 host tests
in 160.95 seconds. Final matching-source package checks are recorded privately.

26native silent app cases pass (16legacy,4drone/schema13/reopen,2scenes/output,
2schema14migration,2schema15migration). Two native numerical suites pass, including
OUTPUT overflow regression. Temporary files individually verified/removed. Original
WAV/settings hashes preserved; three transferred WAV hashes rechecked. Running-app
guard refused one accidental overlap; subsequent sequential tests pass. Never run
Miyoo suites concurrently. No forced audible playback or Onion process changes.

Host O2 renderer/dummy,2000requests: idle2.555→1.780us; selection515.325→131.048us;
edits516.367→516.601us (roughly unchanged with larger values). Native dummy100requests:
idle91.038us (includes first scene), selection4002.622us, edits4733.685us. No native
before comparison or real-FB worst-case guarantee. App-case maxcallback26783us,
18late hints across differing cases, not XRUNs. All44offline graph casesclips0;
all11default checksums match18. p95/worst us per512:18default4883.333/11305.667;
24default5088.334/18100.5;24texture6111.5/16341.5;24texture+OUTPUT5858.667/19646.166.
Stock unmanaged clock/background/SSH/VNC; no DSP speedup/zero-XRUN claim.

Owner reports readable/responsive UI, clean audio and saved settings. Closed-app,
current binary hash and schema16save were verified; LAST_GOOD now24, PREVIOUS18.
Physical log:10002callbacks/1024,span223550ms,worst28577us,late0,softwareclips0,
491slices/776collision impacts/73drone events,242updates. This is short owner
acceptance, not long stress/headphone/power/VNC-off clearance. Keep18previous rollback. Further native actions
require app closed; no more screenshots for routine navigation.

The final install ZIP updates bundled user guidance after the initial device staging;
its code is the same tested binary. Keep the initial staging ZIP archived privately.
Do not equate package-content identity with a new DSP build. No public publication.

Research and next steps: INSTRUMENT_RESEARCH_AND_NEXT_STEPS.md compares primary
GrainBrick/PocketTracker/MIASMORA/LGPT/PO-33 documentation with local source. Next:
complete remaining release validation, manual slice editor/cut model, full-instrument offline
export, then dual/wavetable synthesis; no fake parity percentages or new engine now.
UI_QA_ALPHA24.md and ADR0027 detail changes/measurements. No external code/assets copied.

The SDK was restored offline for this build, verified against its pinned archive
and compacted again. Restore via `python3 scripts/toolchain/fetch_sdk.py` before
ARM rebuilds. Current logs/scripts are private `.local/ui-qa24` and `.local/hardware`.
Keep generated snapshots compressed; do not restore obsolete loose build files.

## Earlier maintenance checkpoint (historical)

Maintenance only: no new features. An exceptional-input OUTPUT overflow fix and
a redundant diagnostic initialization cleanup accompany the storage/docs work. Current host/ARM outputs
and canonical release artifacts are retained. Historical rebuild trees are now
verified compressed archives in `build/archive/source-rebuilds`; loose evidence
is in `build/archive/evidence`, package experiments in `build/archive/packages`,
and current alpha23 logs in `build/logs/alpha23`. Private relocation/hash manifests
are in `.local/maintenance`. Read WORKSPACE_MAINTENANCE.md before restoring outputs.

The extracted SDK was removed only after a full match to its pinned archive.
Before any ARM rebuild, restore it offline with:
`python3 scripts/toolchain/fetch_sdk.py`. The archive remains in `.local/sdk`.
Do not assume the old cached compiler path currently exists.

Connectivity was restored: per-connection `-o MACs=hmac-sha1` succeeded after
ordinary authentication dropped. Use interactive credentials and private connection
settings; no SSH service/configuration changes were made. Three original 8-second
PCM16/48k stereo test WAVs were hash-verified in `state/WAV`; host originals and
manifests are in `.local/audio`. The app itself remains alpha22, not alpha23.
No device access or writes are part of this maintenance run.

The code audit covered all 42 source translation units and found no orphan CMake
sources. Clang analysis is clean after removing one dead initialization. The OUTPUT
regression reproduced 512 non-finite samples before hardening and zero afterward;
153,600 normal-range stereo frames across six settings remain bit-identical.
The SDK was temporarily restored for successful ARM validation, then compacted.
Maintenance verification: all 32 host tests pass (163.30s), all 31 ASan/UBSan/LSan
cases pass (129.59s), plus runtime5/tooling8/hardware8/source2, ARM ELF/dependencies,
format, documentation/source gates and 440-reference preservation. No hardware
success is inferred. Private logs and exact post-maintenance ARM hash are in
`.local/maintenance/summary.json`. Prior candidate40f94504ec9d is not this build.

## Earlier alpha23 checkpoint (historical)

Owner confirms alpha22 sound/UI is much better, then requests more volume with FX.
Implemented original global post-rack−24to+18dB gain, linked sample-peak limiter,
ceiling/release, stereo balance/mono and actual reduction telemetry. Existing gains
load0dB/off; enable limiter before raising gain (try+6). No mixer/governor changes.
Rack/modulation reset keeps output for fair dry/wet comparison; full reset restores
0dB/off and pauses. No extra source voices/sample copies/heavy buffers/dependencies.

B on DRONE/GENERATE opens eight original ambient starting points. ↑/↓ selects,
←/→ changes seed, A explicitly applies only drone generation. Level, FX/output,
other sources, sample ownership, transport and snapshot eligibility are preserved.
B/X cancels; MENU exits without applying. Other pages retain waveform B. The chooser
uses cached bounded cards and blocks reset/snapshot gestures. Zero LEVEL stays zero.

Schema16/132fields appends6after126; missing-only1–15migration preserves old bytes.
New patches require23+. Stored OUTPUT21; STYLE UI22; SOURCE11/FX10cards and23total
destinations. No earlier stored IDs/reordering. The pending alpha22 CLI guard is
included in23; skip an unnecessary intermediate deployment of final22.

Frozen pre-maintenance candidate40f94504ec9d, appSHA
5e2ae1ca1bc73997152b2d13c9e1a7dacdde5d41f2858302494cc4c10a2232c1.
Its install is 208290bytes/repeated byte-identical. It predates the current
OUTPUT hardening fix: rebuild/repackage into a new output before deployment; do not
assume its native scripts or executable hashes identify the current code. Not installed yet. Earlier SSH login drops were resolved during the WAV transfer
using the documented compatibility option; native alpha23 validation is still pending. A read-only
status earlier showed CURRENT22, LAST_GOOD18, app closed, owner schema15. The subsequent WAV transfer added only the three requested samples, not a new app.
Do not change SSH/authentication/services or repeatedly retry broken sessions.

## Verification and remaining work

Independent279-file source draft builds and full32host passes161.06s. Main full32
initially had30pass/two stale schema15test expectations; corrected wave/openat/UI
3pass149.79s. Full31sanitizer initially30pass/wave same stale expectation; corrected
wave/UI2pass77.14s outside ptrace/leaks retained. Focused output/scenes/model/UI,
actual ambient apply/cancel/output/play/save/reopen, native SDK hard-float ELF/deps,
runtime5/tooling8/hardware8/source2, analyzer/format/source/docs/reference pass.
Final --help lists OUTPUT/GENERATE/scenes and ambient probe; final runtime5pass.
Final exact-code279-file source build passes32host161.36s; final full31sanitizers
pass127.97s outside ptrace/leaks retained. Matching source is finalized with docs;
only documentation changes follow the tested source draft.
Never count host/SDK/dummy as physical listening. Native alpha23 is not tested yet.

Host level audit: original drone RMS−22.45dBFS. Chorus MIX0.5 −25.51, delay0.5
−26.17, resonator0.5 −28.45; tape0.5 −21.92, reverb0.5 −22.25, all-band EQ+6
−14.22. Source/settings-dependent; avoid permanent boosts in every FX. OUTPUT+6
is independently tested≈1.995×amplitude. See ALPHA23_BASELINE and ADR0026.

Exact next steps:
1. Restore the SDK before rebuilding; for deployment, confirm app closed and renew
   the interactive SSH control session with the documented compatibility option. Read-only
   deploy preflight, then --apply approved ongoing app-root update using final23ZIP.
   Never overwrite/terminate a running app. Credentials only echo-off interactive.
2. Private alpha23-{legacy,synth,migration14,migration15,ambient}.sh contain sequential
   bounded silent native26app cases, preserving owner WAV/v9–15hashes. Numerical
   output/drone_scenes unit scripts have exact hashes and remove only their test
   file. Do not run host filesystem tests on device. Bench compares18default with
   23default/texture/texture+OUTPUT. Do not run device probes concurrently.
3. Only after validation, prune superseded22using exact host deployment tar/member/
   type/node/hash guards; keep23+verified18only, preserve owner data. Before pruning
   confirm no live executable references. Update CURRENT evidence/docs/private journal.
4. One owner check: OUTPUT LIMITER ON/GAIN+6, clean loudness; drone B scenes browse/
   cancel/apply keeps level/FX/playback, MENU/save/reopen. No routine screenshots.
5. Final gates/explicit local commit and exact corresponding-source allowlist/hash
   verification. No push/tag/release: native publication remains unauthorized.

Next musical workflows after this validation: manual slice step/cut editing, full
instrument offline WAV export, dual/generated-wavetable oscillator. Keep real-time
bounds and measured CPU budget. All-FX scheduling outliers, long listening/headphones,
power/sleep and VNC-disabled resource baselines remain open. No XRUN counter exists.

## Earlier alpha22 device record (historical)

CURRENT8c2776a58c12,alpha22, appSHA
96ecdd23d687b0fedb60cd0264d006027d256097ab8103577c63b32362eb86e4.
LAST_GOOD/PREVIOUSafe50220da16 verified18; only22+18remain,3744KiB including owner
files at last build pruning, before the additional 4544KiB WAV folder. Final22CLI-only package dee621c025cb stays host-only;23includes
its correction. Owner praises22sound/UI but explicit extended clean/save acceptance
is not inferred. Keep18verifiedrollback. All earlier440reference inputs unchanged.

## Durable contracts / debug memory

Private .local/hardware holds connection/approval, guarded scripts, logs, archives,
input sequences and session-20261119.md. Ongoing app-only approval is recorded in
deployment/approval.json. Remote scripts stream SSH stdin; large exec requests reset
this embedded server. Interactive auth only, no credentials/endpoints/captures in
tracked files. Device writes stay inside /mnt/SDCARD/App/GrainMM. No system/mixer/
CPU/SSH/network/runtime/library changes; no reboot or critical process termination.
Keep only current+verified rollback; exact archived/hash-checked pruning, no broad
cleanup. Existing SSHfbcon probes exited before UI requests, not frame validation.
Use normal owner menu launch and logs; do not repeat captures. Keep raw matching-
surface cache copies (avoids vendor blit logging) and180°rectangle rotation. SDL1
PauseAudio is not join: close audio before sample destruction. libpadsp proc-anchor/
type checks remain; never disable preload. Backend lateness hints are not XRUNs.

SOURCE0,4,5,10,11,15,16,17,18,19,20; FX13,8,2,7,12,14,6,1,3,21; TEMPO9; STYLE22.
Snapshot: main view hold SELECT first+A capture / SELECT+B recall, release between;
source-generation guard, transport kept, no sample/history/SDcopy. Successful source
replacement invalidates; failed load retains it. Owner accepted snapshot/icon.
EVOLVE18requires WAV grains and SLICE OFF; SCAN HOLD is not live recording. DRONE19/
GENERATE20are standalone sound. Scenes change no source ownership or clock phase.

All440baseline inputs and private guides stay intact/ignored; no bulkstage, gitclean,
reset/history rewrite/push/tag/release. GPL-3.0-only original work; earlier MIT and
upstream terms separate. Credit GrainMM by elly(@lkotlarenko), GrainBrick by ETRNLIAM.
Permission confirmed; never mention/request private correspondence. MIASMORA behavior
reference only, no copied code/assets. Public main is documentation only; native
metadata stays false/unstable. Older baselines are historical, not next-session tasks.
